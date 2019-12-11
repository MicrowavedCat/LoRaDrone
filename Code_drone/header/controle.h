#ifndef CONTROLE_H
#define CONTROLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/* Flux de données permettant la détection des dipositifs branchés au Raspberry. */
#define BUS "/dev/i2c-1"

const void i2c(void);

#endif
