#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <pthread.h>
/* Librairie de communication serie sur Raspberry */
#include <wiringPi.h>
#include <wiringSerial.h>

/* Flux de donnees permettant, avec les modules Lora, la communication serie */
#define FLUX "/dev/ttyAMA0"

/* Coordonnees de pilotage dans le message */
static volatile unsigned short int coordonnee[6] = {0};

extern void transmission(void);

#endif
