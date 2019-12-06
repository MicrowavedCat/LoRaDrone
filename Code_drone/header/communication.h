#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
/* Librairie de communication série */
#include <wiringPi.h>
#include <wiringSerial.h>

/* Flux de données permettant, sans fil et à distance, la communication série */
#define FLUX "/dev/ttyAMA0"

void tache(void);

/* variable globale contenant le message envoyer par la télécommande */
unsigned char *msg_recu;

#endif
