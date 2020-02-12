#define DEVELOPPEMENT       true                                      //Activer ou désactiver l'affichage de messages de débogage à la console

#define PIN_RECEPTION       16                                        // Communication avec le module LoRa : on reçoit des informations sur le GPIO 16,
#define PIN_TRANSMISSION    17                                          // on en envoie sur le 16,
#define VITESSE_LORA        9600                                        // à une vitesse de 9600 bauds
#define XA                  13                                        // PINs des joysticks, le GPIO 13 récupère les déplacements horizontaux du joystick de gauche,
#define YA                  12                                          // le 12, ses déplacements verticaux,
#define BA                  14                                          // le 14, s'il a été enfoncé,
#define XB                  27                                          // le 27 récupère les déplacements horizontaux du joystick de droite,
#define YB                  26                                          // le 26, ses déplacements verticaux,
#define BB                  15                                          // le 15, s'il a été enfoncé
#define BOUTON_STOP         2                                         // GPIO du bouton d'arrêt d'urgence
#define PIN_ROUGE           25                                        // GPIO des LED : 25 = rouge
#define PIN_VERT            33                                          // 33 = vert
#define PIN_BLEU            32                                          // 32 = bleu
#define MSG_LINK            "LINK\4"                                  // Messages possibles : "LINK\4" est un message qu'on doit recevoir afin d'établir une connexion
#define MSG_PAIR            "PAIR\4"                                  // "PAIR\4" est la réponse qu'il faut envoyer au drone afin de le prévenir que la connexion est établie
#define MSG_SECURITE        "SECURITE\4"                              // "SECURITE\4" est le message envoyé si la connexion est établie et que la sécurité est toujours activée
#define MSG_STOP            "STOP\4"                                  // "STOP\4" est le message envoyé si le bouton d'arrêt d'urgence est enfoncé
#define CARACTERE_FIN       "\4"                                      // "\4" est, en ASCII, un caractère de fin de transmission ; les messages échangés se terminent donc tous par celui-ci

char message_recu[6];                                                 // Contient le message reçu
char message_a_envoyer[32];                                           // Contient le message à envoyer
bool securite = true;                                                 // État de la sécurité
bool arret_urgence = false;                                           // État de l'arrêt d'urgence
bool msg_confirmation_envoyes = false;                                // État de l'envoi des messages d'établissement de la connexion
bool annonce_connexion = true;                                        // Doit-on prévenir encore l'utilisateur quand la connexion sera établie ?
int bouton_A = 1;                                                     // État du joystick de gauche
int bouton_B = 1;                                                     // État du joystick de droite
pthread_mutex_t mutex_message_a_envoyer = PTHREAD_MUTEX_INITIALIZER;  // Mutex servant à rendre l'écriture de la variable message_a_envoyer atomique
/*####################################################################################################*/



/*####################################################################################################*/
/*setup et loop*/

