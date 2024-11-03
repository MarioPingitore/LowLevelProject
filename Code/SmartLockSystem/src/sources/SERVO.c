#include "headers/SERVO.h"
#include "headers/pinConfig.h"
#include <avr/io.h>
#include <util/delay.h>

// Servo functions
void servo_init(void) {
    DDRB |= 1 << SERVO_PIN;  // Set pin 9 as output
    TCCR1A |= (1 << WGM11) | (1 << COM1A1);
    TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11);
    ICR1 = 39999;  // 20ms period (50Hz)
}

void servo_set_position(uint16_t position) {
    OCR1A = position;
}