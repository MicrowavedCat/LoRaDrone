#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <pthread.h>

#define MAX 511 /* 2^9 = 511 valeurs */
#define MIN 0
/* GPIO du raspberry sur lequel on branche l'ESC relié à un moteur */
#define PIN[] = {
  1, /* Correspond au PIN physique 12 (BCM18) */
  23, /* Correspond au PIN physique 33 (BCM13) */
  24, /* Correspond au PIN physique 35 (BCM19) */
  26 /* Correspond au PIN physique 32 (BCM12) */
}

#endif
