#include "../header/communication.h"
/* Variables globales vérifiant les états de la connexion drone-télécommande */
#define PAIR "PAIR\4"
#define LINK "LINK\4"
#define CONN "CONN\4"
#define LOST "LOST\4"
#define STOP "STOP\4" 

/* Vérifie l'ouverture du flux UART de communication série ttyAMA0 */
void connexion(void){
    fd = serialOpen(FLUX, 9600);
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
    unsigned short int continuation = 0;
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
            if((buffer[i] == '\4') || i > sizeof(buffer)+1)) {
                /* Réupèration du message en copiant le buffer dans la variable du message recu */
                memcpy(msg_recu, buffer, sizeof(buffer));
                /* Fin de la chaine de caractères */
                for(i = 0 ; i < sizeof(buffer) ; i++){ buffer[i] = '\0'; }
                i = 0; /* Réinitialisation du buffer */
            } else { i++; }
            /* Arrêt d'urgence du drone */
            if(strcmp(msg_recu, STOP) { sortie(); }
        }
    }
}

void ecriture(unsigned char *message) {
    usleep(1000000);
    serialPrintf(fd, PAIR);
}

void sortie(void){
    SerialClose(fd);
    free(msg_recu);
    pthread_exit(NULL);
    exit(0);
}

void synchronisation(void){
    connexion();
    pthread_t th[2];
    pthread_create(&th[0], NULL, (void *)lecture, NULL);
    pthread_create(&th[1], NULL, (void *)ecriture, &message);
    for (unsigned short int i=0; i<2; i++)
        pthread_join(th[i], NULL);
}
