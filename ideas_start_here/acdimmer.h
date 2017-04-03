
#ifndef __ACDIMMER_H_
#define __ACDIMMER_H_

#define OFF_VAL (20)

void acdimmer_init(unsigned int num_of_lights, unsigned int * p_pins);

void acdimmer_bulb_set(unsigned int light_id, unsigned int brightness); //sets the brightness for 1 light

void acdimmer_bulb_array_set(unsigned int * p_brightness); //sets the brightness for an array of lights

void acdimmer_enable(void);

#endif //__ACDIMMER_H_
