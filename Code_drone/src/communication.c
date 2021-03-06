#include "../header/communication.h"
#include "../header/moteur.h"

#define TAILLE 31
/* Message de communication drone-telecommande */
#define PAIR "PAIR\4"
#define LINK "LINK\4"
#define STOP "STOP\4"
#define SECURITE "SECURITE\4"

/* file descriptor permettant de stocker le flux de communication */
static volatile int fd;
/* Variable globale contenant le message envoyer par la telecommande */
static unsigned char *msg_recu = "";
/* Coordonnees directionnelle envoyees au drone par la telecommande */
extern volatile unsigned short int coordonnee[6];

extern volatile unsigned short int securite_retiree;

/****
* @function *substr
* @param *chaine : chaine de caractere source
* @param debut : case de depart de la sous chaine de destination
* @param fin : case finale de la sous chaine de destination
* Extrait une sous-chaine d'une chaine de caractere, 
* entre une case de debut et de fin 
****/
static const unsigned char *substr(volatile unsigned char *chaine,
				   const unsigned short int debut, const unsigned short int fin){
    /* Longueur de la chaine finale */
    volatile unsigned short int longueur = fin - debut;
    /* Allocation de la taille de la chaine finale a la longueur + 1 */
    unsigned char *msg = (unsigned char*)malloc(sizeof(unsigned char) * (longueur + 1));
    /* On extrait et copie le(s) caractere(s) entre les cases de debut et de fin */
    for(volatile unsigned short int i = debut; i<fin && (*(chaine + i)!='\0'); i++){
        *msg = *(chaine + i);
        msg++;
    }
    *msg = '\0'; /* Chaine terminee */
    return msg - longueur; /* Chaine extraite */
}

/**** 
* @function filtrage
* Filtre les message recus en verifiant les elements qui le compose,
* les separateurs commencant par X, Y et Z definissent les coordonnees de pilotage
****/
static void filtrage(void){
    /* Si on recoit le message STOP, on coupe immediatement la rotation des moteurs */
    if(!(strcmp(msg_recu, STOP))){ 
	for(volatile unsigned short int i=0; i<6; i++)
            coordonnee[i] = 0;
    /* Si on recoit le message SECURITE, on stabilise le drone en mode stationaire */
    }else if(!(strcmp(msg_recu, SECURITE))){
        for(volatile unsigned short int i=0, j=3; ((i<2) && (j<5)); i++, j++){
            coordonnee[i] = 2048;
	    coordonnee[j] = 2048;
        }
	for(volatile unsigned short int i=2; i<6; i+=3)
            coordonnee[i] = 0;
    /* Verification que le message soit bien du format :
    XA----YA----BA-XB----YB----BB- */
    }else if(!(strcmp(substr(msg_recu, 0, 2), "XA")) && !(strcmp(substr(msg_recu, 6, 8), "YA")) &&
	     !(strcmp(substr(msg_recu, 12, 14), "BA")) && !(strcmp(substr(msg_recu, 15, 17), "XB")) &&
	     !(strcmp(substr(msg_recu, 21, 23), "YB")) && !(strcmp(substr(msg_recu, 27, 29), "BB")) &&
	     (msg_recu[30] == '\4') && (strcmp(msg_recu, PAIR))){

        /* Variable tampon de verification des coordonnees de pilotage dans le message */
	static volatile unsigned short int tmp[6] = {0};
	/* Position en abscisse et ordonnee du joystick de gauche */
	for(volatile unsigned short int i=0, j=2; ((i<2) && (j<13)); i++, j+=6)
            tmp[i] = (const unsigned short int)strtol(substr(msg_recu, j, j+4), NULL, 10);
        /* Position enfoncee ou non du joystick de gauche */
        tmp[2] = (const unsigned short int)strtol(substr(msg_recu, 14, 15), NULL, 10);
        /* Position en abscisse et ordonnee du joystick de droite */
	for(volatile unsigned short int i=3, j=17; ((i<5) && (j<28)); i++, j+=6)
            tmp[i] = (const unsigned short int)strtol(substr(msg_recu, j, j+4), NULL, 10);
        /* Position enfoncee ou non du joystick de droite */
        tmp[5] = (const unsigned short int)strtol(substr(msg_recu, 29, 30), NULL, 10);

	/* Verification des valeurs transmises au drone par les joysticks */
	if(((tmp[0] >= 0) && (tmp[0] <= 4095)) || ((tmp[1] >= 0) && (tmp[1] <= 4095)) 
	    || ((tmp[2] == 0) || (tmp[2] == 1)) || ((tmp[3] >= 0) && (tmp[3] <= 4095)) 
	    || ((tmp[4] >= 0) && (tmp[4] <= 4095)) || ((tmp[5] == 0) || (tmp[5] == 1))){
            /* Les valeurs des joysticks renvoient entre 0 et 4095, et un bouton enfonce, 0 ou 1.
            Si les coordonnees correspondent, le tampon de verification est egal aux coordonnees. */
            for(volatile unsigned short int i=0; i<6; i++)
	        coordonnee[i] = tmp[i];

            if(securite_retiree == 0){ securite_retiree = 1; }
        }
    }
}

