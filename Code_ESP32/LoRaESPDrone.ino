#define RECEPTION 16
#define TRANSMISSION 17
#define VITESSE_RXTX 9600

#define PAIR "PAIR\4"
#define CONNECT "CONNECT\4"
#define LOST "LOST\4"
#define LINKED "LINKED\4"

#define XA 13
#define YA 12
#define BA 14
#define XB 27
#define YB 26
#define BB 15

#define ROUGE 33
#define VERT 34
#define BLEU 35

char tabMessage[31];
char messageAEnvoyer[30] = "PAIR";
bool connecte = false;
unsigned long tempsDernierMessageRecu = 0;
bool premiereConnexion = true;
bool securite = true;

void setup() {
    Serial.begin(115200);
    Serial2.begin(VITESSE_RXTX, SERIAL_8N1, RECEPTION, TRANSMISSION);   //Vitesse de transmission, paramètre UART (parité), pin de réception, pin de transmission

    pinMode(ROUGE, OUTPUT);   //Définit les pins en tant que sorties
    pinMode(VERT, OUTPUT);
    pinMode(BLEU, OUTPUT);

    disableCore0WDT();                                //Désactivation des watchdogs pour éviter un "bug" faisant redémarrer le microcontrôleur toutes les 5 secondes.
    disableCore1WDT();
    
    Serial.println("Connecté !");
    pthread_t threads[3];
    /*int returnValue;
             
    for( int i = 0; i< 4; i++ ) {
 
        returnValue = pthread_create(&threads[i], NULL, printThreadId, (void *)i);
 
        if (returnValue) {
            Serial.println("An error has occurred");
        }
    }*/

    pthread_create(&threads[0], NULL, ecriture, NULL);
    pthread_create(&threads[1], NULL, choixMessage, NULL);
    pthread_create(&threads[2], NULL, controleLed, NULL);

}

void loop() {       //Obligatoire en Arduino
    vTaskDelete(NULL);    //Mais inutile pour ce projet, on le supprime donc avec vTaskDelete.
}

void *choixMessage(void *argum) {
    char bufferA[15];
    char bufferB[15];
    char bufferGeneral[30];
    char *pair = "PAIR";

    while (1) {
        if(isConnecte()) {
            joystick(bufferA, 'A', analogRead(XA), analogRead(YA), digitalRead(BA));
            joystick(bufferB, 'B', analogRead(XB), analogRead(YB), digitalRead(BB));
            memcpy(bufferGeneral, bufferA, sizeof(bufferA));
            memcpy(bufferGeneral, bufferB, sizeof(bufferB));
            memcpy(messageAEnvoyer, bufferGeneral, sizeof(bufferGeneral));
            delay(14); //
        } else {
            memcpy(messageAEnvoyer, pair, sizeof(pair));
            delay(14); //
        }
    } //connect
}

void joystick(char buffer[], char joystick, int X, int Y, int B) {
    sprintf(buffer, "X%c%.4dY%c%.4dB%c%.1d ", joystick, X, joystick, Y, joystick, B);
}

void *ecriture(void *argum) {
    while (1) {
        Serial2.print(messageAEnvoyer);
        usleep(500000);
    }
}


bool isConnecte() {
    if((millis() - tempsDernierMessageRecu) >= 3000 || millis() <= 3000) {
        return false;
    } else {
        return true;
    }
}

//NE PAS MODIFIER
void *lecture(void *argum) {
    char bufferRead[31];
    int i = 0;
    
    Serial.println(".");
    while(1) {
        if(Serial2.available() > 0) {
            bufferRead[i] = Serial2.read();
            if(bufferRead[i] == 4 || i > 32) {
                if(bufferRead[i] == 4) {
                    memcpy(tabMessage, bufferRead, sizeof(bufferRead));
                    Serial.println(tabMessage);
                    tempsDernierMessageRecu = millis();
                }
                for(i = 0 ; i < 31 ; i++) {
                    bufferRead[i] = 0;
                }
                i = 0;
            } else {
                i++;
            }
        }
    }
}

void *controleLed(void *argum) {
   while(true) {
      if(isConnecte() && premiereConnexion) {                 //connecté la première fois
          premiereConnexion = false;
          for(int i = 0 ; i < 5 ; i++) {
            rgb_led(0, 1, 0, 150);
            Serial.println("Vert rapide 3");
          }
          while(securite) {                                  //Sécurité des deux joysticks
            rgb_led(0, 1, 0, 500);
            Serial.println("Vert lent 1");
          }
          rgb_led(0, 0, 1, 0);
          Serial.println("Bleu fixe");
      } else if(!isConnecte() && premiereConnexion) {       //télécommande allumée mais pas connectée
          rgb_led(1, 1, 0, 0);
          Serial.println("Jaune fixe");
      } else if(!isConnecte() && !premiereConnexion) {      //Connexion perdue : jaune en clignotant
          while(!isConnecte) {                                         
             rgb_led(1, 1, 0, 500);
             Serial.println("Jaune lent 1");
          }
          rgb_led(0, 0, 1, 0);                              //Reconnexion : led en bleu
          Serial.println("Bleu fixe");
      }
      delay(10);
        
   }
  
}

//rgb_led(0, 1, 1, 50);
void rgb_led(int r, int g, int b, int millise) {
    Serial.println("On m'appelle ?");
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
    sleep(1);
    delay(millise);
    
    digitalWrite(ROUGE, LOW);
    digitalWrite(VERT, LOW);
    digitalWrite(BLEU, LOW);
    delay(millise);
    //delay(1/(freq*2));
}
