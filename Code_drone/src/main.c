#include "../header/global.h"
#include "../header/communication.h"
#include "../header/moteur.h"
#include "../header/acceleration.h"
#include "../header/distance.h"

/* Execution synchronise des taches */
extern void main(void){
    static pthread_t th[4];
    /* Communication entre le drone et la telecommande */
    pthread_create(&th[0], NULL, (void *)transmission, NULL);
    /* Rotation des helices du drone */
    pthread_create(&th[1], NULL, (void *)propulsion, NULL);
    /* Acceleration lineaire du drone */
    pthread_create(&th[2], NULL, (void *)celerite, NULL);
    /* Distance par rapport au sol du drone */
    pthread_create(&th[3], NULL, (void *)acceleration, NULL);
    
    for(volatile unsigned short int i = 0; i < 4; i++) 
        pthread_join(th[i], NULL);

    pthread_exit(NULL);
    exit(0);
}
