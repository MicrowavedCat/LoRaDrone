#include "../header/moteur.h"

/* GPIO du raspberry sur lequel on branche le moteur */
#define PIN[4]

void cycle(unsigned short int valeur){
  for(unsigned short int i = 0; i < sizeof(PIN); i++){ 
    pwmWrite(PIN[i], valeur);
    delay(1);
  }
}

void configuration(void) {
  if (wiringPiSetup() == -1) {
    printf("Erreur de déploiement de librairie\n");
    exit(1) ;
  }
  
  for(unsigned short int i = 0; i < sizeof(PIN); i++){
    /* Branchement des ESC sur les différents GPIO du raspberry */
    if(i == 0){ PIN[0] = 1; }
    else if(i == 1){ PIN[1] = 23; }
    else if(i == 2){ PIN[2] = 24; }
    else{ PIN[3] = 26; }
    /* Définie sur quel PIN on effectue des opérations */
    pinMode(PIN[i], PWM_OUTPUT);
  }
  
  static unsigned short int puissance;
  /* Configuration de la puissance par impulsions jusqu'au maximum */
  for (puissance = 0 ; puissance < 1024 ; puissance++) {
    /* Ecrire la puissance en impulsion que l'on veut fournir */
    for(unsigned short int i = 0; i < sizeof(PIN); i++){ 
      pwmWrite(PIN[i], puissance);
      delay(1);
    }
  }
  /* Configuration de la puissance par impulsions jusqu'au minimum */
  for (puissance = 1023 ; puissance >= 0 ; puissance--) {
    for(unsigned short int i = 0; i < sizeof(PIN); i++){ 
      pwmWrite(PIN[i], puissance);
      delay(1);
    }
  }
  delay(1);
}

extern void main(void) {
  configuration();
  static const unsigned short int vitesse = 450;
  static const unsigned short int on = 1, off = 3;

  while (1) {
    for(unsigned short int i = 0; i < sizeof(PIN); i++){ 
      pwmWrite(PIN[i], vitesse);
      delay(1);
    }
    sleep(on);

    for(unsigned short int i = 0; i < sizeof(PIN); i++){ 
      pwmWrite(PIN[i], 0);
      delay(1);
    }
    sleep(off);
  }
}
