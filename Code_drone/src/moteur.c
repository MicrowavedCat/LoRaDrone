#include "../header/moteur.h"

/* GPIO du raspberry sur lequel on branche le moteur */
#define PIN 1

extern void main(void){
  unsigned short int position = 0, angle = 0;
  /* Définie sur quel pin on effectue le calcule */
  pinMode(PIN, PWM_OUTPUT);
  /*
  printf("Entrer une valeur : ");
  scanf("%hu", &angle);
  
  position = angle + 60;
  /* Ecrire la puissance que l'on veut fournir
  pwmWrite(PIN, position); */
  while(1) {
    /* Allumé pendant 1 sec */
    pwmWrite(PIN, 450);
    sleep(1);
    /* Eteint pendant 2 sec */
    pwmWrite(PIN, 0);
    sleep(2);
  }
  exit(0);
}
