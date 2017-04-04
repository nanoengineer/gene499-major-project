// MakeShields Motor shield library
// copyright MakeShields, 2010
// this code is public domain, enjoy!

#include <MSMotorShield.h>

// #define MICROSTEPPING
#define STEP 20
#define FORWARD_LIMIT_PIN    (A4)
#define BACKWARD_LIMIT_PIN   (A5)

MS_Stepper motor(200, 1); //200 steps/rev, stepper number 1.
bool homing = false;
unsigned int total_steps = 0;

void setup() {
  // Serial.begin(9600);           // set up Serial library at 9600 bps
  // Serial.println("Stepper test!");
  pinMode(FORWARD_LIMIT_PIN, INPUT_PULLUP);
  pinMode(BACKWARD_LIMIT_PIN, INPUT_PULLUP); //These are pulled LOW when the foam board touches either limit

  motor.setSpeed(5);  // 10 rpm

  homing = true;

  while(homing)
  {
    if(!fwd_limit_reached() && !bwd_limit_reached())
    {
      go_to_fwd_limit();
      total_steps = go_to_bwd_limit();
      motor.step(total_steps/2,FORWARD, INTERLEAVE);
      homing = false;
    }

    else if (fwd_limit_reached())
    {
      total_steps = go_to_bwd_limit();
      motor.step(total_steps/2,FORWARD, INTERLEAVE);
      homing = false;
    }

    else if (bwd_limit_reached())
    {
      total_steps = go_to_fwd_limit();
      motor.step(total_steps/2, BACKWARD, INTERLEAVE);
      homing = false;
    }
  }

}

void loop() {


//  Serial.println("Single coil steps");
//  motor.step(5, FORWARD, SINGLE);
//  motor.step(5, BACKWARD, SINGLE);
////
//  Serial.println("Double coil steps");
//  motor.step(5, FORWARD, DOUBLE);
//  motor.step(5, BACKWARD, DOUBLE);

  // Serial.println("Interleave coil steps");
  // motor.step(STEP/2, FORWARD, INTERLEAVE);
  // motor.step(STEP, BACKWARD, INTERLEAVE);
  // motor.step(STEP/2, FORWARD, INTERLEAVE);

//#ifdef MICROSTEPPING
//  Serial.println("Micrsostep steps");
//  motor.step(10, FORWARD, MICROSTEP);
//  motor.step(10, BACKWARD, MICROSTEP);
//#endif
}

bool fwd_limit_reached(void)
{
  return !digitalRead(FORWARD_LIMIT_PIN);
}

bool bwd_limit_reached(void)
{
  return !digitalRead(BACKWARD_LIMIT_PIN);
}

unsigned int go_to_fwd_limit(void)
{
  unsigned int steps = 0;
  do
  {
    motor.step(1,FORWARD,INTERLEAVE);
    steps++;
  } while (!fwd_limit_reached());
  return steps;
}

unsigned int go_to_bwd_limit(void)
{
  unsigned int steps = 0;
  do
  {
    motor.step(1,BACKWARD,INTERLEAVE);
    steps++;
  } while (!bwd_limit_reached());
  return steps;
}
