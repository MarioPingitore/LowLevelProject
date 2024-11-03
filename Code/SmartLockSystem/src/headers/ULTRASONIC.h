#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

#include <avr/io.h>
#include "headers/pinConfig.h"

void ultrasonic_init(void);
void trigger_measurement(void);
uint16_t get_distance(void);

#endif //ULTRASONIC_H_
