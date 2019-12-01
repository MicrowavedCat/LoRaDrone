#include "../header/communication.h"

const unsigned char* const etat[] = { "PAIR\4", "LINK\4", "CONN\4", "LOST\4", "STOP\4" };

void main(void){
    unsigned short int i;
    pthread_t th[2];
    pthread_create(&th[0], NULL, (void *)lecture, NULL);
    pthread_create(&th[1], NULL, (void *)ecriture, &message[0]);
    for (i=0; i<2; i++) {
        pthread_join(th[i], NULL);
    }
}
