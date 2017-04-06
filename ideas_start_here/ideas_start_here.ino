#include "acdimmer.h"
#include "SimpleTimer.h"
#include "Average.h"
#include "ish_states.h"

//Lightbulb Settings
#define NUM_OF_LIGHTS (12)
#define OFF_VAL (10)

#define MIC_RIGHT_PIN       (A1)

#define SAMPLE_FILTER_COUNT (50)

static unsigned int pin[NUM_OF_LIGHTS] = {0};
static unsigned int test_brightness[NUM_OF_LIGHTS] = {0,120,125,128,50,60,70,80,90,100,110};
static unsigned int off_array[NUM_OF_LIGHTS] = {OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL};

Average<int> right_stats(SAMPLE_FILTER_COUNT);

SimpleTimer m_timer;

int m_mic_right_baseline = 0;
int micTimerId;

//NOTE: define brightness from 0-128
void setup()
{
  Serial.begin(115200);

  // put your setup code here, to run once:
 // Set up the light bulb control pins
 for (int i = 0 ; i< NUM_OF_LIGHTS; i++)
 {
   pin[i] = i + 22; //assign pins 22 - 33 to control light dimming
 }
  acdimmer_init(NUM_OF_LIGHTS,pin);
  acdimmer_enable();
  sleep_state_enter();
  m_timer.setTimeout(22000, mic_init);
}

void loop()
{
  bulb_timer_run();
  m_timer.run();
}

void mic_init(void)
{
  for(int i = 0; i < SAMPLE_FILTER_COUNT; i++)
  {
      // left_stats.push(analogRead(MIC_LEFT_PIN));
      right_stats.push(analogRead(MIC_RIGHT_PIN));
      delay(100);
  }

  // m_mic_left_baseline = left_stats.mode();
  m_mic_right_baseline = right_stats.mode();
  micTimerId =  m_timer.setInterval(50,mic_sample);
}

void mic_sample(void)
{
  right_stats.push(analogRead(MIC_RIGHT_PIN));
  // Serial.print("Mode:");
  // Serial.println(stats.mode());

  //Serial.print("Delta:");
  int data = analogRead(MIC_RIGHT_PIN) - right_stats.mode();

  int scaled_data = (abs(data))*(abs(data));
  Serial.println(scaled_data);

if(state_get() == SLEEP_STATE)
  {
    if(scaled_data > 50)
    {
      active_state_enter();
    }
  }
}
