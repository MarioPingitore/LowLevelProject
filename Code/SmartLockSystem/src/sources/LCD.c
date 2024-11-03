#include "headers/LCD.h"
#include "headers/UART.h"
#include "headers/pinConfig.h"

void lcd_pulse_enable(void) {
    PORTD |= (1 << LCD_EN);
    _delay_us(1);
    PORTD &= ~(1 << LCD_EN);
    _delay_us(50);
}

void lcd_send_nibble(uint8_t nibble) {
    PORTD &= ~((1 << LCD_D4) | (1 << LCD_D5) 
    | (1 << LCD_D6) | (1 << LCD_D7));
    if (nibble & 0x01) PORTD |= (1 << LCD_D4);
    if (nibble & 0x02) PORTD |= (1 << LCD_D5);
    if (nibble & 0x04) PORTD |= (1 << LCD_D6);
    if (nibble & 0x08) PORTD |= (1 << LCD_D7);
    lcd_pulse_enable();
}

void lcd_send_byte(uint8_t rs, uint8_t data) {
    if (rs)
        PORTD |= (1 << LCD_RS);
    else
        PORTD &= ~(1 << LCD_RS);

    _delay_us(1);

    lcd_send_nibble(data >> 4);
    lcd_send_nibble(data & 0x0F);

    _delay_us(50);
}

void lcd_command(uint8_t cmd) {
    lcd_send_byte(0, cmd);
    _delay_ms(2);
}

void lcd_data(uint8_t data) {
    lcd_send_byte(1, data);
}

void lcd_string(const char *str) {
    while (*str) {
        lcd_data(*str);
        str++;
        _delay_ms(1);
    }
}

void lcd_init(void) {
    DDRD |= (1 << LCD_RS) | (1 << LCD_EN) | (1 << LCD_D4) |
     (1 << LCD_D5) | (1 << LCD_D6) | (1 << LCD_D7); //set pins as output

    _delay_ms(50); //wait for lcd initialization

    lcd_send_nibble(0x03);
    _delay_ms(5);
    lcd_send_nibble(0x03);
    _delay_ms(1);
    lcd_send_nibble(0x03);
    _delay_ms(1);
    lcd_send_nibble(0x02);
    _delay_ms(1);

    lcd_command(0x28);  // 4-bit mode, 2 lines, 5x8 font
    lcd_command(0x0C);  // Display on, cursor off, blink off
    lcd_command(0x06);  // Increment cursor, no shift
    lcd_command(0x01);  // Clear display
    _delay_ms(2);
}

