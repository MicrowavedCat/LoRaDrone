#include "../header/moteur.h"
#include "../header/global.h"

/* Impulsion maximale 2^9 = 512 soit de 0 a 511 valeurs */
#define MAX 511
#define MIN 0
#define NB_MOTEUR 4

/* GPIO du raspberry sur lequel on branche l'ESC relié à un moteur */
static const unsigned short int PIN[] = {
  1, /* Correspond au PIN physique 12 (BCM18), */
  23, /* Correspond au PIN physique 33 (BCM13) */
  24, /* Correspond au PIN physique 35 (BCM19) */
  26 /* Correspond au PIN physique 32 (BCM12) */
};

extern volatile unsigned short int coordonnee[6];

/* Définit pour chaque moteur la valeur de la puissance à transmettre */
extern void cycle(unsigned short int valeur){
  for(volatile unsigned short int i = 0; i < NB_MOTEUR; i++){
    /* Ecrire la puissance en impulsion que l'on veut fournir sur un GPIO */
    pwmWrite(PIN[i], valeur);
    usleep(1000);
  }
  sleep(1);
}

/* Etablit le mode de configuration des ESC présent sur chaque PIN */
extern void configuration(void) {
  /* Erreur de librairie */
  if(wiringPiSetup() == -1){ 
    puts("Erreur librairie");
    exit(1); 
  }
  /* Configuration des 4 ESC pour les 4 moteurs sur la sortie de courant */
  for(volatile unsigned short int i = 0; i < NB_MOTEUR; i++)
    /* Définie sur quel PIN on effectue des opérations */
    pinMode(PIN[i], PWM_OUTPUT);
  usleep(1000);
}

/* Définie l'action pouvant être effectué sur un moteur */
static void *moteur(void *puissance) {
  volatile unsigned short int *vitesse = (unsigned short int *) puissance;
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
  cycle(MAX);
  cycle(MIN);
  /* Variable tampon servant à définir si la vitesse est constante */
  volatile short int tmp = -1;
  while(1){
    /* On ne change la vitesse que si elle est différente de l'initialisation */
    if(*vitesse != tmp){
      cycle(*vitesse);
      tmp = *vitesse;
    }
    usleep(10000);
  }
  exit(0);
}

extern void propulsion(void) {
  configuration();
  static pthread_t th_moteur[NB_MOTEUR];
  /* On initialise la puissance de rotation à 0 */
  static volatile unsigned short int puissance[NB_MOTEUR] = {0};
  /* Puissance de rotation configuree sur chaque helice */
  for(volatile unsigned short int i = 0; i < NB_MOTEUR; i++)
    pthread_create(&th_moteur[i], NULL, moteur, (void *) &puissance[i]);
  sleep(5);
  
  while(1) {
    for(int i = 0; i < 6; i++) 
        printf("%d ", coordonnee[i]);
    printf("\n");
    usleep(100000);
    for(volatile unsigned short int i = 0; i < NB_MOTEUR; i++)
        puissance[i] = coordonnee[i];
  }

  for(volatile unsigned short int i = 0; i < NB_MOTEUR; i++)
    puissance[i] = MIN;
  /* Lancement de toutes les moteurs */
  for(volatile unsigned short int i = 0; i < NB_MOTEUR; i++) 
    pthread_join(th_moteur[i], NULL);
  /* Détacher les taches */
  pthread_exit(NULL);
}
