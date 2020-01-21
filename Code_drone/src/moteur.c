#include "../header/moteur.h"

#define MAX 511 /* 2^9 = 511 valeurs */
#define MIN 0
/* GPIO du raspberry sur lequel on branche l'ESC relié à un moteur */
#define int PIN[] = {
    1, /* Correspond au PIN physique 12 (BCM18) */
    23, /* Correspond au PIN physique 33 (BCM13) */
    24, /* Correspond au PIN physique 35 (BCM19) */
    26 /* Correspond au PIN physique 32 (BCM12) */
}

extern void cycle(unsigned short int valeur){
  for(unsigned short int i = 0; i < 4; i++){
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
  for(unsigned short int i = 0; i < 4; i++)
    /* Définie sur quel PIN on effectue des opérations */
    pinMode(PIN[i], PWM_OUTPUT);
  delay(1);
}

unsigned short int puissance[4] = {0};

/* Permet la calibration des ESC par transmission.
On définit une valeur minimale et maximale qu'on émet sur une période,
pour un certain temps données dans chacun des 2 états définits par ces valeurs.

          MAX                       MAX
 2s  _____________ 2s       2s _____________ 2s
     |           |             |           |
 MIN |           |     MIN     |           |   MIN
_____|           |_____________|           |_________
  1s                   1s                       1s
*/
static void *moteur(void *arg) {
  static volatile unsigned short int *vitesse = (unsigned short int *) arg;
  cycle(MAX);
  sleep(1);
  cycle(MIN);
  sleep(2);

  printf("%d\n", *vitesse);
  volatile short int tmp = -1;
  while(1){
    /* Si la vitesse est différente de l'initialisation */
    if(*vitesse != tmp){
      cycle(*vitesse);
      tmp = *vitesse;
    }
    delay(10);
  }
}

extern void main(void) {
  configuration();
  pthread_t th_moteur[4];
  /* Puissance de rotation configurée sur chaque hélice */
  for (int i = 0; i < 4; i++)
    pthread_create(&th_moteur[i], NULL, moteur, (void *) &puissance[i]);

  delay(5000);
  
  /* Descendre la puissance des moteurs, après s'être lancé à 511 */
  for (unsigned short int i = MAX; i >= 480; i--){
    for (unsigned short int j = 0; j < 4; j++){ puissance[j] = i; }
    delay(100);
  }
  /* Réinitialisation de la puissance de chaque hélice */
  for (unsigned short int i = 0; i < 4; i++){ puissance[i] = MIN; }

  for (unsigned short int i = 0; i < 4; i++) 
    pthread_join(th_moteur[j], NULL);

  pthread_exit(NULL);
}
