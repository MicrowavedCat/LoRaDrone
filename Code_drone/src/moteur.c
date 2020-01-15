#include "../header/moteur.h"

/* GPIO du raspberry sur lequel on branche le moteur */
#define PIN 4

extern void main(void){
  unsigned short int position = 0, angle = 0;
  /* Définie sur quel pin on effectue le calcule */
  pinMode(PIN, PWM_OUTPUT);
  /* On passe en mode mark:space */
  pwmSetMode(PWM_MOD_MS);
  /* Diviseur de l'horloge par le timer de rotation d'une hélice, 
  soit 10.24 ms */
  pwmSetClock(192);
  
  printf("Entrer une valeur entre 10 et 360 : ");
  scanf("%hu", angle);
  
  position = angle + 60;
  pwmWrite(PIN, position);
  exit(0);
}
