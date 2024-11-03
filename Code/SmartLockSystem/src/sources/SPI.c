#include <avr/io.h>
#include "headers/SPI.h"
#include "headers/pinConfig.h"

// SPI functions
void spi_init(void) {
    DDRB |= (1<<PB3) | (1<<PB5) | (1<<SS_PIN);
    DDRB &= ~(1<<PB4);
    SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR1) | (1<<SPR0);
    SPSR &= ~(1<<SPI2X);
}

uint8_t spi_transfer(uint8_t data) {
    SPDR = data;
    while(!(SPSR & (1<<SPIF)));
    return SPDR;
}