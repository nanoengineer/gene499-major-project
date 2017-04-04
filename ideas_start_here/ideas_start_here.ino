#include "acdimmer.h"
#include "SimpleTimer.h"
#include "NewPing.h"

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
#define PING_INTERVAL 250 // Milliseconds between sensor pings (29ms is about the min to avoid cross-sensor echo).

static unsigned int pin[NUM_OF_LIGHTS] = {0};
static unsigned int test_brightness[NUM_OF_LIGHTS] = {0,120,125,128,50,60,70,80,90,100,110};
static unsigned int off_array[NUM_OF_LIGHTS] = {OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL,OFF_VAL};

unsigned long pingTimer[SONAR_NUM]; // Holds the times when the next ping should happen for each sensor.
unsigned int cm[SONAR_NUM];         // Where the ping distances are stored.
uint8_t currentSensor = 0;          // Keeps track of which sensor is active.

NewPing sonar[SONAR_NUM] = {     // Sensor object array.
  NewPing(SENSOR_L_TRIG, SENSOR_L_ECHO, SENSOR_L_MAX_DIST), // Each sensor's trigger pin, echo pin, and max distance to ping.
  NewPing(SENSOR_R_TRIG, SENSOR_R_ECHO, SENSOR_R_MAX_DIST)
};

//NOTE: define brightness from 0-128
void setup()
{
  pingSensorInit(void);
  // put your setup code here, to run once:
 // Set up the light bulb control pins
 for (int i = 0 ; i< NUM_OF_LIGHTS; i++)
 {
   pin[i] = i + 22; //assign pins 22 - 33 to control light dimming
 }

  acdimmer_init(NUM_OF_LIGHTS,pin);
  acdimmer_bulb_array_set(off_array);
  acdimmer_enable();
}

void loop()
{
  pingSensorRun();
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
  for (uint8_t i = 0; i < SONAR_NUM; i++) {
    Serial.print(i);
    Serial.print("=");
    Serial.print(cm[i]);
    Serial.print("cm ");
  }
  Serial.println();
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
      cm[currentSensor] = 0;                      // Make distance zero in case there's no ping echo for this sensor.
      sonar[currentSensor].ping_timer(echoCheck); // Do the ping (processing continues, interrupt will call echoCheck to look for echo).
    }
  }
}
