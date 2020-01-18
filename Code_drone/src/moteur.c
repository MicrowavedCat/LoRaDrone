#include "../header/moteur.h"

/* GPIO du raspberry sur lequel on branche le moteur */
#define PIN1 1
#define PIN2 24
#define PIN3 26
#define PIN4 23

void configuration(void) {
  if (wiringPiSetup() == -1) {
    printf("Erreur de déploiement de librairie\n");
    exit(1) ;
  }
  
  /* Définie sur quel PIN on effectue des opérations */
  pinMode(PIN1, PWM_OUTPUT);
  pinMode(PIN2, PWM_OUTPUT);
  pinMode(PIN3, PWM_OUTPUT);
  pinMode(PIN4, PWM_OUTPUT);
  
  static unsigned short int puissance ;
  /* Configuration de la puissance par impulsions jusqu'au maximum */
  for (puissance = 0 ; puissance < 1024 ; puissance++) {
    /* Ecrire la puissance en impulsion que l'on veut fournir */
    pwmWrite (PIN1, puissance);
    delay(1);
    pwmWrite (PIN2, puissance);
    delay(1);
    pwmWrite (PIN3, puissance);
    delay (1);
    pwmWrite (PIN4, puissance);
    delay (1);
  }
  /* Configuration de la puissance par impulsions jusqu'au minimum */
  for (puissance = 1023 ; puissance >= 0 ; puissance--) {
    pwmWrite (PIN1, puissance);
    delay(1);
    pwmWrite (PIN2, puissance);
    delay(1);
    pwmWrite (PIN3, puissance);
    delay (1);
    pwmWrite (PIN4, puissance);
    delay (1);
  }
  delay(1);
}

extern void main(void) {
  configuration();
  static const unsigned short int vitesse = 450;
  static const unsigned short int on = 1, off = 3;

  while (1) {
    pwmWrite (PIN1, vitesse);
    delay(1);
    pwmWrite (PIN2, vitesse);
    delay(1);
    pwmWrite (PIN3, vitesse);
    delay(1);
    pwmWrite (PIN4, vitesse);
    sleep(on);

    pwmWrite (PIN1, 0);
    delay(1);
    pwmWrite (PIN2, 0);
    delay(1);
    pwmWrite (PIN3, 0);
    delay(1);
    pwmWrite (PIN4, 0);
    sleep(off);
  }
}
