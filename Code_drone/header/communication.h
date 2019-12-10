#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <pthread.h>
#include <wiringPi.h>
#include <wringPiSetup.h>

/* Flux de données permettant, sans fil et à distance, la communication série */
#define FLUX "/dev/ttyAMA0"

void tache(void);

/* variable globale contenant le message envoyer par la télécommande */
unsigned char *msg_recu;

#endif
