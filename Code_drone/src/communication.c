#include "../header/communication.h"

unsigned short int continuation = 0;

/* Vérifie l'ouverture du flux de communication série ttyAMA0 */
void connexion(){
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
}

void lecture(void) {
    unsigned char buffer[31];
    msg_recu = malloc(sizeof(buffer));
    unsigned short int i = 0;

    while(1) {
        if(serialDataAvail(fd)) {
            buffer[i] = serialGetchar(fd);
            if(buffer[i] == '\4' || i > 32) {
                if(buffer[i] == '\4')
                    memcpy(msg_recu, buffer, sizeof(buffer));
                for(i = 0 ; i < 31 ; i++)
                    buffer[i] = '\0';
                i = 0;
            } else { i++; }
        }
     }
    // continuation = 1;
}

void ecriture(unsigned char *message) {
    serialPrintf(fd, message);
}
