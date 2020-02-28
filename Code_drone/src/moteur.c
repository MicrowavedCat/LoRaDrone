#include "../header/global.h"
#include "../header/moteur.h"
#include "../header/acceleration.h"
#include "../header/distance.h"

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
/* Parametre d'un moteur, avec la position de son PIN,
sa puissance de rotation, et une securisation de donnee */
struct parametre{
   volatile unsigned short int *puissance;
   volatile unsigned short int id;
   volatile pthread_mutex_t *mutex;
};
/* Tableau de coordonnees a convertir */
extern volatile unsigned short int coordonnee[6];
/* Tableau de valeurs d'acceleration lineaire */
extern volatile short int acceleration[3];
/* Renvoie de la distance */
extern volatile float distance;

/****
* @function cycle
* @param valeur : Vitesse de rotation
* Definit pour chaque moteur la valeur de la puissance à transmettre
****/
extern void cycle(unsigned short int valeur){
  for(volatile unsigned short int i = 0; i < NB_MOTEUR; i++){
    /* Ecrire la puissance en impulsion que l'on veut fournir sur un GPIO */
    pwmWrite(PIN[i], valeur);
    usleep(1000);
  }
}

/****
* @function calibration
* Etablit le mode de configuration des ESC present sur chaque PIN 
*/
static void calibration(void){
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

/****
* @function *moteur
* @param *args : Structure definissant les parametre d'un moteur
* Definie l'action pouvant etre effectuee sur un moteur 
****/
static void *moteur(void *args){
  volatile unsigned short int *vitesse = ((struct parametre*)args)->puissance;
  volatile unsigned short int pin = ((struct parametre*)args)->id;
  /* Variable tampon servant à définir si la vitesse est constante */
  volatile short int tmp = -1;
  while(1){
    /* On ne change la vitesse que si elle est differente de l'initialisation */
   if(vitesse != tmp){
      tmp = vitesse;
      pwmWrite(PIN[P->id], valeur);
    }
    usleep(10000);
  }
}

/****
* @function propulsion
* Definie l'action sur un ou plusieurs moteurs pour l'orienter
****/
extern void propulsion(void){
  calibration();
  volatile struct parametre *p = (struct parametre *)malloc(sizeof(struct parametre));
  /* Vitesse de rotation des moteurs */
  volatile unsigned short int *vitesse = {MIN};
  volatile unsigned short int pin;
  /* Thread a creer */
  static pthread_t th_moteur[NB_MOTEUR];
  
  usleep(100000);
   
  while(1){
     usleep(100000);
     
     printf("Vitesse : "); 
     scanf("%d", &vitesse);
     p->puissance = vitesse;
     
     printf("GPIO : "); 
     scanf("%d", &pin);
     p->id = pin;

    /* Puissance de rotation configuree sur chaque helice */
    for(volatile unsigned short int i = 0; i < 4; i++)
       pthread_create(&th_moteur[i], NULL, moteur, (void *)p);
  }
 /* Lancement de tous les moteurs */
 for(volatile unsigned short int i = 0; i < 4; i++)
    pthread_join(th_moteur[i], NULL);
 /* Detacher les taches */
 pthread_exit(NULL);
 free((void *)p);
}

/****
* @function atterissage
* Permet l'atterissage automatique
****/
extern void atterissage(void){
   /* Coupe les moteurs si on est au niveau du sol */
   if((volatile unsigned short int)distance <= 20)
      cycle(MIN);
   else if((volatile unsigned short int)distance <= 100){
      /* On fait baisser progressivement dans tous les moteurs,
      la puissance de rotation jusqu'a ce qu'il atterisse. */
      for(volatile unsigned short int i = p->puissance; i >= 480; i--){
         sleep(2);
         cycle(i);
      }
   }
}
