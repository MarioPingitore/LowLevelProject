#ifndef SPI_H_
#define SPI_H_

#include <avr/io.h>
#include "headers/pinConfig.h"

void spi_init(void);
uint8_t spi_transfer(uint8_t data);

#endif //SPI_H_