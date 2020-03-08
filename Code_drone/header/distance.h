#ifndef DISTANCE_H
#define DISTANCE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <wiringPi.h>

/* Renvoie de la distance */
volatile float distance;

extern void altitude(void);

#endif
