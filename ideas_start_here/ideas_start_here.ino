#include "acdimmer.h"


#define NUM_OF_LIGHTS (12)
static unsigned int pin[NUM_OF_LIGHTS] = {0};


void setup()
{
  // put your setup code here, to run once:
 // Set up the light bulb control pins
 for (int i = 0 ; i< NUM_OF_LIGHTS; i++)
 {
   pin[i] = i + 22; //assign pins 22 - 33 to control light dimming
 }
 acdimmer_init(NUM_OF_LIGHTS,pin);
}

void loop()
{
  // put your main code here, to run repeatedly:
}
