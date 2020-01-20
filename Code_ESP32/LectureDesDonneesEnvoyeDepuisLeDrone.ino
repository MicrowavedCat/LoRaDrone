#define RECEPTION 16
#define TRANSMISSION 17
#define VITESSE_RXTX 9600

char tabMessage[31];

void setup() {
    Serial.begin(115200);
    Serial2.begin(VITESSE_RXTX, SERIAL_8N1, RECEPTION, TRANSMISSION);    //Vitesse de transmission, paramètre UART (parité), pin de réception, pin de transmission
}

void loop() {
    char bufferRead[31];
    int i = 0;

    Serial.println(".");
    while(1) {
      if(Serial2.available() > 0) {
         bufferRead[i] = Serial2.read();
         if(bufferRead[i] == 4 || i > 32) {
            if(bufferRead[i] == 4) {
                copyMessage(bufferRead);
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

void copyMessage(char bufferRead[]) {
    int i;
    for(i = 0 ; i < 31 ; i++) {
        tabMessage[i] = bufferRead[i]; 
    }
}
