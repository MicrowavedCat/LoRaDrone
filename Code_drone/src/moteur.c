#include "../header/moteur.h"
#include "../header/global.h"

/* Impulsion maximale 2^9 = 512 soit de 0 a 511 valeurs */
#define MAX 511
#define MIN 0
#define NB_MOTEUR 4

/* GPIO du raspberry sur lequel on branche l'ESC relie a un moteur */
static const unsigned short int PIN[NB_MOTEUR] = {
   1, /* Correspond au PIN physique 12 (BCM18), */
  23, /* Correspond au PIN physique 33 (BCM13) */
  24, /* Correspond au PIN physique 35 (BCM19) */
  26 /* Correspond au PIN physique 32 (BCM12) */
};
extern volatile unsigned short int coordonnee[6];

/* Definit pour chaque moteur la valeur de la puissance à transmettre */
extern void cycle(unsigned short int valeur){
  for(volatile unsigned short int i = 0; i < NB_MOTEUR; i++){
    /* Ecrire la puissance en impulsion que l'on veut fournir sur un GPIO */
    pwmWrite(PIN[i], valeur);
    usleep(1000);
  }
}

/* Etablit le mode de configuration des ESC present sur chaque PIN */
extern void configuration(void) {
  /* Erreur de librairie */
  if(wiringPiSetup() == -1){
    puts("Erreur librairie");
    exit(1);
  }
  /* Configuration des 4 ESC pour les 4 moteurs sur la sortie du courant */
  for(volatile unsigned short int i = 0; i < NB_MOTEUR; i++)
    /* Definie un PIN sur le mode sortie de courant */
    pinMode(PIN[i], PWM_OUTPUT);

  usleep(1000);
  /* Permet la calibration des ESC par transmission.
  On definit une valeur minimale et maximale qu'on emet sur une periode,
  pour un certain temps donne, dans chacun des 2 etats definits par ces valeurs.
            MAX                       MAX
   2s  _____________ 2s       2s _____________ 2s
       |           |             |           |
   MIN |           |     MIN     |           |   MIN
  _____|           |_____________|           |_________
    1s                   1s                       1s
  */
  cycle(MAX);
  sleep(1);
  /* Retour a l'etat minimal */
  cycle(MIN);
  sleep(1);
}

/* Definie l'action pouvant etre effectuee sur un moteur */
static void *moteur(void *puissance) {
  volatile unsigned short int *vitesse = (unsigned short int *)puissance;
  /* Variable tampon servant à définir si la vitesse est constante */
  volatile short int tmp = -1;
  while(1){
    /* On ne change la vitesse que si elle est differente de l'initialisation */
    if(*vitesse != tmp){
      cycle(*vitesse);
      tmp = *vitesse;
    }
    usleep(10000);
  }
}

extern void propulsion(void) {
  configuration();
  static pthread_t th_moteur[NB_MOTEUR];
  /* On initialise la puissance de rotation a 0 */
  static volatile unsigned short int puissance[NB_MOTEUR] = {MIN};
  /* Puissance de rotation configuree sur chaque helice */
  for(volatile unsigned short int i = 0; i < NB_MOTEUR; i++)
    pthread_create(&th_moteur[i], NULL, moteur, (void *)&puissance[i]);
  
  sleep(3);
  
  while(1) {
    usleep(100000);
    for(volatile unsigned short int i = 0; i < NB_MOTEUR; i++)
        puissance[i] = coordonnee[i];
  }

  /* Lancement de tous les moteurs */
  for(volatile unsigned short int i = 0; i < NB_MOTEUR; i++) 
    pthread_join(th_moteur[i], NULL);
  /* Détacher les taches */
  pthread_exit(NULL);
}