void setup() {
    disableCore0WDT();                                                          // Désactivation des watchdogs pour éviter un "bug"
    disableCore1WDT();                                                            // faisant redémarrer le microcontrôleur toutes les 5 secondes
    int nb_threads = 6;                                                         // Nombre de threads
    if (DEVELOPPEMENT) Serial.begin(115200);                                    // Pour le débogage uniquement, permet de communiquer avec un ordinateur
    Serial2.begin(VITESSE_LORA, SERIAL_8N1, PIN_RECEPTION, PIN_TRANSMISSION);   // Configuration de la communication UART avec le module LoRa

    pinMode(XA, INPUT);                                                         //Définition les PINs en tant qu'entrées/sorties
    pinMode(YA, INPUT);
    pinMode(BA, INPUT);
    pinMode(XB, INPUT);
    pinMode(YB, INPUT);
    pinMode(BB, INPUT);
    pinMode(BOUTON_STOP, INPUT);
    pinMode(PIN_ROUGE, OUTPUT);           
    pinMode(PIN_VERT, OUTPUT);
    pinMode(PIN_BLEU, OUTPUT);
    
    if (DEVELOPPEMENT) Serial.println("Allumé !");
    pthread_t threads[nb_threads];                                              // Création du tableau des threads

    if (!digitalRead(BA) && analogRead(XB) == 4095 && analogRead(YB) == 4095) {                       // Permet de reprendre le contrôle du drone après avoir éteient la télécommande en plein vol
        sprintf(message_recu, "%s", MSG_LINK);                                                          // au démarrage, garder le joystick de gauche enfoncé et celui de droite au maximum en haut à droite ;
        msg_confirmation_envoyes = true;                                                                // si cela a été fait, on fait croire à la télécommande qu'on a reçu le message contenu dans MSG_LINK,
        securite = false;                                                                               // que les messages de confirmation de la connexion ont été envoyés et que la sécurité a été enlevée
    }

    if (pthread_create(&threads[0], NULL, controle_arret_urgence, NULL)) if (DEVELOPPEMENT) Serial.println("Erreur thread 0 controle_arret_urgence"); // Création de tous les threads
    if (pthread_create(&threads[1], NULL, controle_securite, NULL)) if (DEVELOPPEMENT) Serial.println("Erreur thread 1 controle_securite");
    if (pthread_create(&threads[2], NULL, controle_led, NULL)) if (DEVELOPPEMENT) Serial.println("Erreur thread 2 controle_led");
    if (pthread_create(&threads[3], NULL, choix_message, NULL)) if (DEVELOPPEMENT) Serial.println("Erreur thread 3 choix_message");
    if (pthread_create(&threads[4], NULL, ecriture, NULL)) if (DEVELOPPEMENT) Serial.println("Erreur thread 4 ecriture");
    if (pthread_create(&threads[5], NULL, lecture, NULL)) if (DEVELOPPEMENT) Serial.println("Erreur thread 5 lecture");

    for (int i=0; i<nb_threads; i++) pthread_join(threads[i], NULL);                                                                                  // Attente de tous les threads
}

void loop() {
    vTaskDelete(NULL);    // Suppression de la fonction loop, obligatoire en Arduino, mais inutile pour ce projet
}
/*####################################################################################################*/



/*####################################################################################################*/
/*Quelques fonctions permettant de mettre à jour/vérifier certaines valeurs*/

/*Fonction vérifiant continuellement si l'utilisateur demande d'activer ou désactiver la sécurité*/
void *controle_securite(void *argum) {
    while (true) {
        bouton_A = digitalRead(BA);                                 // Récupération de la valeur du bouton du joystick de gauche
        bouton_B = digitalRead(BB);                                 // Récupération de la valeur du bouton du joystick de droite
        if (bouton_A == 0 && bouton_B == 0) securite?false:true;    // Si les 2 boutons sont enfoncés, demande de changement d'état de la sécurité
        delay(10);                                                  // Attente ...
    }
}

/*Fonction vérifiant continuellement si l'utilisateur demande d'activer l'arrêt d'urgence*/
void *controle_arret_urgence(void *agrum) {
    while (!arret_urgence) {
      if (digitalRead(BOUTON_STOP)) arret_urgence = true;           // Si le bouton d'arrêt d'urgence est enfoncé, demande d'un arrêt d'urgence
      delay(1);
    }
}

/*Fonction permettant de vérifier si on est connectés au drone*/
bool is_connecte() {
    if (!strcmp(message_recu, MSG_LINK)) {                          // On déclare être connectés si on a reçu le message contenu dans MSG_LINK
        return true;
    } else {
        return false;
    }
}
/*####################################################################################################*/



/*####################################################################################################*/
/*Partie concernant la gestion de la communication*/

