#define RECEPTION 16
#define TRANSMISSION 17
#define VITESSE_RXTX 9600

#define PAIR "PAIR\4"
#define CONNECT "CONNECT\4"
#define LOST "LOST\4"
#define LINKED "LINKED\4"
#define STOP "STOP\4"

#define BOUTONSTOP 2

#define XA 13
#define YA 12
#define BA 14
#define XB 27
#define YB 26
#define BB 15

#define ROUGE 25
#define VERT 33
#define BLEU 32
char messageRecu[31];
char messageAEnvoyer[32];
bool connecte = false;
bool premiereConnexion = true;
bool securite = true;
bool continueTheTest = true;
bool emergency_stop = false;
int boutonA = 0;
int boutonB = 0;
bool messagesConfirmationEnvoyes = false;

void setup() {
    disableCore0WDT();                                //Désactivation des watchdogs pour éviter un "bug" faisant redémarrer le microcontrôleur toutes les 5 secondes.
    disableCore1WDT();
    Serial.begin(115200);
    Serial2.begin(VITESSE_RXTX, SERIAL_8N1, RECEPTION, TRANSMISSION);   //Vitesse de transmission, paramètre UART (parité), pin de réception, pin de transmission
    delay(100);

    pinMode(ROUGE, OUTPUT);   //Définit les pins en tant que sorties
    pinMode(VERT, OUTPUT);
    pinMode(BLEU, OUTPUT);
    pinMode(BOUTONSTOP, INPUT);
    
    Serial.println("Allumé !");
    pthread_t threads[5];

    /*
    pthread_create(&threads[0], NULL, ecriture, NULL);
    pthread_create(&threads[1], NULL, choixMessage, NULL);
    pthread_create(&threads[1], NULL, test, NULL);
    if (pthread_create(&threads[1], NULL, controle, NULL)) Serial.println("Erreur thread 1 controle");
    */

    if (pthread_create(&threads[0], NULL, lecture, NULL)) Serial.println("Erreur thread 0 lecture");
    if (pthread_create(&threads[1], NULL, controleLed, NULL)) Serial.println("Erreur thread 2 controleLed");
    if (pthread_create(&threads[2], NULL, ecriture, NULL)) Serial.println("Erreur thread 3 ecriture");
    if (pthread_create(&threads[3], NULL, choixMessage, NULL)) Serial.println("Erreur thread 4 choixMessage");
    if (pthread_create(&threads[4], NULL, arretUrgence, NULL)) Serial.println("Erreur thread 5 arretUrgence");

    for (int i=0; i<4; i++) {
        pthread_join(threads[i], NULL);
    }
}

 /*
* securite
* 
 */
void *controle(void *argum) {
    while(true) {
        

        if(boutonA == 1 && boutonB == 1) {        //securite enclenchée
           securite = false;
        
            while(boutonA == 0 && boutonB == 0) {      //pilotage libre
                //pilotage etc ...
                  
                 
            }  //deconnexion
               //aterrissage en "douceur"
            securite = true;
        }

    }
}


void *arretUrgence(void *agrum) {
    while (!emergency_stop) {
      if (digitalRead(BOUTONSTOP)) emergency_stop = true;
      delay(1);
    }
}


void loop() {             //Obligatoire en Arduino
    vTaskDelete(NULL);    //Mais inutile pour ce projet, on le supprime donc avec vTaskDelete.
}

void *choixMessage(void *argum) {
    char bufferA[15];
    char bufferB[15];

    while (!messagesConfirmationEnvoyes) {
        if (isConnecte()) {
            memcpy(messageAEnvoyer, PAIR, sizeof(PAIR));
            sleep(2);
            messagesConfirmationEnvoyes = true;
        } else {
            delay(100);
        }
    }
    while (messagesConfirmationEnvoyes) {
        if (emergency_stop) {
            sprintf(messageAEnvoyer, "%s", STOP);
        } else {
            //0 = enfoncé et 1 = pas enfoncé
            boutonA = digitalRead(BA);
            boutonB = digitalRead(BB);
            joystick(bufferA, 'A', analogRead(XA), analogRead(YA), boutonA);
            joystick(bufferB, 'B', analogRead(XB), analogRead(YB), boutonB);
    
            sprintf(messageAEnvoyer, "%s", bufferA, bufferB);
            strcat(messageAEnvoyer, "\4");
        }
        delay(10);
    }
}

