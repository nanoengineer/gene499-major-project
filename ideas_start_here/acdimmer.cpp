
#include "acdimmer.h"
#include "Arduino.h"
#include "TimerOne.h"
#include "SimpleTimer.h"

#define MAX_LIGHT_NUM (12)
#define MAX_DIM_LEVEL (128)

static unsigned int light_pin[MAX_LIGHT_NUM] = {0};
static volatile unsigned int dim_counter[MAX_LIGHT_NUM] = {0};               // Variable to use as a counter
static unsigned int dim_level[MAX_LIGHT_NUM] = {0};

static volatile boolean zero_cross[MAX_LIGHT_NUM] = {0};  // Boolean to store a "switch" to tell us if we have crossed zero
// int AC_pin = 11;                // Output to Opto Triac
static unsigned int num_of_bulbs = 0;
static unsigned int freqStep = 65;    // This is the delay-per-brightness step in microseconds.
// It is calculated based on the frequency of your voltage supply (50Hz or 60Hz)
// and the number of brightness steps you want.
//
// The only tricky part is that the chopper circuit chops the AC wave twice per
// cycle, once on the positive half and once at the negative half. This meeans
// the chopping happens at 120Hz for a 60Hz supply or 100Hz for a 50Hz supply.

// To calculate freqStep you divide the length of one full half-wave of the power
// cycle (in microseconds) by the number of brightness steps.
//
// (1000000 uS / 120 Hz) / 128 brightness steps = 65 uS / brightness step
//
// 1000000 us / 120 Hz = 8333 uS, length of one half-wave.
static int          timerId[MAX_LIGHT_NUM] = {0};
static ish_state_t m_current_state = SLEEP_STATE;
SimpleTimer timer;


void acdimmer_init(unsigned int num_of_lights, unsigned int * p_pins)
{
  num_of_bulbs = num_of_lights;
  for(int i = 0; i < num_of_lights; i++)
  {
    pinMode(p_pins[i], OUTPUT);
    light_pin[i] = p_pins[i];
  } // Set all the TRIAC pins as output
  Timer1.initialize(freqStep);                      // Initialize TimerOne library for the freq we need
  Serial.println("Dimmer Init");
}

void acdimmer_bulb_set(unsigned int light_id, unsigned int brightness)
{
    if(brightness <= 128 && brightness >= OFF_VAL)
    {
      dim_level[light_id] = MAX_DIM_LEVEL - brightness;
    }
    else if (brightness < OFF_VAL)
    {
      dim_level[light_id] = OFF_VAL;
    }
}

void acdimmer_bulb_array_set(unsigned int * p_brightness)
{
    for (int i = 0; i < num_of_bulbs; i++)
    {
          if(p_brightness[i] <= 128 && p_brightness[i] >= OFF_VAL)
          {
            dim_level[i] = MAX_DIM_LEVEL - p_brightness[i];
          }
          else if (p_brightness[i] < OFF_VAL)
          {
            dim_level[i] = OFF_VAL;
          }
    }
}

static void zero_cross_detect(void)
{
  int blank[MAX_LIGHT_NUM] = {0};
  memcpy(dim_counter, blank, MAX_LIGHT_NUM);
  for (int i = 0; i < num_of_bulbs; i++)
  {
    digitalWrite(light_pin[i], LOW);       // turn off TRIAC (and AC)
    zero_cross[i] = true;
  }
  //Serial.println("Zero Cross");
}

// Turn on the TRIAC at the appropriate time
static void dim_check(void)
{
  //Serial.println("Dim Check");
  for(int i = 0; i < num_of_bulbs; i++)
  {
    if(zero_cross[i] == true)
    {
      if(dim_counter[i] >= dim_level[i])
      {
        digitalWrite(light_pin[i], HIGH); // turn on light
        dim_counter[i]=0;  // reset time step counter
        zero_cross[i] = false; //reset zero cross detection
      }
      else
      {
        dim_counter[i]++; // increment time step counter
      }
    }
  }
}

void acdimmer_enable(void)
{
  Serial.println("Dimmer Enable");
  Timer1.attachInterrupt(dim_check, freqStep);
  attachInterrupt(0, zero_cross_detect, RISING);   // Attach an Interupt to Pin 2 (interupt 0) for Zero Cross Detection
}


static void path_trace(unsigned int * p_path, unsigned int node_count, long path_time)
{

    for (unsigned int i = 0; i < node_count; i++)
    {
      
    }
}

void sleep_state_enter (void)
{

}

void single_state_enter( ish_state_t state )
{
  if (state == SINGLE_LEFT_STATE)
  {


  }
  else if (state == SINGLE_RIGHT_STATE)
  {

  }
}

void double_state_enter(void)
{

}
