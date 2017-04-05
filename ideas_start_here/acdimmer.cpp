
#include "acdimmer.h"
#include "Arduino.h"
#include "TimerOne.h"
#include "SimpleTimer.h"

#define MAX_LIGHT_NUM (12)
#define MAX_DIM_LEVEL (128)

#define SINGLE_STATE_TIME_STEP (350)
#define SINGLE_STATE_NUM_OF_STEPS (120)
#define SINGLE_STATE_BRIGHTNESS_STEP (5)
#define SINGLE_STATE_INIT_BRIGHTNESS (50)
#define DOUBLE_STATE_INIT_BRIGHTNESS (50)

#define SLEEP_STATE_MAX_BRIGHTNESS       (80)
#define SLEEP_STATE_MIN_BRIGHTNESS       (20)
#define SLEEP_STATE_BRIGHTNESS_DELTA     (SLEEP_STATE_MAX_BRIGHTNESS-SLEEP_STATE_MIN_BRIGHTNESS)
#define SLEEP_STATE_BRIGHTNESS_OFFSET    (SLEEP_STATE_BRIGHTNESS_DELTA/4)

#define SLEEP_STATE_INC_INTERVAL_MS      (30)
#define SLEEP_STATE_BRIGHTNESS_INC       (1)  //inc brightness by 1 every interval_ms

#define SLEEP_STATE_PERIOD_MS            (SLEEP_STATE_INC_INTERVAL_MS*(7+1)) //(7 + x), x being how much idle time

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
static int          generalTimerId;
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

static unsigned int convert_brightness_to_dim(unsigned int brightness)
{
    unsigned int dim_level = 0;

    if(brightness <= 128 && brightness >= OFF_VAL)
    {
      dim_level = MAX_DIM_LEVEL - brightness;
    }
    else if (brightness < OFF_VAL)
    {
      dim_level = MAX_DIM_LEVEL - OFF_VAL;
    }
    else
    {
      dim_level = 0;
    }

    return dim_level;
}

void acdimmer_bulb_set(unsigned int light_id, unsigned int brightness)
{

      dim_level[light_id] = convert_brightness_to_dim(brightness);
}

void acdimmer_bulb_array_set(unsigned int * p_brightness)
{
    for (int i = 0; i < num_of_bulbs; i++)
    {
        dim_level[i] = convert_brightness_to_dim(p_brightness[i]);
    }
}

