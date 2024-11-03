#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "headers/pinConfig.h"

void lcd_init(void);
void lcd_command(uint8_t cmd);
void lcd_data(uint8_t data);
void lcd_string(const char *str);

#endif //LCD_H_