/****
* @function connexion
* Verifie l'ouverture du flux de communication serie ttyAMA0 
****/
static void connexion(void){
    /* Dispositif d'entree et nombre de caracteres par seconde */
    fd = serialOpen(FLUX, 9600);
    /* Probleme d'ouverture serie du flux de connexion */
    if(fd < 0){
        puts("Erreur communication");
        exit(1);
    /* Erreur de deploiement de certaines fonctionnalite de la librairie wiringPi */
    }else if(wiringPiSetup() == -1){
        puts("Erreur de librairie");
        exit(2);
    }
}

/****
* @function *lecture
* @param *flux : le descripteur de flux de donnée
* Fonction permettant de lire le flux de donnees envoye par la telecommande 
****/
static void *lecture(void * flux){
    static volatile unsigned short int i = 0;
    /* Variable de recuperation des caracteres servant de tampon */
    static unsigned char buffer[TAILLE];
    msg_recu = malloc(sizeof(buffer));
    while(1){
        /* Si le flux de donnees est lisible */
        if(serialDataAvail(fd)){
            /* Renvoie un caractere correspondant au code ascii entier */
            buffer[i] = serialGetchar(fd);
            /* S'il y a fin de transmission ou depassement de la taille du message */
            if((buffer[i] == '\4') || (i > TAILLE+1)){
		/* Recuperation du message en copiant le buffer dans le container du message recu */
                memcpy(msg_recu, buffer, sizeof(buffer));
                filtrage();
                /* Fin de la chaine de caracteres */
                for(i = 0; i < TAILLE; i++){ buffer[i] = '\0'; }
                i = 0; /* Reinitialisation du message */
            /* Stockage des caracteres dans le message */
            }else{
                usleep(1000);
                i++;
            }
        }
    }
}

/****
* @function *ecriture
* @param *flux : le descripteur de flux de donnée
* Fonction permettant d'ecrire dans le flux de donnees a la telecommande
****/
static void *ecriture(void * flux) {
    /* Variable booléenne servant d'indice d'intégrité */
    static volatile unsigned short int validation = 0;
    /* Tant que la telecommande n'est pas appairee au drone */
    while(!validation) {
        /* On emet des tentatives de liaison */
        serialPrintf(fd, LINK);
        /* Si la telecommande est appairee au drone */
        if(strcmp(msg_recu, PAIR) == 0){
            validation = 1;
            /* On cesse d'ecrire des messages */
            exit(0);
        }
        sleep(5);
     }
}

/****
* @function sortie
* Permet de determiner toutes les actions a effectuer,
* permettant de terminer la communciations drone-telecommande 
****/
static void sortie(void){
    free(msg_recu);
    serialClose(fd);
    pthread_exit(NULL);
    exit(0);
}

/****
* @function transmission
* Listing de tous les processus a creer et lancer en multitache 
****/
extern void transmission(void){
    connexion();
    static pthread_t th_com[2];
    /* Ecriture et lecture */
    pthread_create(&th_com[0], NULL, ecriture, (void *)&fd);
    pthread_create(&th_com[1], NULL, lecture, (void *)&fd);
    /* Lancement de toutes les taches */
    for(volatile unsigned short int i=0; i<2; i++)
        pthread_join(th_com[i], NULL);
    sortie();
}
