#include "../header/communication.h"
#include "../header/controle.h"

#define TAILLE 31
/* Variables globales définissant les états de la connexion drone-télécommande */
#define CONNECTED "connect\4"
#define LOST "lost\4"

/* file descriptor permettant de stocker le flux de communication */
static volatile int fd;

/* Vérifie l'ouverture du flux de communication série ttyAMA0 */
static const int connexion(void){
    /* Dispositif d'entrée et nombre de caractères par seconde */
    fd = serialOpen(FLUX, 9600);
    /* Problème d'ouverture série du flux de connexion */
    if(fd < 0){
        puts("Erreur communication");
        exit(1);
    /* Erreur de déploiement de certaines fonctionnalité de la librairie wiringPi */
    }else if(wiringPiSetup() == -1){
        puts("Erreur de librairie");
        exit(2);
    }else{ return fd; }
}

/* Fonction permettant de lire le flux de données envoyé par la télécommande */
static void *lecture(void * flux){
    /* Variable de récupération des caractères servant de tampon */
    unsigned char buffer[TAILLE];
    /* Message reçu par le drone */
    msg_recu = malloc(sizeof(buffer));
    static volatile unsigned short int i = 0;
    while(1){
        /* Si le flux de données est lisible */
        if(serialDataAvail(fd)){
            /* Renvoie un caractère correspondant au code ascii entier */
            buffer[i] = serialGetchar(fd);
            /* S'il y a fin de transmission ou dépassement de la taille du message */
            if((buffer[i] == '\4') || (i > TAILLE+1)){
                /* Réupèration du message en copiant le buffer dans la variable du message recu */
                memcpy(msg_recu, buffer, sizeof(buffer));
                printf("%s\n", msg_recu);
                /* Fin de la chaine de caractères */
                for(i = 0 ; i < 31 ; i++){ buffer[i] = '\0'; }
                i = 0; /* Réinitialisation du buffer */
            /* Stockage des caractères dans le buffer */
            }else{ i++; }
        }
    }
}

/* Fonction permettant d'écrire dans le flux de données à la télécommande */
static void *ecriture(void * flux) {
  /* Variable booléenne servant d'indice d'intégrité */
  static volatile unsigned short int validation;
  /* Si la télécommande est appairée au drone */
  if(strcmp(msg_recu, "pair\4")){ 
    validation = 1; 
    serialPrintf(fd, "link\4");
  }else{ validation = 0; }
  while((validation = 1) && (strcmp(msg_recu, CONNECTED))){
    /* Ecriture d'un message de connexion à rythme régulier,
    pour s'assurer que la communication fonctionne. */
    sleep(3);
    serialPrintf(fd, CONNECTED);
  }
}

/* Permet de déterminer toutes les actions à effectuer,
permettant de terminer la communciations drone-télécommande */
static void sortie(void){
    free(msg_recu);
    serialClose(fd);
    pthread_exit(NULL);
    exit(0);
}

/* Listing de tous les processus à créer et lancer en multitâche */
extern void tache(void){
    connexion();
    static pthread_t th_com[2];
    /* Ecriture et lecture synchronisés */
    pthread_create(&th_com[0], NULL, lecture, (void *)&fd);
    pthread_create(&th_com[1], NULL, ecriture, (void *)&fd);
    pthread_mutex_unlock(&mutex);
    /* Lancement de toutes les tâches */
    for(unsigned short int i = 0; i < 2; i++)
        pthread_join(th_com[i], NULL);
    sortie();
}
