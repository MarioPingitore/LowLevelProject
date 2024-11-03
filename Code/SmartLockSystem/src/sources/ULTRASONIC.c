#include "headers/ULTRASONIC.h"
#include "headers/pinConfig.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile uint16_t pulse_duration = 0; //memorize echo pulse duration
volatile uint8_t new_data_available = 0; //flag to check if there are new data

ISR(PCINT1_vect) {
    static uint16_t rising_edge, falling_edge;
    if (PINC & (1 << ECHOPIN)) {
        rising_edge = TCNT1;
    } else {
        falling_edge = TCNT1;
        pulse_duration = falling_edge - rising_edge;
        new_data_available = 1;
    }
}

void ultrasonic_init(void) {
    DDRC |= (1 << TRIGPIN); //set TRIGPIN as output
    DDRC &= ~(1 << ECHOPIN); //set ECHOPIN as input
    PORTC &= ~(1 << TRIGPIN); //Ensure TRIGPIN starts LOW

    PCICR |= (1 << PCIE1);
    PCMSK1 |= (1 << PCINT9);
    
    sei();
}

void trigger_measurement(void) {
    PORTC |= (1 << TRIGPIN);
    _delay_us(10);
    PORTC &= ~(1 << TRIGPIN);
}

uint16_t get_distance(void) {
    trigger_measurement();
    _delay_ms(60); //wait for measurement to complete
    if (new_data_available) {
        new_data_available = 0;
        return pulse_duration / 58;  // Convert to mm
    }
    return 0;  // No echo received
}

