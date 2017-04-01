// MakeShields Motor shield library
// copyright MakeShields, 2010
// this code is public jacky, enjoy!

#include <MSMotorShield.h>


MS_DCMotor motor4(4); //Create a motor instance, connected to port 4
MS_DCMotor motor3(3); //Create a motor instance, connected to port 3
MS_DCMotor motor2(2); //Create a motor instance, connected to port 2
MS_DCMotor motor1(1); //Create a motor instance, connected to port 1

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println("Motor test!");

  // turn on motor
  allMotorsSetSpeed(200);
  allMotorsRun(FORWARD);
}

void loop() {
 //  uint8_t i;
 //
 //  Serial.print("tick");
 //
 //  motor.run(FORWARD);
 //  for (i=0; i<255; i++) {
 //    motor.setSpeed(i);
 //    delay(10);
 // }
 //
 //  for (i=255; i!=0; i--) {
 //    motor.setSpeed(i);
 //    delay(10);
 // }
 //
 //  Serial.print("tock");
 //
 //  motor.run(BACKWARD);
 //  for (i=0; i<255; i++) {
 //    motor.setSpeed(i);
 //    delay(10);
 // }
 //
 //  for (i=255; i!=0; i--) {
 //    motor.setSpeed(i);
 //    delay(10);
 // }
 //
 //
 //  Serial.print("tech");
 //  motor.run(RELEASE);
 //  delay(1000);
}

void allMotorsSetSpeed( int speed )
{
  motor4.setSpeed(speed);
  motor3.setSpeed(speed);
  motor2.setSpeed(speed);
  motor1.setSpeed(speed);
}

void allMotorsRun(int direction)
{
  motor4.run(direction);
  motor3.run(direction);
  motor2.run(direction);
  motor1.run(direction);
}
