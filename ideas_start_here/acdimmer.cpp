
#include "acdimmer.h"
#include "Arduino.h"
#include "TimerOne.h"
#include "SimpleTimer.h"



#define ACTIVATION_PIN       (11)
#define SLEEP_PIN            (10)
#define STOP_PIN             (9)

//-----------------------------------------------------------------------------------
#define MAX_LIGHT_NUM (12)
#define MAX_DIM_LEVEL (128)


#define SLEEP_STATE_MAX_BRIGHTNESS       (94)
#define SLEEP_STATE_MIN_BRIGHTNESS       (10)
#define SLEEP_STATE_BRIGHTNESS_DELTA     (SLEEP_STATE_MAX_BRIGHTNESS-SLEEP_STATE_MIN_BRIGHTNESS)
#define SLEEP_STATE_BRIGHTNESS_OFFSET    (SLEEP_STATE_BRIGHTNESS_DELTA/6)
#define SLEEP_STATE_INC_INTERVAL_MS      (30)


#define ACTIVE_STATE_MAX_BRIGHTNESS      (120)
#define ACTIVE_STATE_MIN_BRIGHTNESS      (12)

#define ACTIVE_STATE_BRIGHTNESS_DELTA    (ACTIVE_STATE_MAX_BRIGHTNESS-ACTIVE_STATE_MIN_BRIGHTNESS)
#define ACTIVE_STATE_BRIGHTNESS_OFFSET   (ACTIVE_STATE_BRIGHTNESS_DELTA/12)

#define ACTIVE_STATE_INIT_BRIGHTNESS     (12)

#define ACTIVE_STATE_INC_INTERVAL_MS          (15)

#define STABLE_ACTIVE_INC_INTERVAL_MS     (5)
#define STABLE_ACTIVE_MAX_BRIGHTNESS      (126)
#define STABLE_ACTIVE_MIN_BRIGHTNESS      (26)
#define STABLE_ACTIVE_BRIGHTNESS_DELTA    (STABLE_ACTIVE_MAX_BRIGHTNESS-STABLE_ACTIVE_MIN_BRIGHTNESS)
#define STABLE_ACTIVE_BRIGHTNESS_OFFSET     (STABLE_ACTIVE_BRIGHTNESS_DELTA/5)


#define ACTIVE_STATE_NUM_OF_TILT          (5)




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

static int          motorCtrlTimerId;

static int activeSequenceTiming[ACTIVE_STATE_NUM_OF_TILT] = {5000, 9000, 6000, 3000, 1000};

static int activeSequenceCount = 0;

static ish_state_t m_current_state = SLEEP_STATE;
SimpleTimer timer;

static void motor_ctrl_bwd(void);
static void motor_ctrl_fwd(void);


//----------------------------
//Dimming Variables
static unsigned int w_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
static unsigned int x_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
static unsigned int y_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
static unsigned int z_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
static unsigned int a_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
static unsigned int b_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
static unsigned int c_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
static unsigned int d_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
static unsigned int e_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
static unsigned int f_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
static unsigned int g_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
static unsigned int h_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;

static int w_inc = 0;
static int x_inc = 0;
static int y_inc = 0;
static int z_inc = 0;
static int a_inc = 0;
static int b_inc = 0;
static int c_inc = 0;
static int d_inc = 0;
static int e_inc = 0;
static int f_inc = 0;
static int g_inc = 0;
static int h_inc = 0;

static int *inc_array[MAX_LIGHT_NUM] = {&w_inc,&x_inc,&y_inc,&z_inc,&a_inc,&b_inc,
                                        &c_inc,&d_inc,&e_inc,&f_inc,&g_inc,&h_inc};

static int *level_array[MAX_LIGHT_NUM] = {&w_level,&x_level,&y_level,&z_level,&a_level,&b_level,
                                         &c_level,&d_level,&e_level,&f_level,&g_level,&h_level};


static void control_pins_init(void)
{
  pinMode(ACTIVATION_PIN, OUTPUT);
  pinMode(SLEEP_PIN, OUTPUT);
  pinMode(STOP_PIN, OUTPUT);

  digitalWrite(ACTIVATION_PIN, HIGH);
  digitalWrite(SLEEP_PIN, HIGH);
  digitalWrite(STOP_PIN, HIGH);
}

