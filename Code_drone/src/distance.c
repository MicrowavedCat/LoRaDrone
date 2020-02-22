#include "../header/distance.h"

/* PIN sur le raspberry emtteur et recepteur */
static const unsigned short int PIN[2] = {
  21, /* Correspond au PIN recepteur physique 29 (BCM5) */
  22 /* Correspond au PIN emetteur physique 31 (BCM6) */
};

/* Renvoie de la distance */
extern volatile float distance;

static void configuration(void){
  /* Erreur de librairie */
  if(wiringPiSetup() == -1){
    puts("Erreur librairie");
    exit(1);
  }
  /* Pin de reception en mode sortie */
  pinMode(PIN[0], OUTPUT);
  /* Pin d'emission en mode entree */
  pinMode(PIN[1], INPUT);
  /* Etat du signal logique haut */
  digitalWrite(PIN[0], 1);
  usleep(10);
  /* Etat du signal logique bas */
  digitalWrite(PIN[0], 0);
}

/* Permet de relever le temps entre une emission,
et une reception d'onde utlrasonore avec l'horloge interne */
static const long propagation(void){
  static struct timeval tv;
  /* Date et heure courante de l'horloge interne */
  gettimeofday(&tv, NULL);
  /* On ecrit le delai en une notation scientifique constante : 1e6
  Et donne le nombre de secondes et microsecondes ecoulees depuis le 1er janvier 1970 */
  return tv.tv_sec * (volatile unsigned int)1e6 + tv.tv_usec;
}

extern void collision(void){
  usleep(10000);
  configuration();
  static volatile unsigned short int echo = 0, tmp = 0,
      impulsion = 0, reflection = 0;
  static long emission, reception;
  /* Tant qu'il n'y a pas eu d'onde emise ou recue */
  while((impulsion == 0) || (reflection == 0)){
    tmp = echo;
    /* Lecture de l'etat du signal logique du PIN emetteur */
    echo = digitalRead(PIN[1]);
    /* On considere l'onde comme emise */
    if((impulsion == 0) && (tmp == 0) && (echo == 1)){
      impulsion = 1;
      emission = propagation();
    /* On considere l'onde comme reflechie */
    }else if((impulsion == 1) && (tmp == 1) && (echo == 0)){
      reflection = 1;
      reception = propagation();
    }
  }
  /* Calcul de la distance */
  distance = (reception - emission) / 58;
}
