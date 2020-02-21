#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <pthread.h>

/* Coordonnees de pilotage dans le message */
volatile unsigned short int coordonnee[6];

#endif
