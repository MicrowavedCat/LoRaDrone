#include "../header/communication.h"

unsigned short int continuation = 0;
const unsigned char* const etat[] = { "PAIR\4", "LINK\4", "CONN\4", "LOST\4", "STOP\4" };

/* Vérifie l'ouverture du flux UART de communication série ttyAMA0 */
void connexion(void){
    fd = serialOpen(FLUX, 9600);
    /* 9600 est le nombre de caractères par seconde transmis
    Problème d'ouverture série du flux de connexion */
    if (fd < 0) {
        fprintf(stderr, "Erreur d'ouverture de flux : %s\n", strerror(errno));
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
    /* variable de récupération des caractères servant de tampon */
    unsigned char buffer[31];
    /* message recu par la télécommande */
    msg_recu = malloc(sizeof(buffer));
    unsigned short int i = 0;
    while(1) {
        /* Si le flux de données et est lisibles */
        if(serialDataAvail(fd)) {
            /* Renvoi en indice du buffer le code ascii entier correpondant aux données dans ttyAMA0 */
            buffer[i] = serialGetchar(fd);
            /* S'il y a une fin de transmission, ou dépassement de la taille du message */
            if(buffer[i] == '\4' || i > sizeof(buffer)+1) {
                /* on réupère le message en copiant le buffer dans la variable du message recu */
                memcpy(msg_recu, buffer, sizeof(buffer));
                /* on met fin à la chaine de caractères */
                for(i = 0 ; i < sizeof(buffer) ; i++){ buffer[i] = '\0'; }
                i = 0; /* et on réinitialise le buffer */
            } else { i++; }
        }
    }
}

void arret_urgence(void){
   if(strcmp(ms_recu, etat[4]){
      /* a venir */
   }
}

void ecriture(unsigned char *message) {
    for(unsigned short j = 0; j <= 3; i++){
        usleep(1000000);
        serialPrintf(fd, message[j]);
    }
}

void synchronisation(void){
    connexion();
    pthread_t th[2];
    pthread_create(&th[0], NULL, (void *)lecture, NULL);
    pthread_create(&th[1], NULL, (void *)ecriture, &message);
    for (unsigned short int i=0; i<2; i++)
        pthread_join(th[i], NULL);
}
