#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <string.h>
#include <fcntl.h>
/* Librairie de communication serie sur Raspberry */
#include <wiringSerial.h>

/* Flux de donnees permettant, avec les modules Lora, la communication serie */
#define FLUX "/dev/ttyAMA0"

/* Coordonnees de pilotage dans le message */
volatile unsigned short int coordonnee[6];

extern void transmission(void);

#endif