/*Fonction assurant la réception de données tant que la connexion n'est pas encore établie*/
void *lecture(void *argum) {
    char buffer_read[6];                                                                      // Création du buffer qui contiendra temporairement le message reçu
    int i = 0;                                                                                // Création du compteur
    
    if (DEVELOPPEMENT) Serial.println(".");
    while (!is_connecte()) {                                                                  // Une fois qu'on sera connectés, on arrêtera de lire
        if (Serial2.available() > 0) {                                                        // Si des données dont disponibles,
            buffer_read[i] = Serial2.read();                                                    // on lit 1 caractère dans le buffer
            if (i == 4 && buffer_read[i] == 4) {                                              // Si le dernier caractère reçu est un caractère de fin de transmission ("\4") et que le message fait 5 caractères,
                memcpy(message_recu, buffer_read, sizeof(buffer_read));                         // on l'enregistre dans la variable prévue à cet effet
                if (DEVELOPPEMENT) { Serial.print("message_recu : "); Serial.println(message_recu); }
                for (i = 0 ; i < 6 ; i++) buffer_read[i] = 0;                                 // Réinitialisation de toutes les cases du buffer
                i = 0;                                                                        // Réinitialisation du compteur
            } else if ((i < 4 && buffer_read[i] == 4) || (i == 4 && buffer_read[i] != 4)) {   // Si le caractère de fin de transmission est arrivé "trop tôt" ou si le message fait 5 caractères, mais que le dernier caractère n'est pas celui de fin de transmission,
                for (i = 0 ; i < 6 ; i++) buffer_read[i] = 0;                                   // on réinitialise de toutes les cases du buffer et
                i = 0;                                                                          // on réinitialise aussi le compteur
            } else if (i < 4 && buffer_read[i] != 4) {                                        // Sinon, si le message fait moins de 5 caractères et que le caractère de fin de transmission n'est pas encore arrivé,
                i++;                                                                            // tout va bien, on incrémente et on continue
            } else {
                if (DEVELOPPEMENT) { Serial.println("Cette situation n'est pas censée se produire ..."); }
            }
        }
        delay(100);                                                                           // On attend ...
    }
}

/*Fonction assurant l'envoi de données une fois que la connexion est établie*/
void *ecriture(void *argum) {
    while (true) {
        if (is_connecte()) {                                        // On n'écrit rien tant qu'on n'est pas connectés,
            while (true) {                                            // mais dès qu'on l'est, on n'arrête plus
                if (DEVELOPPEMENT) { Serial.print("message_a_envoyer : "); Serial.println(message_a_envoyer); }
                pthread_mutex_lock(&mutex_message_a_envoyer);       // Mise en place du verrou pour éviter de lire des données incomplètes ou corrompues
                Serial2.print(message_a_envoyer);                   // Envoi du message au drone par l'intermédiaire du module LoRa
                pthread_mutex_unlock(&mutex_message_a_envoyer);     // Retrait du verrou
                delay(400);                                         // Envoi de 2 messages et demi par seconde, envoi plus rapide impossible : corruption du message
            }
        }
        delay(10);                                                  // Attente ...
    }
}

/*Fonction assurant la modification du message à envoyer en fonction de tous les facteurs disponibles*/
void *choix_message(void *argum) {
    char buffer_a[15];
    char buffer_b[15];

    while (!msg_confirmation_envoyes) {                                           // Tant que les messages de connexion n'ont pas été envoyés,
        if (is_connecte()) {                                                        // si on a reçu une demande de connexion (MSG_LINK)
            pthread_mutex_lock(&mutex_message_a_envoyer);                         // On verrouille le mutex
            memcpy(message_a_envoyer, MSG_PAIR, sizeof(MSG_PAIR));                // On modifie le message à envoyer en confirmation de connexion
            pthread_mutex_unlock(&mutex_message_a_envoyer);                       // On déverrouille le mutex
            sleep(2);                                                             // On laisse le temps à quelques messages de s'envoyer,
            msg_confirmation_envoyes = true;                                        // et on sauvegarde le fait que la connexion est maintenant établie
        } else {
            delay(10);                                                            // Attente ...
        }
    }
    while (true) {
        if (arret_urgence) {                                                      // Si le bonton d'arrêt d'urgence a été enfoncé
            pthread_mutex_lock(&mutex_message_a_envoyer);                         // On verrouille le mutex
            sprintf(message_a_envoyer, "%s", MSG_STOP);                           // On modifie le message à envoyer
            pthread_mutex_unlock(&mutex_message_a_envoyer);                       // On déverrouille le mutex
            pthread_exit(0);                                                      // Puis un termine ce thread afin que le message ne soit plus jamais modifié
        } else if (securite) {
            pthread_mutex_lock(&mutex_message_a_envoyer);                         // On verrouille le mutex
            sprintf(message_a_envoyer, "%s", MSG_SECURITE);                       // Le message à envoyer devient le contenu de MSG_SECURITE
            pthread_mutex_unlock(&mutex_message_a_envoyer);                       // On déverrouille le mutex
        } else {
            joystick(buffer_a, 'A', analogRead(XA), analogRead(YA), bouton_A);    // Formatage des données du joystick de gauche
            joystick(buffer_b, 'B', analogRead(XB), analogRead(YB), bouton_B);    // Formatage des données du joystick de droite
            pthread_mutex_lock(&mutex_message_a_envoyer);                         // On verrouille le mutex
            sprintf(message_a_envoyer, buffer_a, buffer_b);                       // On met le contenu des 2 buffers dans le message à envoyer,
            strcat(message_a_envoyer, CARACTERE_FIN);                               // et on y ajoute le catactère de fin de transmission
            pthread_mutex_unlock(&mutex_message_a_envoyer);                       // On déverrouille le mutex
        }
        delay(10);
    }
}

