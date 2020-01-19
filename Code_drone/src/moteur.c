#include "../header/moteur.h"

/* GPIO du raspberry sur lequel on branche l'ESC d'un moteur */
#define PIN[] = {
    1, 
    23,   
    24,    
    26,
};

void cycle(unsigned short int valeur){
  for(unsigned short int i = 0; i < sizeof(PIN); i++){ 
    /* Ecrire la puissance en impulsion que l'on veut fournir sur un GPIO */
    pwmWrite(PIN[i], valeur);
    delay(1);
  }
}

void configuration(void) {
  if (wiringPiSetup() == -1) {
    printf("Erreur de librairie\n");
    exit(1) ;
  }
  /* Configuration des 4 ESC pour les 4 moteurs */
  for(unsigned short int i = 0; i < sizeof(PIN); i++){
    /* Définie sur quel PIN on effectue des opérations */
    pinMode(PIN[i], PWM_OUTPUT);
  }
  
  static unsigned short int puissance;
  /* Configuration de la puissance par impulsions jusqu'au maximum */
  for(puissance = 0 ; puissance < 1024 ; puissance++)
    cycle(puissance);
  /* Configuration de la puissance par impulsions jusqu'au minimum */
  for(puissance = 1023 ; puissance >= 0 ; puissance--)
    cycle(puissance);
  delay(1);
}

extern void main(void) {
  configuration();
  static const unsigned short int vitesse = 450;
  static const unsigned short int on = 1, off = 3;

  while(1) {
    /* Allumé */
    cycle(vitesse);
    sleep(on);
    /* Eteint */
    cycle(0);
    sleep(off);
  }
}
