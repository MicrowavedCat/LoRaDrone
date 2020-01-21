#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <pthread.h>
/* Librairie de communication série sur Raspberry */
#include <wiringPi.h>
#include <wiringSerial.h>

/* Variables globales définissant les états de la connexion drone-télécommande */
#define CONNECTED "connect\4"
#define LOST "lost\4"
/* Flux de données permettant, avec les modules Lora, la communication série */
#define FLUX "/dev/ttyAMA0"

extern void tache(void);

/* variable globale contenant le message envoyer par la télécommande */
unsigned char *msg_recu;

#endif
