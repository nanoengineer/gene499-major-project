#include "Average.h"


#define SAMPLE_FILTER_COUNT (50)
int sensorPin = A0; // select the input pin for the potentiometer
int ledPin = 13; // select the pin for the LED
int sensorValue = 0; // variable to store the value coming from the sensor

int microphone_samples[SAMPLE_FILTER_COUNT] = {0};
bool init_sampling = true;

Average<int> stats(SAMPLE_FILTER_COUNT);

void setup ()
{
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  //Serial.println("Hi");

  for(int i = 0; i < SAMPLE_FILTER_COUNT; i++)
  {
      stats.push(analogRead(sensorPin));
      delay(100);
  }
}

void loop ()
{
    // Serial.print("Mode:");
    // Serial.println(stats.mode());



  while(true)
  {
    for(int i = 0; i < SAMPLE_FILTER_COUNT; i++)
    {
        stats.push(analogRead(sensorPin));

        // Serial.print("Mode:");
        // Serial.println(stats.mode());

        //Serial.print("Delta:");
        int data = analogRead(sensorPin) - stats.mode();


        int scaled_data = abs(data)*abs(data);
        Serial.println(scaled_data);




        delay(50);
    }
  }



}
