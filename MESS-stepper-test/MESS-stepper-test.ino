// MakeShields Motor shield library
// copyright MakeShields, 2010
// this code is public domain, enjoy!

#include <MSMotorShield.h>

// #define MICROSTEPPING
#define STEP 20

MS_Stepper motor(200, 1); //200 steps/rev, stepper number 1.

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println("Stepper test!");

  motor.setSpeed(5);  // 10 rpm
}

void loop() {
//  Serial.println("Single coil steps");
//  motor.step(5, FORWARD, SINGLE);
//  motor.step(5, BACKWARD, SINGLE);
////
//  Serial.println("Double coil steps");
//  motor.step(5, FORWARD, DOUBLE);
//  motor.step(5, BACKWARD, DOUBLE);

  Serial.println("Interleave coil steps");
  motor.step(STEP/2, FORWARD, INTERLEAVE);
  motor.step(STEP, BACKWARD, INTERLEAVE);
  motor.step(STEP/2, FORWARD, INTERLEAVE);

//#ifdef MICROSTEPPING
//  Serial.println("Micrsostep steps");
//  motor.step(10, FORWARD, MICROSTEP);
//  motor.step(10, BACKWARD, MICROSTEP);
//#endif
}
