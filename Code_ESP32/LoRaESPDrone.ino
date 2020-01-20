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

char tabMessage[31];
char messageAEnvoyer[30] = "PAIR";
bool connecte = false;

void setup() {
    Serial.begin(115200);
    Serial2.begin(VITESSE_RXTX, SERIAL_8N1, RECEPTION, TRANSMISSION);   //Vitesse de transmission, paramètre UART (parité), pin de réception, pin de transmission
    
    disableCore0WDT();
    disableCore1WDT();
    
    Serial.println("Connecté !");
    pthread_t threads[4];
    /*int returnValue;
  
    for( int i = 0; i< 4; i++ ) {
 
        returnValue = pthread_create(&threads[i], NULL, printThreadId, (void *)i);
 
        if (returnValue) {
            Serial.println("An error has occurred");
        }
    }*/

    pthread_create(&threads[0], NULL, ecriture, NULL);
    pthread_create(&threads[1], NULL, choixMessage, NULL);
}

void loop() {
    vTaskDelete(NULL); 
}

void *choixMessage(void *argum) {
    char bufferA[15];
    char bufferB[15];
    char bufferGeneral[30];
    char *pair = "PAIR";

    while (1) {
        if(connecte) {
            joystick(bufferA, 'A', analogRead(XA), analogRead(YA), digitalRead(BA));
            joystick(bufferB, 'B', analogRead(XB), analogRead(YB), digitalRead(BB));
            memcpy(bufferGeneral, bufferA, sizeof(bufferA));
            memcpy(bufferGeneral, bufferB, sizeof(bufferB));
            memcpy(messageAEnvoyer, bufferGeneral, sizeof(bufferGeneral));
        } else {
            memcpy(messageAEnvoyer, pair, sizeof(pair));
        }
    }
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
