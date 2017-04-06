// MakeShields Motor shield library
// copyright MakeShields, 2010
// this code is public domain, enjoy!

#include <MSMotorShield.h>
#include "ish_states.h"
// #define MICROSTEPPING
#define STEP 20
#define FORWARD_LIMIT_PIN    (A1)
#define BACKWARD_LIMIT_PIN   (A0)

#define ACTIVATION_PIN       (A2)
#define SLEEP_PIN            (A3)
#define STOP_PIN             (A4)

MS_Stepper motor(200, 1); //200 steps/rev, stepper number 1.
bool homing = false;
unsigned int total_steps = 0;
int step_position = 0;

ish_state_t m_current_state = SLEEP_STATE;

void setup() {
  Serial.begin(112500);           // set up Serial library at 9600 bps
  // Serial.println("Stepper test!");
  digitalWrite(FORWARD_LIMIT_PIN, INPUT_PULLUP);
  digitalWrite(BACKWARD_LIMIT_PIN, INPUT_PULLUP); //These are pulled LOW when the foam board touches either limit

  motor.setSpeed(2);  // 10 rpm

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
  step_position = 0;
  motor.release();
}

void loop()
{
  if((digitalRead(ACTIVATION_PIN) == LOW) && (digitalRead(STOP_PIN) == HIGH))
  {
    active_state_enter();
  }
  if((digitalRead(SLEEP_PIN) == LOW) && (digitalRead(STOP_PIN) == HIGH))
  {
    sleep_state_enter();
  }
  if((digitalRead(STOP_PIN) == LOW))
  {
    motor.release();
    Serial.println("STOP");
  }
}

void active_state_enter(void)
{
  Serial.println("Active State");
  m_current_state = ACTIVE_STATE;

  while((digitalRead(STOP_PIN) == HIGH) && (digitalRead(SLEEP_PIN) == HIGH))//
  {
    while((digitalRead(ACTIVATION_PIN) == LOW) && (!fwd_limit_reached()) && (digitalRead(STOP_PIN) == HIGH))
    {
      Serial.println("Active Fwd");
      motor.step(1,FORWARD, INTERLEAVE);
      step_position++;
    }
    while((digitalRead(ACTIVATION_PIN) == HIGH) && (!bwd_limit_reached()) && (digitalRead(STOP_PIN) == HIGH))
    {
      Serial.println("Active Bwd");
      motor.step(1,BACKWARD, INTERLEAVE);
      step_position--;
    }
  }
}

void sleep_state_enter(void)
{
  Serial.println("Sleep State");
  m_current_state = SLEEP_STATE;

  while((step_position != 0) && digitalRead(STOP_PIN) == HIGH)
  {
    if (step_position > 0)
    {
      Serial.println("Sleep Bwd");
      motor.step(1, BACKWARD, INTERLEAVE);
      step_position--;
    }
    else if (step_position < 0)
    {
      Serial.println("Sleep Fwd");
      motor.step(1, FORWARD, INTERLEAVE);
      step_position++;
    }
  }
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
