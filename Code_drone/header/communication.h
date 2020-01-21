#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <pthread.h>
/* Librairie de communication série sur Raspberry */
#include <wiringPi.h>
#include <wiringSerial.h>

/* Flux de données permettant, avec les modules Lora, la communication série */
#define FLUX "/dev/ttyAMA0"

/* variable globale contenant le message envoyer par la télécommande */
unsigned char *msg_recu;

extern void tache(void);

#endif