/*Fonction permettant de formater les données d'un joystick*/
void joystick(char buffer[], char joystick, int X, int Y, int B) {
    sprintf(buffer, "X%c%.4dY%c%.4dB%c%.1d", joystick, X, joystick, Y, joystick, B);  // Formatage des données d'un joystick (format : X.....Y.....B..)
}
/*####################################################################################################*/



/*####################################################################################################*/
/*Partie concernant la gestion de la LED*/

/*Fonction assurant la modification de l'état des LED en fonction de tous les facteurs disponibles*/
void *controle_led(void *argum) {
   while (true) {
      if (arret_urgence) {                                          // Bouton d'arrêt d'urgence enfoncé
          while (true) {                                            // Une fois activé, l'arrêt d'urgence ne peut plus être désactivé
              rgb_led(1, 0, 0, 100);                                //  > Rouge rapide, 5 clignotements par seconde
              delay(10);                                            // On attend ...
          }
      } else if (!is_connecte()) {                                  // Télécommande allumée mais pas encore connectée
          rgb_led(1, 1, 0, 0);                                      //  > Jaune fixe
          while (!is_connecte() && !arret_urgence) delay(100);      // Si le bouton d'arrêt d'urgence a été enfoncé, on n'attend pas d'être connectés
      } else if (is_connecte()) {                                   // Connecté
          if (annonce_connexion) {                                  // Si on vient de se connecter,
              annonce_connexion = false;                              // on désactive l'indicateur
              for (int i = 0 ; i < 5 ; i++) {                       // Clignote 5 fois
                  rgb_led(0, 1, 0, 150);                            //  > Vert rapide, 3 clignotements par seconde
                  if (arret_urgence) break;                         // On n'attend pas d'avoir terminé de clignoter si le bouton d'arrêt d'urgence a été enfoncé
              }
          }
          while (!arret_urgence && securite) {                      // Sécurité des deux joysticks activée
              rgb_led(0, 1, 0, 500);                                //  > Vert lent, 1 clignotement par seconde
          }
          if (!securite) {                                          // Si on est bien passés à la suite après avoir désactivé la sécurité
              rgb_led(0, 0, 1, 0);                                  //  > Bleu fixe
          }
      }
      delay(100);                                                   // On attend ...
        
   }
  
}

/*Fonction permettant d'allumer les LED*/
void rgb_led(int r, int g, int b, int millise) {
    stop_led();                           // Arrêt de toutes les LED
    if (r == 1) {                         // Si on a demandé à allumer la LED rouge,
        digitalWrite(PIN_ROUGE, HIGH);      // on l'allume
    }
    if (g == 1) {                         // Si on a demandé à allumer la LED verte,
        digitalWrite(PIN_VERT, HIGH);       // on l'allume
    }
    if (b == 1) {                         // Si on a demandé à allumer la LED bleue,
        digitalWrite(PIN_BLEU, HIGH);       // on l'allume
    }
    if (millise != 0) {                   // Si on demandé à faire clignoter la LED,
      delay(millise);                       // on attend le temps demandé,
      stop_led();                           // on éteint la LED,
      delay(millise);                       // on attend le temps demandé
    }
}

/*Fonction permettant d'éteindre toutes les LED*/
void stop_led() {
    digitalWrite(PIN_ROUGE, LOW);         // Extinction de la LED rouge
    digitalWrite(PIN_VERT, LOW);          // Extinction de la LED verte
    digitalWrite(PIN_BLEU, LOW);          // Extinction de la LED bleue
}
/*####################################################################################################*/
