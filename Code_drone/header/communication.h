#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <string.h>
#include <fcntl.h>
/* Librairie de communication serie sur Raspberry */
#include <wiringSerial.h>

/* Flux de donnees permettant, avec les modules Lora, la communication serie */
#define FLUX "/dev/ttyAMA0"

extern void transmission(void);

#endif
