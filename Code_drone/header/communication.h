#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <pthread.h>
/* Librairie de communication série sur Raspberry */
#include <wiringPi.h>
#include <wiringSerial.h>

/* Flux de données permettant, sans fil et à distance, la communication série */
#define FLUX "/dev/ttyAMA0"

const void tache(void);

/* variable globale contenant le message envoyer par la télécommande */
unsigned char *msg_recu;

#endif
