#include "../header/communication.h"

void main(void){
//    const unsigned char* const etat[] = { "PAIR\4", "LINK\4", "CONN\4", "LOST\4", "STOP\4" };

    fd=serialOpen(FLUX, 9600);
    // 9600 est le nombre de caractères par seconde transmis
    // Problème d'ouverture série du flux de connexion
    if (fd < 0) {
        fprintf(stderr, "Erreur d'ouverture de flux : %s\n", strerror(errno));
        exit(1);
    }
    // Erreur de déploiement de la librairie wiringPiSetup
    if (wiringPiSetup() == -1) {
        fprintf(stderr, "Erreur de librairie : %s\n", strerror(errno));
        exit(2);
    }

    unsigned short int i;
    unsigned char *message = "TEST PUTE VOLANTE";
    pthread_t th[2];
    pthread_create(&th[0], NULL, (void *)lecture, NULL);
    pthread_create(&th[1], NULL, (void *)ecriture, &message);
    for (i=0; i<2; i++) {
        pthread_join(th[i], NULL);
    }
}
