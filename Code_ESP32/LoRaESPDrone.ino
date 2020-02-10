#define RECEPTION 16
#define TRANSMISSION 17
#define VITESSE_RXTX 9600

#define PAIR "PAIR\4"
#define CONNECT "CONNECT\4"
#define LOST "LOST\4"
#define LINKED "LINKED\4"

#define BOUTONSTOP 9

#define XA 13
#define YA 12
#define BA 14
#define XB 27
#define YB 26
#define BB 15

#define ROUGE 25
#define VERT 33
#define BLEU 32
char tabMessage[31];
char messageAEnvoyer[30] = "PAIR";
bool connecte = false;
unsigned long tempsDernierMessageRecu = 0;
bool premiereConnexion = true;
bool securite = true;
bool continueTheTest = true;
bool emergency_stop = false;
int boutonA = 0;
int boutonB = 0;
pthread_mutex_t mutexMessageAEnvoyer = PTHREAD_MUTEX_INITIALIZER;

void setup() {
    disableCore0WDT();                                //Désactivation des watchdogs pour éviter un "bug" faisant redémarrer le microcontrôleur toutes les 5 secondes.
    disableCore1WDT();
    Serial.begin(115200);
    Serial2.begin(VITESSE_RXTX, SERIAL_8N1, RECEPTION, TRANSMISSION);   //Vitesse de transmission, paramètre UART (parité), pin de réception, pin de transmission

    pinMode(ROUGE, OUTPUT);   //Définit les pins en tant que sorties
    pinMode(VERT, OUTPUT);
    pinMode(BLEU, OUTPUT);
    
    Serial.println("Allumé !");
    pthread_t threads[5];
    /*int returnValue;
             
    for( int i = 0; i< 4; i++ ) {
 
        returnValue = pthread_create(&threads[i], NULL, printThreadId, (void *)i);
 
        if (returnValue) {
            Serial.println("An error has occurred");
        }
    }*/

    /*
    pthread_create(&threads[0], NULL, ecriture, NULL);
    pthread_create(&threads[1], NULL, choixMessage, NULL);
    pthread_create(&threads[1], NULL, test, NULL);
    */
    pthread_create(&threads[0], NULL, lecture, NULL);
    pthread_create(&threads[1], NULL, controle, NULL);
    pthread_create(&threads[2], NULL, controleLed, NULL);
    pthread_create(&threads[3], NULL, ecriture, NULL);
    pthread_create(&threads[4], NULL, choixMessage, NULL);
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






}

void loop() {             //Obligatoire en Arduino
    vTaskDelete(NULL);    //Mais inutile pour ce projet, on le supprime donc avec vTaskDelete.
}

void *choixMessage(void *argum) {
    char bufferA[14];
    char bufferB[14];

    while (true) {
        Serial.println("tyjdyfhjdfhjdtfmhjdtshgotughou");
        if(isConnecte()) {
            //1 = enfoncé et 0 = pas enfoncé
            boutonA = digitalRead(BA)? 1:0;  
            boutonB = digitalRead(BB)? 1:0;
            joystick(bufferA, 'A', analogRead(XA), analogRead(YA), boutonA);
            joystick(bufferB, 'B', analogRead(XB), analogRead(YB), boutonB);
            
            pthread_mutex_lock(&mutexMessageAEnvoyer);
            sprintf(messageAEnvoyer, "%s%s\4", bufferA, bufferB);
            pthread_mutex_unlock(&mutexMessageAEnvoyer);
            delay(500);
            
        } else {
            memcpy(messageAEnvoyer, PAIR, sizeof(PAIR));
            delay(500);
        }
    } //connect
}

void joystick(char buffer[], char joystick, int X, int Y, int B) {
    sprintf(buffer, "X%c%.4dY%c%.4dB%c%.1d ", joystick, X, joystick, Y, joystick, B);
}

void *ecriture(void *argum) {
    while (true) {
        Serial2.print(messageAEnvoyer);
        usleep(1000000);
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
    disableCore0WDT();                                //Désactivation des watchdogs pour éviter un "bug" faisant redémarrer le microcontrôleur toutes les 5 secondes.
    disableCore1WDT();
    char bufferRead[31];
    int i = 0;
    
    Serial.println(".");
    while(true) {
        if(Serial2.available() > 0) {
            //Serial.println("avail");
            bufferRead[i] = Serial2.read();
            if(bufferRead[i] == 4 || i > 32) {
                //Serial.println(">>>fin ou plein"); ////////////////////////////////////////
                if(bufferRead[i] == 4) {
                    //Serial.println(">>>fin"); ////////////////////////////////////////
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
        delay(50);
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


void *test(void *argum) {
    Serial.println("Simulation d'une attente de connexion (6s)");
    delay(6000);                             //simulation d'une attente de connexion

    Serial.println("Simulation d'une connexion (6s)");
    keep_alive();                          //simulation d'une connexion

    Serial.println("Simulation d'un retrait de la sécurité (6s)");
    securite = false;                        //simulation d'un retrait de la sécurité
    keep_alive();

    Serial.println("Simulation d'une déconnexion (10s)");
    delay(2000);                             //simulation d'une déconnexion
    delay(8000);
    
    Serial.println("Simulation d'une reconnexion (6s)");
    keep_alive();                           //simulation d'une reconnexion

    Serial.println("Simulation d'un arrêt d'urgence (instantané)");
    emergency_stop = true;
}

void keep_alive() {
  for (int i = 0; i<3; i++) {
    tempsDernierMessageRecu = millis();   //keep alive
    delay(2000);
  }
}
