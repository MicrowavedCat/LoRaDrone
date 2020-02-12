#define DEVELOPPEMENT true      //Activer ou désactiver l'affichage de messages de débogage à la console


#define PIN_RECEPTION 16        // Communication avec le module LoRa
#define PIN_TRANSMISSION 17
#define VITESSE_LORA 9600

#define XA 13                   // PINs des joysticks
#define YA 12
#define BA 14
#define XB 27
#define YB 26
#define BB 15
#define BOUTON_STOP 2           // PIN du bouton d'arrêt d'urgence
#define PIN_ROUGE 25            // PINs des LEDs
#define PIN_VERT 33
#define PIN_BLEU 32

#define MSG_PAIR "PAIR\4"       // Messages possibles
#define MSG_LINK "LINK\4"
#define MSG_STOP "STOP\4"
#define MSG_SECURITE "SECURITE\4"

char message_recu[6];
char message_a_envoyer[32];
bool securite = true;                     // État de la sécurité
bool arret_urgence = false;               // État de l'arrêt d'urgence
bool msg_confirmation_envoyes = false;    // État de l'envoi des messages d'établissement de la connexion
bool annonce_connexion = true;            // Doit-on prévenir encore l'utilisateur quand la connexion sera établie ?
int bouton_A = 1;                         // État du joystick de gauche
int bouton_B = 1;                         // État du joystick de droite
/*####################################################################################################*/



/*####################################################################################################*/
/*setup et loop*/

void setup() {
    disableCore0WDT();                                                          // Désactivation des watchdogs pour éviter un "bug"
    disableCore1WDT();                                                            // faisant redémarrer le microcontrôleur toutes les 5 secondes
    if (DEVELOPPEMENT) Serial.begin(115200);                                                       // Pour le débogage uniquement, permet de communiquer avec un ordinateur
    Serial2.begin(VITESSE_LORA, SERIAL_8N1, PIN_RECEPTION, PIN_TRANSMISSION);   // Configuration de la communication UART avec le module LoRa

    pinMode(XA, INPUT);                   //Définition les PINs en tant qu'entrées/sorties
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
    pthread_t threads[6];

    if (!digitalRead(BA) && analogRead(XB) == 4095 && analogRead(YB) == 4095) {                       // Permet de reprendre le contrôle du drone après avoir éteient la télécommande en plein vol
        sprintf(message_recu, "%s", MSG_LINK);                                                          // au démarrage, garder le joystick de gauche enfoncé et celui de droite
        msg_confirmation_envoyes = true;                                                                // au maximum en haut à droite
        securite = false;
    }

    if (pthread_create(&threads[0], NULL, lecture, NULL)) if (DEVELOPPEMENT) Serial.println("Erreur thread 0 lecture");
    if (pthread_create(&threads[1], NULL, controle_led, NULL)) if (DEVELOPPEMENT) Serial.println("Erreur thread 1 controle_led");
    if (pthread_create(&threads[2], NULL, ecriture, NULL)) if (DEVELOPPEMENT) Serial.println("Erreur thread 2 ecriture");
    if (pthread_create(&threads[3], NULL, choix_message, NULL)) if (DEVELOPPEMENT) Serial.println("Erreur thread 3 choix_message");
    if (pthread_create(&threads[4], NULL, arret_d_urgence, NULL)) if (DEVELOPPEMENT) Serial.println("Erreur thread 4 arret_urgence");
    if (pthread_create(&threads[5], NULL, controle_securite, NULL)) if (DEVELOPPEMENT) Serial.println("Erreur thread 5 controle_securite");

    for (int i=0; i<6; i++) {
        pthread_join(threads[i], NULL);
    }
}

void loop() {             //Obligatoire en Arduino
    vTaskDelete(NULL);    //Mais inutile pour ce projet, on le supprime donc avec vTaskDelete.
}
/*####################################################################################################*/



/*####################################################################################################*/
/*Quelques fonctions permettant de mettre à jour/vérifier certaines valeurs*/

void *controle_securite(void *argum) {
    while (true) {
        bouton_A = digitalRead(BA);
        bouton_B = digitalRead(BB);
        if (bouton_A == 0 && bouton_B == 0) {                        // Demande de changement d'état de la sécurité
            securite?false:true;
        }
        delay(10);                                                  // Attente d'appui ...
    }
}

void *arret_d_urgence(void *agrum) {
    while (!arret_urgence) {
      if (digitalRead(BOUTON_STOP)) arret_urgence = true;           // Demande d'un arrêt d'urgence
      delay(1);
    }
}

bool is_connecte() {
    if (!strcmp(message_recu, MSG_LINK)) {
        return true;
    } else {
        return false;
    }
}
/*####################################################################################################*/



