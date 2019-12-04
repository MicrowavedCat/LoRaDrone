#include "../header/communication.h"
/* Variables globales vérifiant les états de la connexion drone-télécommande */
#define PAIR "pair\4"
#define LINK "link\4"
#define CONNECTED "connected\4"
#define LOST "lost\4"
#define STOP "stop\4" 
#define END "end\4" 

/* file descriptor permettant de stocker le flux de communication UART */
int fd;

/* Vérifie l'ouverture du flux UART de communication série ttyAMA0 */
int connexion(void){
    int fd = serialOpen(FLUX, 9600);
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

/* Fonction permettant de lire en UART le flux de données concernant la télécommade */
void lecture(void * args) {
    /* Variable de récupération des caractères servant de tampon */
    unsigned char buffer[31];
    /* Message recu par la télécommande */
    msg_recu = malloc(sizeof(buffer));
    unsigned short int i = 0;
    while(1) {
        /* Si le flux de données et est lisibles */
        if(serialDataAvail(fd)) {
            /* Renvoi en indice du buffer le code ascii entier correpondant aux données dans ttyAMA0 */
            buffer[i] = serialGetchar(fd);
            /* S'il y a une fin de transmission, ou dépassement de la taille du message */
            if((buffer[i] == '\4') || (i > sizeof(buffer)+1)) {
                /* Réupèration du message en copiant le buffer dans la variable du message recu */
                memcpy(msg_recu, buffer, sizeof(buffer));
                printf("%s\n", msg_recu);
                /* Arrêt d'urgence du drone */
                if(strcmp(msg_recu, STOP)) { sortie(); }
                /* Fin de la chaine de caractères */
                for(i = 0 ; i < sizeof(buffer) ; i++){ buffer[i] = '\0'; }
                i = 0; /* Réinitialisation du buffer */
            } else { i++; }
        }
    }
}

void *ecriture(void * args) {
    while(1){
        usleep(500000);
        write(fd, PAIR, strlen(PAIR);
        /* Si write ne marche pas faire :
        serialPrintf(fd, PAIR); 
        */
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
    /* ecriture + lecture en simultané */
    pthread_create(&th[0], NULL, lecture, (void *)fd);
    pthread_create(&th[1], NULL, ecriture, (void*)fd);
    for (unsigned short int i = 0; i < 2; i++)
        pthread_join(th[i], NULL);
}
