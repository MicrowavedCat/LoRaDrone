#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <wiringPi.h>
#include <wiringSerial.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* Flux de données permettant, sans fil et à distance, la communication série */
#define FLUX "/dev/ttyAMA0"

void *ecriture(void *, unsigned char *);
void *lecture(void *);
void tache(void);

unsigned char *msg_recu;

#endif