/*####################################################################################################*/
/*Partie concernant la gestion de la communication*/

void *lecture(void *argum) {
    char buffer_read[31];
    int i = 0;
    
    if (DEVELOPPEMENT) Serial.println(".");
    while (!is_connecte()) {
        if (Serial2.available() > 0) {
            buffer_read[i] = Serial2.read();
            if (buffer_read[i] == 4 || i >= 6) {
                memcpy(message_recu, buffer_read, sizeof(buffer_read));
                if (DEVELOPPEMENT) { Serial.print("message_recu : "); Serial.println(message_recu); }
                for (i = 0 ; i < 6 ; i++) {
                    buffer_read[i] = 0;
                }
                i = 0;
            } else {
                i++;
            }
        }
        delay(100);
    }
}

void *ecriture(void *argum) {
    while (true) {
        if (is_connecte()) {
            while (true) {
                if (DEVELOPPEMENT) { Serial.print("message_a_envoyer : "); Serial.println(message_a_envoyer); }
                Serial2.print(message_a_envoyer);
                delay(400);
            }
        }
        delay(10);
    }
}

void *choix_message(void *argum) {
    char bufferA[15];
    char bufferB[15];

    while (!msg_confirmation_envoyes) {
        if (is_connecte()) {
            memcpy(message_a_envoyer, MSG_PAIR, sizeof(MSG_PAIR));
            sleep(2);
            msg_confirmation_envoyes = true;
        } else {
            delay(100);
        }
    }
    while (msg_confirmation_envoyes) {
        if (arret_urgence) {
            sprintf(message_a_envoyer, "%s", MSG_STOP);
        } else if (securite) {
            sprintf(message_a_envoyer, "%s", MSG_SECURITE);
        } else {
            joystick(bufferA, 'A', analogRead(XA), analogRead(YA), bouton_A);
            joystick(bufferB, 'B', analogRead(XB), analogRead(YB), bouton_B);
    
            sprintf(message_a_envoyer, "%s", bufferA, bufferB);
            strcat(message_a_envoyer, "\4");
        }
        delay(10);
    }
}

void joystick(char buffer[], char joystick, int X, int Y, int B) {                    // Fonction permettant de formater les données d'un joystick avant de les transmettre
    sprintf(buffer, "X%c%.4dY%c%.4dB%c%.1d", joystick, X, joystick, Y, joystick, B);  // Format : XA....YA....BA.XB....YB....BB.\4
}
/*####################################################################################################*/



/*####################################################################################################*/
/*Partie concernant la gestion de la LED*/

void *controle_led(void *argum) {
   while (true) {
      if (arret_urgence) {                                          //Bouton d'arrêt d'urgence appuyé
          while (true) {                                            //Une fois activé, l'arrêt d'urgence ne peut plus être désactivé
              rgb_led(1, 0, 0, 100);                                //  > Rouge rapide, 5 clignotements par seconde
          }
      } else if (!is_connecte()) {                                  // Télécommande allumée mais pas encore connectée
          rgb_led(1, 1, 0, 0);                                      //  > Jaune fixe
          while (!is_connecte() && !arret_urgence) delay(100);
      } else if (is_connecte()) {                                   // Connecté
          if (annonce_connexion) {
              annonce_connexion = false;
              for (int i = 0 ; i < 5 ; i++) {
                  rgb_led(0, 1, 0, 150);                            //  > Vert rapide, 3 clignotements par seconde
                  if (arret_urgence) break;
              }
          }
          while (!arret_urgence && securite) {                      // Sécurité des deux joysticks activée
              rgb_led(0, 1, 0, 500);                                //  > Vert lent, 1 clignotement par seconde
          }
          if (!securite) {                                          // Si on est bien passés à la suite après avoir désactivé la sécurité
              rgb_led(0, 0, 1, 0);                                  //  > Bleu fixe
          }
      }
      delay(100);
        
   }
  
}
  
void rgb_led(int r, int g, int b, int millise) {
    stop_led();
    if (r == 1) {
        digitalWrite(PIN_ROUGE, HIGH);
    }
    if (g == 1) {
        digitalWrite(PIN_VERT, HIGH);
    }
    if (b == 1) {
        digitalWrite(PIN_BLEU, HIGH);
    }
    //delay(1/(freq*2));
    if (millise != 0) {
      delay(millise);    
      stop_led();
      delay(millise);
      //delay(1/(freq*2));
    }
}

void stop_led() {
    digitalWrite(PIN_ROUGE, LOW);
    digitalWrite(PIN_VERT, LOW);
    digitalWrite(PIN_BLEU, LOW);
}
/*####################################################################################################*/
