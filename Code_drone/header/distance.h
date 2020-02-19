#ifndef DISTANCE_H
#define DISTANCE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <wiringPi.h>

/* Le telemetre renvoie la distance en cm,
par rapport a un obstacle au sol. */
volatile float distance;

#endif
