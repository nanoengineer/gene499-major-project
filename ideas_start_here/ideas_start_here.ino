#include "acdimmer.h"


#define NUM_OF_LIGHTS (12)
static unsigned int pin[NUM_OF_LIGHTS] = {0};
static unsigned int test_brightness[NUM_OF_LIGHTS] = {0,120,125,128,50,60,70,80,90,100,110};
static unsigned int off_array[NUM_OF_LIGHTS] = {0,0,0,0,0,0,0,0,0,0,0,0};

//NOTE: define brightness from 0-128
void setup()
{
  Serial.begin(9600);
  // put your setup code here, to run once:
 // Set up the light bulb control pins
 for (int i = 0 ; i< NUM_OF_LIGHTS; i++)
 {
   pin[i] = i + 22; //assign pins 22 - 33 to control light dimming
 }
 
  acdimmer_init(NUM_OF_LIGHTS,pin);
  acdimmer_bulb_array_set(test_brightness);
  acdimmer_enable();
//  Serial.println("Yo");
}

void loop()
{
  // put your main code here, to run repeatedly:
//
// for (int i = 0 ; i< NUM_OF_LIGHTS; i++)
// {
//    acdimmer_bulb_set(i,100);
//    delay(1000);
//    acdimmer_bulb_array_set(off_array);
// }  
//
// for (int i = 11 ; i>-1 ; i--)
// {
//    acdimmer_bulb_set(i,100);
//    delay(1000);
//    acdimmer_bulb_array_set(off_array);
// }  
}
