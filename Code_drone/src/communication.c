#include "../header/communication.h"
#include "../header/controle.h"

#define TAILLE 32
/* Variables globales définissant les états de la connexion drone-télécommande */
#define PAIR "PAIR\4"
#define LINK "LINK\4"

/* Vérifie l'ouverture du flux de communication série ttyAMA0 */
static void connexion(void){
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
    }
}

static inline unsigned short int decoupage(unsigned short int position_depart, 
    unsigned short int offset) {
    coordonnee[offset] = (unsigned short int)msg_recu[position_depart];
    coordonnee[offset] += (unsigned short int)msg_recu[position_depart-1]*10;
    coordonnee[offset] += (unsigned short int)msg_recu[position_depart-2]*100;
    coordonnee[offset] += (unsigned short int)msg_recu[position_depart-3]*1000;
}

static inline unsigned short int filtrage_msg(void){
    if(msg_recu[0] == 'X' && msg_recu[1] == 'A' &&
        msg_recu[6] == 'Y' && msg_recu[7] == 'A' &&
        msg_recu[12] == 'B' && msg_recu[13] == 'A' &&
        msg_recu[15] == 'X' && msg_recu[16] == 'B' &&
        msg_recu[21] == 'Y' && msg_recu[22] == 'B' &&
        msg_recu[27] == 'B' && msg_recu[28] == 'B' &&
        msg_recu[30] == '\4'){
            decoupage((unsigned short int)5, (unsigned short int)0);
            decoupage((unsigned short int)11, (unsigned short int)1);
            decoupage((unsigned short int)20, (unsigned short int)2);
            decoupage((unsigned short int)26, (unsigned short int)3);
            coordonnee[4] = (unsigned short int) msg_recu[14];
            coordonnee[5] = (unsigned short int) msg_recu[28];
            for(unsigned short int i=0; i<6; i++)
               printf("%d ", coordonnee[i]);
            putchar('\n');
            return 1;
    }else{ return 0; }
}

/* Fonction permettant de lire le flux de données envoyé par la télécommande */
static void *lecture(void * flux){
    /* Variable de récupération des caractères servant de tampon */
    static unsigned char buffer[TAILLE];
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
                filtrage_msg();
                /* Fin de la chaine de caractères */
                for(i = 0; i < TAILLE; i++){ buffer[i] = '\0'; }
                i = 0; /* Réinitialisation du buffer */
            /* Stockage des caractères dans le buffer */
            }else{ 
                usleep(1000); 
                i++;
            }
        }
    }
}

/* Fonction permettant d'écrire dans le flux de données à la télécommande */
static void *ecriture(void * flux) {
  /* Si la télécommande est appairée au drone */
  while(!validation) {
    serialPrintf(fd, LINK);
    if(strcmp(msg_recu, PAIR) == 0){ 
        validation = 1;
    }
    sleep(5);
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
extern void main(void){
    connexion();
    static pthread_t th_com[2];
    /* Ecriture et lecture synchronisés */
    pthread_create(&th_com[0], NULL, lecture, (void *)&fd);
    pthread_create(&th_com[1], NULL, ecriture, (void *)&fd);
    /* Lancement de toutes les tâches */
    for(volatile unsigned short int i = 0; i < 2; i++)
        pthread_join(th_com[i], NULL);
    sortie();
}