void acdimmer_bulb_all_set(unsigned int brightness)
{
    for (int i = 0; i < num_of_bulbs; i++)
    {
      dim_level[i] = convert_brightness_to_dim(brightness);
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
  // Serial.println("zc");
}

// Turn on the TRIAC at the appropriate time
static void dim_check(void)
{
  //Serial.println("dc");
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
  attachInterrupt(2, zero_cross_detect, RISING);   // Attach an Interupt to Pin 2 (interupt 0) for Zero Cross Detection
}


// static void path_trace(unsigned int * p_path, unsigned int node_count, long path_time)
// {
//
//     for (unsigned int i = 0; i < node_count; i++)
//     {
//
//     }
// }

static void left_state_cb(void)
{
  static unsigned int step_index = 1;

  unsigned int order_array[MAX_LIGHT_NUM] = {0,7,9,11,4,2,10,5,3,8,1,6};

  const unsigned int head_brightness = 70;
  const unsigned int head_index = step_index - 1;

  //Serial.println(step_index);
  for (unsigned int i = 0; i < step_index; i++)
  {
    //Serial.println(head_index);
    if (i == (head_index))
    {
      acdimmer_bulb_set(order_array[i], head_brightness);
    }
    else
    {
      acdimmer_bulb_set(order_array[i], (head_brightness - 15*(head_index-i)));
    }
  }

  step_index++;

  if (step_index == 13)
  {
    step_index = 1;
    acdimmer_bulb_all_set(SINGLE_STATE_INIT_BRIGHTNESS);
  }
  // }
}

static void right_state_cb(void)
{
  static unsigned int step_index = 1;

  unsigned int order_array[MAX_LIGHT_NUM] = {6,1,8,3,5,10,2,4,11,9,7,0};

  const unsigned int head_brightness = 70;
  const unsigned int head_index = step_index - 1;

  //Serial.println(step_index);
  for (unsigned int i = 0; i < step_index; i++)
  {
    //Serial.println(head_index);
    if (i == (head_index))
    {
      acdimmer_bulb_set(order_array[i], head_brightness);
    }
    else
    {
      acdimmer_bulb_set(order_array[i], (head_brightness - 15*(head_index-i)));
    }
  }
  step_index++;

  if (step_index == 13)
  {
    step_index = 1;
    acdimmer_bulb_all_set(SINGLE_STATE_INIT_BRIGHTNESS);
  }
  // }
}

void double_state_cb(void)
{
  static unsigned int level = 20;
  static int inc = 1;

  level+=inc;
  if((level>=128) || (level<=20))
    inc*=-1;

  acdimmer_bulb_all_set(level);
}

static void sleep_state_cb(void)
{
  //Length wise there are 8 rows of symmetric bulbs
  //o--o--o--o--o--o--o--o
  //w  x  y  z  z  y  x  w

  static unsigned int time_ms = 0;

  static unsigned int w_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
  static unsigned int x_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
  static unsigned int y_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
  static unsigned int z_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;

  // static bulb_state_t w_state = IDLE;
  // static bulb_state_t x_state = IDLE;
  // static bulb_state_t y_state = IDLE;
  // static bulb_state_t z_state = IDLE;

  static int w_inc = 0;
  static int x_inc = 0;
  static int y_inc = 0;
  static int z_inc = 0;

  static unsigned int step_index = 0;

  //---------------------------------

  const unsigned int halfway_pt = (SLEEP_STATE_BRIGHTNESS_DELTA);


  if (step_index == 0)
  {
    w_inc = 1;
  }
  if (step_index == (halfway_pt+1))
  {
    w_inc = -1;
  }
  if (step_index == 2*halfway_pt+1)
  {
    w_inc = 0;
  }
  if (step_index == SLEEP_STATE_BRIGHTNESS_OFFSET)
  {
    x_inc = 1;
  }
  if (step_index == SLEEP_STATE_BRIGHTNESS_OFFSET + halfway_pt+1)
  {
    x_inc = -1;
  }
  if (step_index == SLEEP_STATE_BRIGHTNESS_OFFSET + 2*halfway_pt+1)
  {
    x_inc = 0;
  }
  if (step_index == 2*SLEEP_STATE_BRIGHTNESS_OFFSET)
  {
    y_inc = 1;
  }
  if (step_index == 2*SLEEP_STATE_BRIGHTNESS_OFFSET + halfway_pt+1)
  {
    y_inc = -1;
  }
  if (step_index == 2*SLEEP_STATE_BRIGHTNESS_OFFSET + 2*halfway_pt+1)
  {
    y_inc = 0;
  }
  if (step_index == 3*SLEEP_STATE_BRIGHTNESS_OFFSET)
  {
    z_inc = 1;
  }
  if (step_index == 3*SLEEP_STATE_BRIGHTNESS_OFFSET + halfway_pt+1)
  {
    z_inc = -1;
  }
  if (step_index == 3*SLEEP_STATE_BRIGHTNESS_OFFSET + 2*halfway_pt+1)
  {
    z_inc = 0;
  }

  //Increment all the brightness levels
  w_level += w_inc;
  x_level += x_inc;
  y_level += y_inc;
  z_level += z_inc;

  Serial.println(w_level);
  Serial.println(x_level);
  Serial.println(y_level);
  Serial.println(z_level);
  Serial.println(".");

  //Head
  acdimmer_bulb_set(0,w_level);
  acdimmer_bulb_set(7,w_level);
  acdimmer_bulb_set(1,w_level);
  acdimmer_bulb_set(6,w_level);

  acdimmer_bulb_set(9,x_level);
  acdimmer_bulb_set(8,x_level);

  acdimmer_bulb_set(3,y_level);
  acdimmer_bulb_set(11,y_level);
  acdimmer_bulb_set(5,y_level);
  acdimmer_bulb_set(4,y_level);

  acdimmer_bulb_set(10,z_level);
  acdimmer_bulb_set(2,z_level);

  step_index++;

  //everything's done, go back to beginning
  if (step_index == 3*SLEEP_STATE_BRIGHTNESS_OFFSET+2*halfway_pt+2)
  {
    step_index = 0;
    w_level = SLEEP_STATE_MIN_BRIGHTNESS;
    x_level = SLEEP_STATE_MIN_BRIGHTNESS;
    y_level = SLEEP_STATE_MIN_BRIGHTNESS;
    z_level = SLEEP_STATE_MIN_BRIGHTNESS;
  }

}


void single_state_enter( ish_state_t state )
{
  // timer.disable(generalTimerId);

  if (state == SINGLE_LEFT_STATE)
  {
      m_current_state = SINGLE_LEFT_STATE;

      acdimmer_bulb_all_set(SINGLE_STATE_INIT_BRIGHTNESS);
      generalTimerId = timer.setInterval(SINGLE_STATE_TIME_STEP, left_state_cb);
  }
  else if (state == SINGLE_RIGHT_STATE)
  {
      m_current_state = SINGLE_RIGHT_STATE;

      acdimmer_bulb_all_set(SINGLE_STATE_INIT_BRIGHTNESS);
      generalTimerId = timer.setInterval(SINGLE_STATE_TIME_STEP, right_state_cb);
  }
}

void timer_run(void)
{
  timer.run();
}

void double_state_enter(void)
{
  // timer.disable(generalTimerId);
  m_current_state = DOUBLE_STATE;
  acdimmer_bulb_all_set(DOUBLE_STATE_INIT_BRIGHTNESS);

  generalTimerId = timer.setInterval(50, double_state_cb);
}

void sleep_state_enter (void)
{
  // timer.disable(generalTimerId);
  m_current_state = SLEEP_STATE;
  acdimmer_bulb_all_set(SLEEP_STATE_MIN_BRIGHTNESS);
  generalTimerId = timer.setInterval(SLEEP_STATE_INC_INTERVAL_MS, sleep_state_cb);
}
