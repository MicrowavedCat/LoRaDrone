#include "../header/communication.h"
#include "../header/controle.h"

#define TAILLE 32
/* Variables globales définissant les états de la connexion drone-télécommande */
#define PAIR "PAIR\4"
#define LINK "LINK\4"

/* variable globale contenant le message envoyer par la télécommande */
static unsigned char *msg_recu = "";

/* Vérifie l'ouverture du flux de communication série ttyAMA0 */
static void connexion(void){
    /* Dispositif d'entrée et nombre de caractères par seconde */
    fd = serialOpen(FLUX, 9600);
    /* Problème d'ouverture série du flux de connexion */
    if(fd < 0){
        puts("Erreur communication");
        exit(1);
    /* Erreur de déploiement de certaines fonctionnalité de la librairie wiringPi */
    }else if(wiringPiSetup() == -1){
        puts("Erreur de librairie");
        exit(2);
    }
}

/* Extrait une sous-chaine d'une chaine de caractère, entre une case de début et de fin */ 
static const unsigned char* extraction(const unsigned char *chaine, 
				       const unsigned short int debut, const unsigned short int fin){
    /* Longueur de la chaine finale */
    volatile unsigned short int longueur = fin - debut;
    /* Allocation de la taille de la chaine finale à la longueur + 1 */
    unsigned char *msg = (unsigned char*)malloc(sizeof(unsigned char) * (longueur + 1));
    /* On extrait et copie le(s) caractère(s) entre les cases de début et de fin */
    for(volatile unsigned short int i = debut; 
	i < fin && (*(chaine + i) != '\0'); i++){
        *msg = *(chaine + i);
        msg++;
    }
    *msg = '\0'; /* Chaine terminée */
    return msg - longueur; /* Chaine extraite */
}

static void filtrage_msg(void){
    if((!strcmp(extraction(msg_recu, 0, 2), "XA")) && 
       (!strcmp(extraction(msg_recu, 6, 8), "YA")) &&
       (!strcmp(extraction(msg_recu, 12, 14), "BA")) && 
       (!strcmp(extraction(msg_recu, 15, 17), "XB")) && 
       (!strcmp(extraction(msg_recu, 21, 23), "YB")) && 
       (!strcmp(extraction(msg_recu, 27, 29), "BB")) && (msg_recu[30] == '\4')){
	    coordonnee[0] = atoi(extraction(msg_recu, 2, 6));
	    coordonnee[1] = atoi(extraction(msg_recu, 8, 12));
            coordonnee[2] = atoi(extraction(msg_recu, 14, 15));
            coordonnee[3] = atoi(extraction(msg_recu, 17, 21));
            coordonnee[4] = atoi(extraction(msg_recu, 23, 27));
            coordonnee[5] = atoi(extraction(msg_recu, 29, 30));
	    for(volatile unsigned short int i=0; i<6; i++)
               printf("Coordonnee[%hu] : %hu\n", i, coordonnee[i]);
    }else{ 
       puts("Format de message non valide");
       exit(1); 
    }
}

/* Fonction permettant de lire le flux de données envoyé par la télécommande */
static void *lecture(void * flux){
    /* Variable de récupération des caractères servant de tampon */
    static unsigned char buffer[TAILLE];
    /* Message reçu par le drone */
    msg_recu = malloc(sizeof(buffer));
    static volatile unsigned short int i = 0;
    while(1){
        /* Si le flux de données est lisible */
        if(serialDataAvail(fd)){
            /* Renvoie un caractère correspondant au code ascii entier */
            buffer[i] = serialGetchar(fd);
            /* S'il y a fin de transmission ou dépassement de la taille du message */
            if((buffer[i] == '\4') || (i > TAILLE+1)){
                /* Réupèration du message en copiant le buffer dans la variable du message recu */
                memcpy(msg_recu, buffer, sizeof(buffer));
                printf("%s\n", msg_recu);
                filtrage_msg();
                /* Fin de la chaine de caractères */
                for(i = 0; i < TAILLE; i++){ buffer[i] = '\0'; }
                i = 0; /* Réinitialisation du buffer */
            /* Stockage des caractères dans le buffer */
            }else{ 
                usleep(1000); 
                i++;
            }
        }
    }
}

/* Fonction permettant d'écrire dans le flux de données à la télécommande */
static void *ecriture(void * flux) {
  /* Si la télécommande est appairée au drone */
  while(!validation) {
    serialPrintf(fd, LINK);
    if(strcmp(msg_recu, PAIR) == 0){ 
        validation = 1;
    }
    sleep(5);
  }
}

/* Permet de déterminer toutes les actions à effectuer,
permettant de terminer la communciations drone-télécommande */
static void sortie(void){
    free(msg_recu);
    serialClose(fd);
    pthread_exit(NULL);
    exit(0);
}

/* Listing de tous les processus à créer et lancer en multitâche */
extern void main(void){
    connexion();
    static pthread_t th_com[2];
    /* Ecriture et lecture synchronisés */
    pthread_create(&th_com[0], NULL, lecture, (void *)&fd);
    pthread_create(&th_com[1], NULL, ecriture, (void *)&fd);
    /* Lancement de toutes les tâches */
    for(volatile unsigned short int i = 0; i < 2; i++)
        pthread_join(th_com[i], NULL);
    sortie();
}
