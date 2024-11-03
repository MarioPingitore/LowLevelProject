#ifndef SERVO_H_
#define SERVO_H_

#include <avr/io.h>
#include "headers/pinConfig.h"

void servo_init(void);
void servo_set_position(uint16_t position);

#endif //SERVO_H_
