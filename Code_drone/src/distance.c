#include "../header/distance.h"

#define PIN_RECEPTEUR 29 /* Correspond au PIN physique 40 (BCM21) */
#define PIN_EMETTEUR 28 /* Correspond au PIN physique 38 (BCM20) */

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
  pinMode(PIN_RECEPTEUR, OUTPUT);
  /* Pin d'emission en mode entree */
  pinMode(PIN_EMETTEUR, INPUT);
}

static void etalonnage(){
  /* Ici on effectue un front descandant soit le fait de passer,
  de l'etat du signal logique haut a bas, sur le recepteur.
  Par default, l'etat du signal logique est bas, on l'a passer a haut.
  Ainsi, a 0, le signal d'horloge interne termine la reception.
  -----|
    1  | (Etat haut du signal logique)
       V
    0  | (Etat bas du signal logique)
       |-----
  */
   digitalWrite(PIN_RECEPTEUR, 1);
   usleep(10);
   digitalWrite(PIN_RECEPTEUR, 0);
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
    /* Impulsion - reception */
    etalonnage();

    static volatile unsigned short int echo, tmp, 
       impulsion, reflection;
    static volatile unsigned int emission, reception;
    /* Toutes les variable sont intialisee a une valeur,
    et celles-ci changeront lors du programme. */
    echo = tmp = impulsion = reflection = 0;
    
    /* Tant qu'il n'y a pas eu d'onde emise ou recue */
    while((impulsion == 0) || (reflection == 0)){
      tmp = echo;
      /* Lecture de l'etat du signal logique du PIN emetteur */
      echo = digitalRead(PIN_EMETTEUR);
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
