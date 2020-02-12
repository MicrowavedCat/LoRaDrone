#include "../header/communication.h"

#define TAILLE 31

/* file descriptor permettant de stocker le flux de communication */
static volatile int fd;
/* Variable globale contenant le message envoyer par la telecommande */
static unsigned char *msg_recu = "";

/* Verifie l'ouverture du flux de communication serie ttyAMA0 */
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

/* Extrait une sous-chaine d'une chaine de caractere, entre une case de debut et de fin */ 
static const unsigned char* extraction(volatile unsigned char *chaine, 
				       const unsigned short int debut, const unsigned short int fin){
    /* Longueur de la chaine finale */
    volatile unsigned short int longueur = fin - debut;
    /* Allocation de la taille de la chaine finale a la longueur + 1 */
    unsigned char *msg = (unsigned char*)malloc(sizeof(unsigned char) * (longueur + 1));
    /* On extrait et copie le(s) caractere(s) entre les cases de debut et de fin */
    for(volatile unsigned short int i = debut; 
        i < fin && (*(chaine + i) != '\0'); i++){
        *msg = *(chaine + i);
        msg++;
    }
    *msg = '\0'; /* Chaine terminee */
    return msg - longueur; /* Chaine extraite */
}

/* Filtre les message recus en vérifiant les elements qui le compose,
les separateurs commencant par X, Y et Z definissent les coordonnees de pilotage */
static void filtrage(void){
    /* Verification que le message soit bien du format :
    XA----YA----BA-XB----YB----BB-  */
    if(!(strcmp(extraction(msg_recu, 0, 2), "XA")) && !(strcmp(extraction(msg_recu, 6, 8), "YA")) &&
       !(strcmp(extraction(msg_recu, 12, 14), "BA")) && !(strcmp(extraction(msg_recu, 15, 17), "XB")) &&
       !(strcmp(extraction(msg_recu, 21, 23), "YB")) && !(strcmp(extraction(msg_recu, 27, 29), "BB")) &&
       /* Terminaison du message */ (msg_recu[30] == '\4')){
        /* Position en abscisse du bouton de gauche */
        coordonnee[0] = (const unsigned short int)atoi(extraction(msg_recu, 2, 6));
        /* Position en ordonnee du bouton de gauche */
        coordonnee[1] = (const unsigned short int)atoi(extraction(msg_recu, 8, 12));
        /* Position enfoncee ou non du bouton de gauche */
        coordonnee[2] = (const unsigned short int)atoi(extraction(msg_recu, 14, 15));
        /* Position en abscisse du bouton de droite */
        coordonnee[3] = (const unsigned short int)atoi(extraction(msg_recu, 17, 21));
        /* Position en ordonnee du bouton de droite */
        coordonnee[4] = (const unsigned short int)atoi(extraction(msg_recu, 23, 27));
        /* Position enfoncee ou non du bouton de droite */
        coordonnee[5] = (const unsigned short int)atoi(extraction(msg_recu, 29, 30));
        /* Les valeurs des joysticks revnoie entre 0 et 4095,
        Si les coordonnees sont ne correspondent pas a cet interval */
	for(volatile unsigned short int i=0; i<6; i++){
            if(coordonnee[i] < (const unsigned short int)0 || 
	       coordonnee[i] > (const unsigned short int)4095){
                puts("Coordonnees non valide");
                exit(2); 
            }
        }    
    }else{ 
        puts("Format de message non valide");
        exit(1); 
    }
}

/* Fonction permettant de lire le flux de donnees envoye par la telecommande */
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

/* Fonction permettant d'ecrire dans le flux de donnees a la telecommande */
static void *ecriture(void * flux) {
    /* Variable booléenne servant d'indice d'intégrité */
    static volatile unsigned short int validation = 0;
    /* Tant que la telecommande n'est pas appairee au drone */
    while(!validation) {
        /* On emet des tentatives de liaison */
        serialPrintf(fd, "LINK\4");
        /* Si la telecommande est appairee au drone */
        if(strcmp(msg_recu, "PAIR\4") == 0){ 
            validation = 1;
            /* On cesse d'ecrire des messages */
            exit(0);
        }
        sleep(5);
     }
}

/* Permet de determiner toutes les actions a effectuer,
permettant de terminer la communciations drone-telecommande */
static void sortie(void){
    free(msg_recu);
    serialClose(fd);
    pthread_exit(NULL);
    exit(0);
}

/* Listing de tous les processus a creer et lancer en multitache */
extern void transmission(void){
    connexion();
    static pthread_t th_com[2];
    /* Ecriture et lecture */
    pthread_create(&th_com[0], NULL, lecture, (void *)&fd);
    pthread_create(&th_com[1], NULL, ecriture, (void *)&fd);
    /* Lancement de toutes les taches */
    for(volatile unsigned short int i = 0; i < 2; i++)
        pthread_join(th_com[i], NULL);
    sortie();
}
