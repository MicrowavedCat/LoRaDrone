#define ROUGE 12
#define VERT 13
#define BLEU 14

void setup() {
    Serial.begin(115200);     //Créer connexion
    pinMode(ROUGE, OUTPUT);   //Définit le pin en tant que sortie
    pinMode(VERT, OUTPUT);
    pinMode(BLEU, OUTPUT);
}

void loop() {
    rgb_led(0, 1, 1, 50);
    //delay(10000);
}

void rgb_led(int r, int g, int b, int millise) {
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
    delay(millise);
    
    digitalWrite(ROUGE, LOW);
    digitalWrite(VERT, LOW);
    digitalWrite(BLEU, LOW);
    delay(millise);
    //delay(1/(freq*2));
}
