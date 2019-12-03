#include "../header/communication.h"
/* Taille Maximale du tableau */
#define MAX 31
/* Variables globales vérifiant les états de la connexion drone-télécommande */
#define PAIR "PAIR\4"
#define LINK "LINK\4"
#define CONN "CONN\4"
#define LOST "LOST\4"
#define STOP "STOP\4" 

int fd;

/* Vérifie l'ouverture du flux UART de communication série ttyAMA0 */
void connexion(void){
    /* 9600 est le nombre de caractères par seconde transmis
    Problème d'ouverture série du flux de connexion */
    if (fd < 0) {
        fprintf(stderr, "Erreur communication : %s\n", strerror(errno));
        exit(1);
    }
    /* Erreur de déploiement de la librairie wiringPiSetup */
    if (wiringPiSetup() == -1) {
        fprintf(stderr, "Erreur de librairie : %s\n", strerror(errno));
        exit(2);
    }
}

/* Fonction permettant de lire en UART le flux de données concernant la télécommade */
void lecture(void) {
    fd = serialOpen(FLUX, 9600);
    connexion();
    /* Variable de récupération des caractères servant de tampon */
    unsigned char buffer[MAX];
    /* Message recu par la télécommande */
    msg_recu = malloc(sizeof(buffer));
    unsigned short int i = 0;
    while(1) {
        /* Si le flux de données et est lisibles */
        if(serialDataAvail(fd)) {
            /* Renvoi en indice du buffer le code ascii entier correpondant aux données dans ttyAMA0 */
            buffer[i] = serialGetchar(fd);
            /* S'il y a une fin de transmission, ou dépassement de la taille du message */
            if((buffer[i] == '\4') || (i > MAX+1)) {
                /* Réupèration du message en copiant le buffer dans la variable du message recu */
                memcpy(msg_recu, buffer, sizeof(buffer));
                printf("%s\n", msg_recu);
                /* Arrêt d'urgence du drone */
                if(strcmp(msg_recu, STOP) { sortie(); }
                /* Fin de la chaine de caractères */
                for(i = 0 ; i < MAX ; i++){ buffer[i] = '\0'; }
                i = 0; /* Réinitialisation du buffer */
            } else { i++; }
        }
    }
}

void ecriture(unsigned char *message) {
    fd = serialOpen(FLUX, 9600);
    connexion();
    usleep(500000);
    serialPrintf(fd, PAIR);
    /**** A venir ... ****/
}

/* Permet de déterminer toutes les actions à effectuer,
permettant de terminer la communciations drone-télécommande */
void sortie(void){
    free(msg_recu);
    serialClose(fd);
    pthread_exit(NULL);
    exit(0);
}

void tache(void){
    pthread_t th[2];
    pthread_create(&th[0], NULL, (void *)lecture, NULL);
    pthread_create(&th[1], NULL, (void *)ecriture, &message);
    for (unsigned short int i = 0; i < 2; i++)
        pthread_join(th[i], NULL);
}
