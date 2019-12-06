#include "../header/communication.h"
/* Variables globales définissant les états de la connexion drone-télécommande */
#define PAIR "pair\4"
#define LINK "link\4"
#define CONNECT "connect\4"
#define LOST "lost\4"
#define STOP "stop\4" 
#define END "end\4" 

/* file descriptor permettant de stocker le flux de communication UART */
int fd;

/* Vérifie l'ouverture du flux UART de communication série ttyAMA0 */
int connexion(void){
    fd = serialOpen(FLUX, 9600);
    /* 9600 est le nombre de caractères par seconde transmis
    Problème d'ouverture série du flux de connexion */
    if (fd < 0) {
        fprintf(stderr, "Erreur communication : %s\n", strerror(errno));
        exit(1);
    /* Erreur de déploiement de la librairie wiringPiSetup */
    } else if (wiringPiSetup() == -1) {
        fprintf(stderr, "Erreur de librairie : %s\n", strerror(errno));
        exit(2);
    } else { return fd; }
}

/* Fonction permettant de lire le flux de données UART */
void lecture(void * flux) {
    /* Variable de récupération des caractères servant de tampon */
    unsigned char buffer[31];
    /* Message reçu par la télécommande */
    msg_recu = malloc(sizeof(buffer));
    unsigned short int i = 0;
    while(1) {
        /* Si le flux de données est lisible */
        if(serialDataAvail(fd)) {
            /* Renvoi en indice du buffer le code ascii entier correpondant aux données dans ttyAMA0 */
            buffer[i] = serialGetchar(fd);
            /* S'il y a fin de transmission ou dépassement de la taille du message */
            if((buffer[i] == '\4') || (i > sizeof(buffer)+1)) {
                /* Réupèration du message en copiant le buffer dans la variable du message recu */
                memcpy(msg_recu, buffer, sizeof(buffer));
                printf("%s\n", msg_recu);
                /* Arrêt d'urgence du drone */
                if(strcmp(msg_recu, STOP)) { sortie(); }
                /* Fin de la chaine de caractères */
                for(i = 0 ; i < sizeof(buffer) ; i++){ buffer[i] = '\0'; }
                i = 0; /* Réinitialisation du buffer */
            /* Stockage des caractères dans le buffer */
            } else { i++; }
        }
    }
}

/* Fonction permettant d'écrire dans le flux de données UART */
void *ecriture(void * flux) {
    while(1){
        usleep(1000000);
        serialPrintf(fd, "CHANGE CE ****** DE MESSAGE ET LAISSE LE PRINTF");
        /**** A venir ... ****/
    }
}

/* Permet de déterminer toutes les actions à effectuer,
permettant de terminer la communciations drone-télécommande */
void sortie(void){
    free(msg_recu);
    serialClose(fd);
    pthread_exit(NULL);
    exit(0);
}

/* Listing de tous les processus à créer et lancer en multitâche */
void tache(void){
    connexion();
    pthread_t th[2];
    /* ecriture et lecture synchronisés */
    pthread_create(&th[0], NULL, lecture, (void *)&fd);
    pthread_create(&th[1], NULL, ecriture, (void *)&fd);
    for (unsigned short int i = 0; i < 2; i++)
        pthread_join(th[i], NULL);
}
