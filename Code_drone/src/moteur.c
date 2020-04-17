#include "../header/moteur.h"
#include "../header/communication.h"
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
   volatile unsigned short int puissance;
   volatile unsigned short int id;
   volatile pthread_mutex_t mutex;
};
/* Tableau de coordonnees a convertir */
extern volatile unsigned short int coordonnee[6];
/* Tableau de valeurs d'acceleration lineaire */
extern volatile short int acceleration[3];
/* Renvoie de l'altitude */
extern volatile float distance;

/****
* @function cycle
* @param valeur : Vitesse de rotation
* Fournie a tous les moteurs la meme puissance
****/
static void cycle(unsigned short int valeur){
   for(volatile unsigned short int i = 0; i < NB_MOTEUR; i++){
      /* Ecrire la puissance en impulsion que l'on veut fournir sur un GPIO */
      pwmWrite(PIN[i], valeur);
      usleep(1000);
   }
}

/****
* @function calibration
* Etablit le mode de configuration des ESC present sur chaque PIN 
****/
static void calibration(void){
   /* Erreur de librairie */
   if(wiringPiSetup() == -1){
      puts("Erreur librairie");
      exit(1);
   }
   /* Configuration des 4 ESC pour les 4 moteurs sur la sortie du courant */
   for(volatile unsigned short int i=0; i<NB_MOTEUR; i++)
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

/* Argument pointant vers la structure des parametre moteur */
static volatile struct parametre *p;

/****
* @function *moteur
* @param *args : Structure definissant les parametre d'un moteur
* Definie l'action pouvant etre effectuee sur un moteur 
****/
static void *moteur(void *args){
   pthread_mutex_t securisation = ((struct parametre*)args)->mutex;
   /* Securiser la transmission des donnees */
   pthread_mutex_lock(&securisation);
   
   /* Variable tampon servant à définir si la vitesse est constante */
   volatile short int tmp = -1;

   while(1){
      /* On ne change la vitesse que si elle est differente de la precedente */
      if(p->puissance != tmp){
         tmp = p->puissance;
         pwmWrite(PIN[p->id], tmp);
      }else{ usleep(10000); }
   }
   /* Deverouiller la securite de transmission des donnees */
   pthread_mutex_unlock(&securisation);
}

/****
* @function conversion
* Permet de faire correspondre l'interval de valeurs des joysticks (de 0 a 4095),
* a celui de la puissance fournie dans les moteurs (de 480 a 511).
* @return valeur : La vitesse convertie definissant la rotation des moteurs
****/
static const unsigned short int conversion(volatile unsigned short int valeur,
                                           const unsigned short int min, 
                                           const unsigned short int max){
   return valeur * (max - min) / 4095 + min;
}

/****
* @function deplacement
* Permet de definir la direction que prend le drone,
* en fonction de la puissance fournie dant les moteurs.
****/
static void deplacement(void){
   static volatile unsigned short int joystick_gauche[2], joystick_droit[2];

   /* Controle haut-bas, gauche-droite */
   joystick_gauche[0] = conversion(coordonnee[0], -5, 5);
   joystick_gauche[1] = conversion(coordonnee[1], 480, 511);
   /* Controle avant-arriere */
   joystick_droit[0] = conversion(coordonnee[3], -5, 5);

   p->puissance = joystick_gauche[1];

   /* Orientation vers la gauche */
   if((joystick_gauche[0] < 0) && (joystick_droit[0] == 0)){
      for(volatile unsigned short int i=0; i<2; i++){
         p->id = i;
         p->puissance = joystick_gauche[1] + joystick_gauche[0];
      }
      for(volatile unsigned short int i=2; i<NB_MOTEUR; i++){
         p->id = i;
         p->puissance = joystick_gauche[1] - joystick_gauche[0];
      }
   /* Orientation vers la droite */
   }else if(joystick_gauche[0] > 0) && (joystick_droit[0] == 0)){
      for(volatile unsigned short int i=0; i<2; i++){
         p->id = i;
         p->puissance = joystick_gauche[1] - joystick_gauche[0];
      }
      for(volatile unsigned short int i=2; i<NB_MOTEUR; i++){
         p->id = i;
         p->puissance = joystick_gauche[1] + joystick_gauche[0];
      }
   }

  /* Orientation avant */
  if(joystick_droit[0] < 0) && (joystick_gauche[0] == 0)){
     for(volatile unsigned short int i=0; i<NB_MOTEUR; i+=2){
         p->id = i;
         p->puissance = joystick_gauche[1] - joystick_droit[0];
      }
      for(volatile unsigned short int i=1; i<NB_MOTEUR; i+=2){
         p->id = i;
         p->puissance = joystick_gauche[1] + joystick_droit[0];
      }
  /* Orientation arriere */
  }else if(joystick_droit[0] < 0) && (joystick_gauche[0] == 0)){
     for(volatile unsigned short int i=0; i<NB_MOTEUR; i+=2){
         p->id = i;
         p->puissance = joystick_gauche[1] + joystick_droit[0];
      }
      for(volatile unsigned short int i=1; i<NB_MOTEUR; i+=2){
         p->id = i;
         p->puissance = joystick_gauche[1] - joystick_droit[0];
      }
  }

   /* Orientations laterales */
   if((joystick_gauche[0] < 0) && (joystick_droit[0] < 0)){
   }else if((joystick_gauche[0] < 0) && (joystick_droit[0] > 0)){
   }else if((joystick_gauche[0] > 0) && (joystick_droit[0] < 0)){
   }else if((joystick_gauche[0] > 0) && (joystick_droit[0] > 0)){}
}

/****
* @function propulsion
* Definie l'action sur un ou plusieurs moteurs pour l'orienter
****/
extern void propulsion(void){
   calibration();
   /* Argument pointant vers la structure des parametres moteur */
   p = (struct parametre *)malloc(sizeof(struct parametre));
   /* Vitesse de rotation des moteurs */
   p->puissance = MIN;
   /* Thread a creer */
   static pthread_t th_moteur[NB_MOTEUR];

   usleep(100000);

   /* Puissance de rotation configuree sur chaque moteur */
   for(volatile unsigned short int i = 0; i < NB_MOTEUR; i++){
      p->id = i;
      pthread_create(&th_moteur[i], NULL, moteur, (void *)p);
   }

   while(1){
      usleep(100000);
      deplacement();
   }
   
   /* Lancement de tous les moteurs */
   for(volatile unsigned short int i=0; i<NB_MOTEUR; i++)
      pthread_join(th_moteur[i], NULL);
   
   /* Detacher les taches */
   pthread_exit(NULL);
   free((void *)p);
}

/****
* @function atterrissage
* Permet l'atterrissage automatique
****/
extern void atterrissage(void){
   /* Si le drone est a 1 metre du sol */
   if((volatile unsigned short int)distance <= 100){
      /* On fait baisser progressivement dans tous les moteurs,
      la puissance de rotation, jusqu'a ce qu'il atterisse. */
      for(volatile unsigned short int i = p->puissance; i >= 480; i--){
         sleep(2);
         cycle(i);
      }
   }else if((volatile unsigned short int)distance <= 15)
      /* Coupe les moteurs si on est au niveau du sol */
      cycle(MIN);
}
