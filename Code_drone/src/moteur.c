#include "../header/moteur.h"

#define MAX 511 /* 2^9 = 511 */
#define MIN 0

/* GPIO du raspberry sur lequel on branche l'ESC relié à un moteur */
static const int PIN[] = {
    1, /* Correspond au PIN physique 12 (BCM18) */
    23, /* Correspond au PIN physique 33 (BCM13) */
    24, /* Correspond au PIN physique 35 (BCM19) */
    26, /* Correspond au PIN physique 32 (BCM12) */
};

extern void cycle(unsigned short int valeur){
  for(unsigned short int i = 0; i < (sizeof(PIN))/4; i++){
    /* Ecrire la puissance en impulsion que l'on veut fournir sur un GPIO */
    pwmWrite(PIN[i], valeur);
    delay(1);
  }
}

/* Etablit le mode de configuration des ESC présent sur chaque PIN */
extern void configuration(void) {
  /* Erreur de librairie */
  if (wiringPiSetup() == -1) {
    printf("Erreur de librairie\n");
    exit(1);
  }
  /* Configuration des 4 ESC pour les 4 moteurs sur la sortie de courant */
  for(unsigned short int i = 0; i < (sizeof(PIN))/4; i++)
    /* Définie sur quel PIN on effectue des opérations */
    pinMode(PIN[i], PWM_OUTPUT);
  delay(1);
}

/* Permet la calibration des ESC par transmission.
On définit une valeur minimale et maximale qu'on émet sur une période,
pour un certain temps données dans chacun des 2 états définits par ces valeurs.

          MAX                       MAX
 2s  _____________ 2s       2s _____________ 2s
     |           |             |           |
 MIN |           |     MIN     |           |   MIN
_____|           |_____________|           |_________

*/
extern void main(void) {
  configuration();
  
  static volatile unsigned short int test;
  static const unsigned short int on = 2, off = 5;
  
  printf("Entrer valeur : ");
  scanf("%hu", &test);

  /* Définition de la période et des valeurs pour le calibrage */
  cycle(MAX);
  sleep(on);
  cycle(MIN);
  sleep(off);
  /* Valeurs récupérées par la télécommande */
  while(1){
    /* Allumer pendant 2 secondes */
    cycle(test);
    sleep(on);
    /* Eteindre pendant 5 secondes */
    cycle(MIN);
    sleep(off);
  }
}
