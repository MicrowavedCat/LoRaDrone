#include "../header/global.h"
#include "../header/communication.h"
#include "../header/moteur.h"

/* Execution synchronise des taches */
extern void main(void){
    static pthread_t th[2];
    /* Communication entre le drone et la telecommande */
    pthread_create(&th[0], NULL, (void *)transmission, NULL);
    /* Rotation des helices du drone */
    pthread_create(&th[1], NULL, (void *)propulsion, NULL);
    
    for(volatile unsigned short int i = 0; i < 2; i++) 
        pthread_join(th[i], NULL);

    pthread_exit(NULL);
    exit(0);
}
