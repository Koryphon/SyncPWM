#include <SyncPWM.h>

SyncPWM pwm;

void setup() {
  pwm.begin(8); // Start a synchronized PWM with sync on pin 8
                // On leonardo, pin 8 is required, work in progress
                // The PWM itself is on pin 3 on Uno/Nano
                // and on pin 6 on Leonardo
  pwm.analogWrite(30);
}

void loop() {
}
