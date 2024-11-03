#include <avr/io.h>
#include <stdlib.h>
#include "headers/UART.h"

#include <avr/io.h>
#include <stdint.h>

void uart_init(uint16_t ubrr) {
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (3 << UCSZ00);
}

void uart_transmit(uint8_t data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void uart_print(const char *s) {
    while (*s) {
        uart_transmit((uint8_t)*s);
        s++;
    }
}