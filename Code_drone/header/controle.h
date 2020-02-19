#ifndef ACCELERATION_H
#define ACCELERATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
/* Librairie i2c */
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

/* Flux de donnees permettant la detection des dipositifs branches au Raspberry. */
#define BUS "/dev/i2c-1"

/* Tableau de valeurs d'acceleration lineaire */
volatile short int celerite[3];

#endif
