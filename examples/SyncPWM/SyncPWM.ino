#include <SyncPWM.h>

SyncPWM pwm;

void setup() {
  pwm.begin(8); // Start a synchronized PWM with sync on pin 8
                // The PWM itself is on pin 3
  pwm.analogWrite(30);
}

void loop() {
}
