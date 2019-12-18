#include "../header/communication.h"
#include "../header/controle.h"
/* Variables globales définissant les états de la connexion drone-télécommande */
#define CONNECTED "connect\4"
#define LOST "lost\4"

/* flux descriptor permettant de stocker le flux de communication */
static volatile int fd;
/* Variable booléenne servant d'indice d'intégrité */
static volatile unsigned short int validation;

/* Vérifie l'ouverture du flux UART de communication série ttyAMA0 */
static const int connexion(void) {
    /* Dispositif d'entrée et nombre de bits par seconde */
    fd = serialOpen(FLUX, 9600);
    /* 9600 est le nombre de caractères par seconde transmis
    Problème d'ouverture série du flux de connexion */
    if(fd < 0) {
        printf("Erreur communication\n");
        exit(1);
    /* Erreur de déploiement de certaines fonctionnalité de la librairie wiringPi */
    } else if(wiringPiSetup() == -1) {
        printf("Erreur de librairie\n");
        exit(2);
    } else { return fd; }
}

/* Fonction permettant de lire le flux de données envoyé par la télécommande */
static void lecture(void * flux) {
    /* Variable de récupération des caractères servant de tampon */
    unsigned char buffer[31];
    /* Message reçu par la télécommande */
    msg_recu = malloc(sizeof(buffer));
    static volatile unsigned short int i = 0;
    while(1) {
        /* Si le flux de données est lisible */
        if(serialDataAvail(fd)) {
            /* Renvoi en indice du buffer le code ascii entier correpondant aux données dans ttyAMA0 */
            buffer[i] = serialGetchar(fd);
            /* S'il y a fin de transmission ou dépassement de la taille du message */
            if((buffer[i] == '\4') || (i > sizeof(buffer)+1)) {
                /* Réupèration du message en copiant le buffer dans la variable du message recu */
                memcpy(msg_recu, buffer, sizeof(buffer));
                printf("%s\n", msg_recu);
                /* Fin de la chaine de caractères */
                for(i = 0 ; i < sizeof(buffer) ; i++){ buffer[i] = '\0'; }
                i = 0; /* Réinitialisation du buffer */
            /* Stockage des caractères dans le buffer */
            } else { i++; }
        }
    }
}

/* Fonction permettant d'écrire dans le flux de données à la télécommande */
static void *ecriture(void * flux) {
    /* Si la télécommande est appairée au drone */
    if(strcmp(msg_recu, "pair\4")) { 
        validation = 1; 
        serialPrintf(fd, "link\4");
    } else { validation = 0; }
    while(validation == 1) {
        /* Ecriture d'un message de connexion à rythme régulier,
        pour s'assurer que la communication fonctionne. */
        usleep(3000000);
        serialPrintf(fd, CONNECTED);
    }
}

/* Permet de déterminer toutes les actions à effectuer,
permettant de terminer la communciations drone-télécommande */
static const void sortie(void) {
    free(msg_recu);
    serialClose(fd);
    pthread_exit(NULL);
    exit(0);
}

/* Listing de tous les processus à créer et lancer en multitâche */
extern const void tache(void) {
    connexion();
    static pthread_t th[2];
    /* Ecriture et lecture synchronisés */
    pthread_create(&th[0], NULL, lecture, (void *)&fd);
    pthread_create(&th[1], NULL, ecriture, (void *)&fd);
    for(unsigned short int i = 0; i < 2; i++)
        pthread_join(th[i], NULL);
    sortie();
}
