#ifndef CONTROLE_H
#define CONTROLE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>

/* Flux de données permettant, sans fil et à distance, la communication série */
#define BUS "dev/i2c-1"

void i2c(void);

#endif
