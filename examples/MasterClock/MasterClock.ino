#include <SyncPWM.h>

SyncPWM sp;

void setup() {
  sp.beginMasterClock();  // start a master clock on PWM pin 3 on Uno/Nano
                          // pin 6 on a Leonardo
}

void loop() {
}
