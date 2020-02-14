#include "../header/global.h"
#include "../header/communication.h"
#include "../header/moteur.h"

extern void main(void){
   static pthread_t th[2];
   pthread_create(&th[0], NULL, (void *) propulsion, NULL);
   pthread_create(&th[1], NULL, (void *) transmission, NULL);
   for(volatile unsigned short int i = 0; i < 2; i++) 
     pthread_join(th[i], NULL);
}
