#include "../header/distance.h"

/* PIN sur le raspberry emtteur et recepteur */
static const unsigned short int PIN[2] = {
  29, /* Correspond au PIN recepteur physique 40 (BCM21) */
  28 /* Correspond au PIN emetteur physique 38 (BCM20) */
};

/* Renvoie de la distance */
extern volatile float distance;

/****
* @function propagation
* Permet de relever le temps entre une emission,
* et une reception d'onde utlrasonore avec l'horloge interne.
****/
static const unsigned int propagation(void){
  static struct timeval tv;
  /* Date et heure courante de l'horloge interne */
  gettimeofday(&tv, NULL);
  /* On ecrit le temps en une notation scientifique constante */
  return (volatile unsigned int)1e6 * tv.tv_sec + tv.tv_usec;
}

/****
* @function configuration
* Permet de configurer le GPIO recepteur et emetteur
****/
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
}
/****
* @function etalonnage
****/
static void etalonnage(void){
  /* Ici on effectue un front descandant soit le fait de passer,
  de l'etat du signal logique haut a bas, sur le recepteur.
  Ainsi, a 0, le signal d'horloge interne termine la reception.
  -----|
    1  | (Etat haut du signal logique)
       V
    0  | (Etat bas du signal logique)
       |-----
   Par default, l'etat du signal logique est bas, on l'a passer a haut. */
   digitalWrite(PIN[0], 1);
   usleep(10);
   digitalWrite(PIN[0], 0);
}

/****
* @function altitude
* Permet de definir quand l'onde est emise et recue, et calcul une distance,
* une fois qu'on a valide le fait qu'elle bien fait un echo.
****/
extern void altitude(void){
  usleep(10000);
  configuration();
  while(1){
    /* Variables de recuperation des etats des signaux logiques,
    ainsi que des emissions et reflections ultrasons. */
    static volatile unsigned short int echo, tmp, 
       impulsion, reflection;
    static volatile unsigned int emission, reception;
    /* Toutes les variable sont intialisee a une valeur,
    et celles-ci changeront lors du programme. */
    echo = tmp = impulsion = reflection = 0;
    
    /* Impulsion et reception */
    etalonnage();
    
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
    /* Distance parcourue par le son : vitesse du son (340 m/s) * temps aller retour du son / 2
    Distance = 340m/s * t(en s) / 2 = 34000 cm/1000000μs * t(en μs) /2 = 17/1000 * t
    T = Distance * 1000 /17 = D * 58,82μs
    Distance en cm = temps propagation (en μs) / 58 */
    distance = (reception - emission) / 58;
    /* Relever altitude toutes les 1/2 secondes */
    sleep(500000);
  }
}
