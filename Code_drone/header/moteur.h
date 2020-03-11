#ifndef MOTEUR_H
#define MOTEUR_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
/* Librairie de communication serie sur Raspberry */
#include <wiringPi.h>

extern void propulsion(void);
extern void atterrissage(void);

#endif
