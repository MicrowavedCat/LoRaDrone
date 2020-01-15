#include "../header/moteur.h"

/* Pin sur lequel on branche le moteur */
#define PIN 4

void main(int argc, char const argv*[]){
  int position = 0, angle = 0;
  pinMode(PIN, PWM_OUTPUT);
  pwmSetMode(PWM_MOD_MS);
  pwmSetClock(192);
  angle = strtod(argv[1], NULL);
  position = angle + 60;
  pwmWrite(PIN, position);
  exit(0);
}