void joystick(char buffer[], char joystick, int X, int Y, int B) {
    sprintf(buffer, "X%c%.4dY%c%.4dB%c%.1d", joystick, X, joystick, Y, joystick, B);
}

void *ecriture(void *argum) {
    while (1) {
        if(isConnecte()) {
          Serial.print("messageAEnvoyer : ");
          Serial.println(messageAEnvoyer);
          Serial2.print(messageAEnvoyer);
          delay(400);
        }
        delay(10);
    }
}


bool isConnecte() {
    if(!strcmp(messageRecu, "LINK\4")) {
        return true;
    } else {
        return false;
    }
}


//NE PAS MODIFIER
void *lecture(void *argum) {
    char bufferRead[31];
    int i = 0;
    
    Serial.println(".");
    while(!isConnecte()) {
        if(Serial2.available() > 0) {
            bufferRead[i] = Serial2.read();
            if(bufferRead[i] == 4 || i > 32) {
                memcpy(messageRecu, bufferRead, sizeof(bufferRead));
                Serial.print("messageRecu : ");
                Serial.println(messageRecu);
                for(i = 0 ; i < 31 ; i++) {
                    bufferRead[i] = 0;
                }
                i = 0;
            } else {
                i++;
            }
        }
        delay(100);
    }
}





//A tester par Moi ah aha  ha ha (Alicia au cas où)
void *controleLed(void *argum) {
   while(true) {
      if(emergency_stop) {
          rgb_led(1, 0, 0, 100);
      }
      if(isConnecte() && premiereConnexion && !emergency_stop) {                 //connecté la première fois
          premiereConnexion = false;
          for(int i = 0 ; i < 5 ; i++) {
            rgb_led(0, 1, 0, 150);
            //Serial.println("Vert rapide 3");
          }
          while(securite && isConnecte() && !emergency_stop) {                                  //Sécurité des deux joysticks
            rgb_led(0, 1, 0, 500);
            //Serial.println("Vert lent 1");
          }
          rgb_led(0, 0, 1, 0);
          //Serial.println("Bleu fixe");
      } else if(!isConnecte() && premiereConnexion && !emergency_stop) {       //télécommande allumée mais pas connectée
          rgb_led(1, 1, 0, 0);
          //Serial.println("Jaune fixe");
      } else if(!isConnecte() && !premiereConnexion && !emergency_stop) {      //Connexion perdue : jaune en clignotant
          while(!isConnecte() && !emergency_stop) {                                         
             rgb_led(1, 1, 0, 500);
             //Serial.println("Jaune lent 1");
          }
          rgb_led(0, 0, 1, 0);                               //Reconnexion : led en bleu
          //Serial.println("Bleu fixe");
      }
      delay(100);
        
   }
  
}
  
void rgb_led(int r, int g, int b, int millise) {
    //Serial.println("On m'appelle ?");
    if (r == 1) {
        digitalWrite(ROUGE, HIGH);
    }
    if (g == 1) {
        digitalWrite(VERT, HIGH);
    }
    if (b == 1) {
        digitalWrite(BLEU, HIGH);
    }
    //delay(1/(freq*2));
    if (millise != 0) {
      delay(millise);    
      digitalWrite(ROUGE, LOW);
      digitalWrite(VERT, LOW);
      digitalWrite(BLEU, LOW);
      delay(millise);
      //delay(1/(freq*2));
    }
}
