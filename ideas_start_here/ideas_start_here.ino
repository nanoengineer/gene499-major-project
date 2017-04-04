#include "acdimmer.h"
#include "SimpleTimer.h"
#include "NewPing.h"
#include "Average.h"
#include "ish_states.h"

//Lightbulb Settings
#define NUM_OF_LIGHTS (12)
#define OFF_VAL (20)

//Distance Sensor Settings
#define SENSOR_L_TRIG 		13
#define SENSOR_L_ECHO 		12
#define SENSOR_L_MAX_DIST 160

#define SENSOR_R_TRIG      11
#define SENSOR_R_ECHO 	   10
#define SENSOR_R_MAX_DIST  160

#define SONAR_NUM     2 // Number of sensors.
#define PING_INTERVAL 100 // Milliseconds between sensor pings (29ms is about the min to avoid cross-sensor echo).
#define NUM_OF_PTS_FOR_TRIG 5

#define TRIG_THRESHOLD    150

static unsigned int pin[NUM_OF_LIGHTS] = {0};
static unsigned int test_brightness[NUM_OF_LIGHTS] = {0,120,125,128,50,60,70,80,90,100,110};
static unsigned int off_array[NUM_OF_LIGHTS] = {OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL};

unsigned long pingTimer[SONAR_NUM]; // Holds the times when the next ping should happen for each sensor.
unsigned int cm[SONAR_NUM];         // Where the ping distances are stored.
unsigned int cm_filter_left[NUM_OF_PTS_FOR_TRIG];
unsigned int cm_filter_right[NUM_OF_PTS_FOR_TRIG];


uint8_t currentSensor = 0;          // Keeps track of which sensor is active.

ish_state_t m_current_state = SLEEP_STATE;

NewPing sonar[SONAR_NUM] = {     // Sensor object array.
  NewPing(SENSOR_L_TRIG, SENSOR_L_ECHO, SENSOR_L_MAX_DIST), // Each sensor's trigger pin, echo pin, and max distance to ping.
  NewPing(SENSOR_R_TRIG, SENSOR_R_ECHO, SENSOR_R_MAX_DIST)
};

//NOTE: define brightness from 0-128
void setup()
{
  Serial.begin(115200);
  pingSensorInit();
  // put your setup code here, to run once:
 // Set up the light bulb control pins
 for (int i = 0 ; i< NUM_OF_LIGHTS; i++)
 {
   pin[i] = i + 22; //assign pins 22 - 33 to control light dimming
 }

  acdimmer_init(NUM_OF_LIGHTS,pin);
  acdimmer_bulb_array_set(test_brightness);
  acdimmer_enable();
  single_state_enter(SINGLE_LEFT_STATE);
}

void loop()
{
  timer_run();
  // pingSensorRun();


  // }
}

static void pingSensorInit(void)
{
  pingTimer[0] = millis() + 75;           // First ping starts at 75ms, gives time for the Arduino to chill before starting.
  for (uint8_t i = 1; i < SONAR_NUM; i++) // Set the starting time for each sensor.
    pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;
}

static void echoCheck() { // If ping received, set the sensor distance to array.
  if (sonar[currentSensor].check_timer())
    cm[currentSensor] = sonar[currentSensor].ping_result / US_ROUNDTRIP_CM;
}

static void oneSensorCycle() { // Sensor ping cycle complete, do something with the results.
  // The following code would be replaced with your code that does something with the ping results.
  static unsigned int filter_index = 0;

  for (uint8_t i = 0; i < SONAR_NUM; i++) {
    Serial.print(i);
    Serial.print("=");
    Serial.print(cm[i]);
    Serial.print("cm ");
  }
  Serial.println();


  cm_filter_left[filter_index] = cm[0];
  cm_filter_right[filter_index] = cm[1];

  filter_index++;

  if(filter_index == NUM_OF_PTS_FOR_TRIG)
  {
    filter_index = 0;
  }

  unsigned int left_sum = 0;
  unsigned int right_sum = 0;

  for (unsigned int i = 0; i < NUM_OF_PTS_FOR_TRIG; i++)
  {
    right_sum = cm_filter_right[i] + right_sum;
    left_sum = cm_filter_left[i] + left_sum;
  }

  unsigned int left_mean = left_sum/NUM_OF_PTS_FOR_TRIG;
  unsigned int right_mean = right_sum/NUM_OF_PTS_FOR_TRIG;

  Serial.print("Left Mean:");
  Serial.println(left_mean);
  Serial.print("Right Mean:");
  Serial.println(right_mean);

  if (true)
  {
    if(left_mean <= TRIG_THRESHOLD && right_mean > TRIG_THRESHOLD)
    {
      m_current_state == SINGLE_LEFT_STATE;
      single_state_enter(m_current_state);
      Serial.println("LEFT");
    }

    else if(left_mean > TRIG_THRESHOLD && right_mean <= TRIG_THRESHOLD)
    {
      m_current_state == SINGLE_RIGHT_STATE;
      single_state_enter(m_current_state);
      Serial.println("RIGHT");
    }

    else if(left_mean <= TRIG_THRESHOLD && right_mean <= TRIG_THRESHOLD)
    {
      m_current_state == DOUBLE_STATE;
      Serial.println("DOUBLE");
    }
    else
    {
      m_current_state == SLEEP_STATE;
      Serial.println("SLEEP");
    }
  }
}

static void pingSensorRun(void)
{
  for (uint8_t i = 0; i < SONAR_NUM; i++)
  { // Loop through all the sensors.
    if (millis() >= pingTimer[i]) {         // Is it this sensor's time to ping?
      pingTimer[i] += PING_INTERVAL * SONAR_NUM;  // Set next time this sensor will be pinged.
      if (i == 0 && currentSensor == SONAR_NUM - 1)
      {
        oneSensorCycle(); // Sensor ping cycle complete, do something with the results.
      }
      sonar[currentSensor].timer_stop();          // Make sure previous timer is canceled before starting a new ping (insurance).
      currentSensor = i;                          // Sensor being accessed.
      cm[currentSensor] = 200;                      // Make distance zero in case there's no ping echo for this sensor.
      sonar[currentSensor].ping_timer(echoCheck); // Do the ping (processing continues, interrupt will call echoCheck to look for echo).
    }
  }
}
