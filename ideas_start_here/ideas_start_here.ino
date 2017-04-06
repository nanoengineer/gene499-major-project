#include "acdimmer.h"
#include "SimpleTimer.h"
#include "Average.h"
#include "ish_states.h"

//Lightbulb Settings
#define NUM_OF_LIGHTS (12)
#define OFF_VAL (10)

#define MIC_LEFT_PIN        (A0)
#define MIC_RIGHT_PIN       (A1)

#define SAMPLE_FILTER_COUNT (50)

static unsigned int pin[NUM_OF_LIGHTS] = {0};
static unsigned int test_brightness[NUM_OF_LIGHTS] = {0,120,125,128,50,60,70,80,90,100,110};
static unsigned int off_array[NUM_OF_LIGHTS] = {OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL};

// unsigned long pingTimer[SONAR_NUM]; // Holds the times when the next ping should happen for each sensor.
// unsigned int cm[SONAR_NUM];         // Where the ping distances are stored.
// unsigned int cm_filter_left[NUM_OF_PTS_FOR_TRIG];
// unsigned int cm_filter_right[NUM_OF_PTS_FOR_TRIG];

Average<int> right_stats(SAMPLE_FILTER_COUNT);

SimpleTimer mic_timer;

uint8_t currentSensor = 0;          // Keeps track of which sensor is active.

ish_state_t m_current_state = SLEEP_STATE;
ish_state_t m_prev_state = SLEEP_STATE;

//int m_mic_left_baseline = 0;
int m_mic_right_baseline = 0;

// NewPing sonar[SONAR_NUM] = {     // Sensor object array.
//   NewPing(SENSOR_L_TRIG, SENSOR_L_ECHO, SENSOR_L_MAX_DIST), // Each sensor's trigger pin, echo pin, and max distance to ping.
//   NewPing(SENSOR_R_TRIG, SENSOR_R_ECHO, SENSOR_R_MAX_DIST)
// };

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


  // mic_init();
  // mic_timer.setInterval(50,mic_sample);

  delay(20000); //Wait for motor to home itself on bootup.

  acdimmer_init(NUM_OF_LIGHTS,pin);
  acdimmer_enable();
  active_state_enter();
}

void loop()
{
  bulb_timer_run();
  // mic_timer.run();
  //pingSensorRun();
  // }
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
}

void mic_sample(void)

{

  right_stats.push(analogRead(MIC_RIGHT_PIN));

  // Serial.print("Mode:");
  // Serial.println(stats.mode());

  //Serial.print("Delta:");
  int data = analogRead(MIC_RIGHT_PIN) - right_stats.mode();


  int scaled_data = abs(data)*abs(data);
  Serial.println(scaled_data);

if(m_current_state == SLEEP_STATE)
  {
    if(scaled_data > 15)
    {
      m_current_state = ACTIVE_STATE;
      Serial.println("Active State");
    }
  }
}