void acdimmer_init(unsigned int num_of_lights, unsigned int * p_pins)
{
  num_of_bulbs = num_of_lights;
  for(int i = 0; i < num_of_lights; i++)
  {
    pinMode(p_pins[i], OUTPUT);
    light_pin[i] = p_pins[i];
  } // Set all the TRIAC pins as output
  Timer1.initialize(freqStep);                      // Initialize TimerOne library for the freq we need
  control_pins_init();
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

static void active_state_cb(void)
{
    static unsigned int step_index = 0;
    static unsigned int tilt_index = 0;
    static unsigned int call_counter = 0;

    call_counter++;

    //---------------------------------
    const unsigned int halfway_pt = (ACTIVE_STATE_BRIGHTNESS_DELTA);

    int bwd_pin_nums[MAX_LIGHT_NUM] = {0,7,9,11,4,2,10,5,3,8,1,6};
    int fwd_pin_nums[MAX_LIGHT_NUM] = {6,1,8,3,5,10,2,4,11,9,7,0};


    for(int i = 0; i < MAX_LIGHT_NUM; i++)
    {
      if (step_index == i*ACTIVE_STATE_BRIGHTNESS_OFFSET/2)
      {
        *(inc_array[i]) = 1;
      }
      else if (step_index == i*ACTIVE_STATE_BRIGHTNESS_OFFSET/2 + halfway_pt+1)
      {
        *(inc_array[i]) = -1;
      }
      else if (step_index == i*ACTIVE_STATE_BRIGHTNESS_OFFSET/2 + 2*halfway_pt+1)
      {
        *(inc_array[i]) = 0;
      }

      *(level_array[i]) += *(inc_array[i]);

      if(tilt_index%2)
      {
        acdimmer_bulb_set(bwd_pin_nums[i],*(level_array[i]));
      }
      else
      {
        acdimmer_bulb_set(fwd_pin_nums[i],*(level_array[i]));
      }
    }

    //{5000, 9000, 6000, 3000, 1000}
    //15 ms per step

    step_index++;
    if (step_index == 11*ACTIVE_STATE_BRIGHTNESS_OFFSET/2 + 2*halfway_pt+2)
    {
      Serial.println(step_index);
    }

    //everything's done, go to next tilt

    if (step_index == 267)
    {
      step_index = 0;
      for(int i = 0; i < MAX_LIGHT_NUM; i++)
      {
        *(level_array[i]) = ACTIVE_STATE_INIT_BRIGHTNESS;
      }

      tilt_index++;

      if(tilt_index < ACTIVE_STATE_NUM_OF_TILT+1)
      {
        generalTimerId = timer.setTimer(activeSequenceTiming[tilt_index]/267,
                                        active_state_cb,
                                        267);
      }
    }
  // }
}


static void stable_active_state_cb(void)
{

  static unsigned int step_index = 0;

  //---------------------------------

  const unsigned int halfway_pt = (SLEEP_STATE_BRIGHTNESS_DELTA);


  for(int i = 0; i < 5; i++)
  {
    if (step_index == i*STABLE_ACTIVE_BRIGHTNESS_OFFSET)
    {
      *(inc_array[i]) = 1;
    }
    else if (step_index == i*STABLE_ACTIVE_BRIGHTNESS_OFFSET + halfway_pt+1)
    {
      *(inc_array[i]) = -1;
    }
    else if (step_index == i*STABLE_ACTIVE_BRIGHTNESS_OFFSET + 2*halfway_pt+1)
    {
      *(inc_array[i]) = 0;
    }

    *(level_array[i]) += *(inc_array[i]);
  }

  //Head
  acdimmer_bulb_set(5,w_level);
  acdimmer_bulb_set(4,w_level);

  acdimmer_bulb_set(3,x_level);
  acdimmer_bulb_set(11,x_level);

  acdimmer_bulb_set(9,y_level);
  acdimmer_bulb_set(8,y_level);

  acdimmer_bulb_set(7,z_level);
  acdimmer_bulb_set(1,z_level);

  acdimmer_bulb_set(6,a_level);
  acdimmer_bulb_set(0,a_level);


  acdimmer_bulb_set(10,128);
  acdimmer_bulb_set(2,128);

  step_index++;

  //everything's done, go back to beginning
  if (step_index == 4*STABLE_ACTIVE_BRIGHTNESS_OFFSET+2*halfway_pt+2 + 10)
  {
    step_index = 0;
    for(int i = 0; i < 5; i++)
    {
      *(level_array[i]) = STABLE_ACTIVE_MIN_BRIGHTNESS;
    }
  }

}

void all_state_variables_reset(void)
{
  w_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
  x_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
  y_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
  z_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
  a_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
  b_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
  c_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
  d_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
  e_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
  f_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
  g_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;
  h_level = SLEEP_STATE_MIN_BRIGHTNESS - 1;

  w_inc = 0;
  x_inc = 0;
  y_inc = 0;
  z_inc = 0;
  a_inc = 0;
  b_inc = 0;
  c_inc = 0;
  d_inc = 0;
  e_inc = 0;
  f_inc = 0;
  g_inc = 0;
  h_inc = 0;

  digitalWrite(ACTIVATION_PIN, HIGH);
  digitalWrite(SLEEP_PIN, HIGH);
  digitalWrite(STOP_PIN, HIGH);

  activeSequenceCount = 0;
  timer.disable(generalTimerId);
}

void bulb_timer_run(void)
{
  timer.run();
}

void sleep_state_cb(void)
{
  static unsigned int step_index = 0;

  //---------------------------------

  const unsigned int halfway_pt = (SLEEP_STATE_BRIGHTNESS_DELTA);

  for(int i = 0; i < 6; i++)
  {
    if (step_index == i*SLEEP_STATE_BRIGHTNESS_OFFSET)
    {
      *(inc_array[i]) = 1;
    }
    else if (step_index == i*SLEEP_STATE_BRIGHTNESS_OFFSET + halfway_pt+1)
    {
      *(inc_array[i]) = -1;
    }
    else if (step_index == i*SLEEP_STATE_BRIGHTNESS_OFFSET + 2*halfway_pt+1)
    {
      *(inc_array[i]) = 0;
    }

    *(level_array[i]) += *(inc_array[i]);
  }

  //Head
  acdimmer_bulb_set(7,w_level);
  acdimmer_bulb_set(1,w_level);

  acdimmer_bulb_set(0,x_level);
  acdimmer_bulb_set(6,x_level);

  acdimmer_bulb_set(9,y_level);
  acdimmer_bulb_set(8,y_level);

  acdimmer_bulb_set(3,z_level);
  acdimmer_bulb_set(11,z_level);

  acdimmer_bulb_set(5,a_level);
  acdimmer_bulb_set(4,a_level);

  acdimmer_bulb_set(10,b_level);
  acdimmer_bulb_set(2,b_level);

  step_index++;

  //everything's done, go back to beginning
  if (step_index == 5*SLEEP_STATE_BRIGHTNESS_OFFSET+2*halfway_pt+2 + 10)
  {
    step_index = 0;
    for(int i = 0; i < 6; i++)
    {
      *(level_array[i]) = SLEEP_STATE_MIN_BRIGHTNESS;
    }
  }
}

static void motor_ctrl_bwd(void)
{
  Serial.println("BWD");
  activeSequenceCount++;
  if (activeSequenceCount <= 5)
  {
    digitalWrite(ACTIVATION_PIN, HIGH);
    motorCtrlTimerId = timer.setTimeout(activeSequenceTiming[activeSequenceCount-1], motor_ctrl_fwd);
  }
  else
  {
    activeSequenceCount = 0;
    digitalWrite(STOP_PIN, LOW);
    generalTimerId = timer.setInterval(STABLE_ACTIVE_INC_INTERVAL_MS, stable_active_state_cb);
    timer.setTimeout(20000, sleep_state_enter);
  }
}

static void motor_ctrl_fwd(void)
{
  Serial.println("FWD");
  activeSequenceCount++;
  if (activeSequenceCount <= 5)
  {
    digitalWrite(ACTIVATION_PIN, LOW);
    motorCtrlTimerId = timer.setTimeout(activeSequenceTiming[activeSequenceCount-1], motor_ctrl_bwd);
  }
  else
  {
    activeSequenceCount = 0;
    digitalWrite(STOP_PIN, LOW);
    generalTimerId = timer.setInterval(STABLE_ACTIVE_INC_INTERVAL_MS, stable_active_state_cb);
    timer.setTimeout(20000, sleep_state_enter);
  }
}

void active_state_enter(void)
{
  all_state_variables_reset();
  m_current_state = ACTIVE_STATE;
  digitalWrite(ACTIVATION_PIN, LOW);
  acdimmer_bulb_all_set(ACTIVE_STATE_INIT_BRIGHTNESS);
  motor_ctrl_fwd();
  generalTimerId = timer.setTimer(activeSequenceTiming[0]/267, active_state_cb, 267);
}

void sleep_state_enter (void)
{
  all_state_variables_reset();
  m_current_state = SLEEP_STATE;
  digitalWrite(SLEEP_PIN, LOW);

  acdimmer_bulb_all_set(SLEEP_STATE_MIN_BRIGHTNESS);
  generalTimerId = timer.setInterval(SLEEP_STATE_INC_INTERVAL_MS, sleep_state_cb);
}

ish_state_t state_get(void)
{
  return m_current_state;
}
