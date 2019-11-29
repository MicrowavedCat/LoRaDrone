#include "../header/communication.h"

int fd;
unsigned short int continuation = 0;
const unsigned char* const etat[] = { "PAIR\4", "LINK\4", "CONN\4", "LOST\4", "STOP\4" };


/* Vérifie l'ouverture du flux de communication série ttyAMA0 */
void connexion(int fd){
    /* 9600 est le nombre de bits par seconde transmis
    Problème d'ouverture série du flux de connection */
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

void lecture(void){
    unsigned char *buffer = malloc(sizeof(unsigned char));
    msg_recu = malloc(sizeof(unsigned char));
    unsigned short int chaine = -2, i = 0;
    while(1) {
        chaine = serialGetchar(fd);
        if (chaine != -1) {
            if (chaine == 4) {
                memcpy(msg_recu, buffer, sizeof(unsigned char));
                msg_recu[i] = '\0';
                i = 0;
            } else {
                buffer[i] = (unsigned char)chaine;
                i++;
            }
            printf("%s\n", msg_recu);
        }
    }
}

void *ecriture(void){
    fd = serialOpen(FLUX, 9600);
    connexion(fd);
    pthread_t th;
    for(unsigned short int i = 0; i < 5; i++){
        usleep(1000000);
        lseek(fd, 0, SEEK_END);
        write(fd, etat[i], strlen(etat[i]));
    }
    continuation = 1;
    pthread_join(th, NULL);
}
