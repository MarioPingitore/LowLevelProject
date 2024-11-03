#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "headers/pinConfig.h"
#include "headers/RFID.h"
#include "headers/SPI.h"
#include "headers/UART.h"

#define MAX_LEN 16

void mfrc522_write(uint8_t reg, uint8_t data) {
    PORTB &= ~(1<<SS_PIN);
    spi_transfer((reg << 1) & 0x7E);
    spi_transfer(data);
    PORTB |= (1<<SS_PIN);
}

uint8_t mfrc522_read(uint8_t reg) {
    uint8_t value;
    PORTB &= ~(1<<SS_PIN);
    spi_transfer(((reg << 1) & 0x7E) | 0x80);
    value = spi_transfer(0x00);
    PORTB |= (1<<SS_PIN);
    return value;
}

void mfrc522_reset(void) {
    mfrc522_write(CommandReg, PCD_RESETPHASE);
}

void mfrc522_antenna_on(void) {
    uint8_t temp = mfrc522_read(TxControlReg);
    if (!(temp & 0x03)) {
        mfrc522_write(TxControlReg, temp | 0x03);
    }
}

void mfrc522_init(void) {
    DDRB |= (1<<RST_PIN);
    PORTB &= ~(1<<RST_PIN);
    _delay_ms(50);
    PORTB |= (1<<RST_PIN);
    _delay_ms(50);
    
    mfrc522_reset();
    
    mfrc522_write(TModeReg, 0x8D);
    mfrc522_write(TPrescalerReg, 0x3E);
    mfrc522_write(TReloadRegL, 0x30);
    mfrc522_write(TReloadRegH, 0);
    mfrc522_write(TxASKReg, 0x40);
    mfrc522_write(ModeReg, 0x3D);
    
    mfrc522_antenna_on();
}

uint8_t mfrc522_request(uint8_t req_mode, uint8_t *tag_type) {
    uint8_t status;
    uint32_t backBits;
    mfrc522_write(BitFramingReg, 0x07);
    tag_type[0] = req_mode;
    status = mfrc522_to_card(PCD_TRANSCEIVE, tag_type, 1, 
    tag_type, &backBits);
    if ((status != 0) || (backBits != 0x10)) {
        status = 2;
    }
    return status;
}

uint8_t mfrc522_to_card(uint8_t command, uint8_t *send_data, 
uint8_t send_len, uint8_t *back_data, uint32_t *back_len) {
    uint8_t status = 2;
    uint8_t irqEn = 0x00;
    uint8_t waitIRq = 0x00;
    uint8_t lastBits;
    uint8_t n;
    uint32_t i;

    switch (command) {
        case PCD_AUTHENT:
            irqEn = 0x12;
            waitIRq = 0x10;
            break;
        case PCD_TRANSCEIVE:
            irqEn = 0x77;
            waitIRq = 0x30;
            break;
        default:
            break;
    }

    n=mfrc522_read(ComIrqReg);
    mfrc522_write(ComIrqReg,n&(~0x80));//clear all interrupt bits
    n=mfrc522_read(FIFOLevelReg);
    mfrc522_write(FIFOLevelReg,n|0x80);//flush FIFO data

    for (i = 0; i < send_len; i++) {
        mfrc522_write(FIFODataReg, send_data[i]);
    }

    mfrc522_write(CommandReg, command);

    if (command == PCD_TRANSCEIVE) {
      n=mfrc522_read(BitFramingReg);
      mfrc522_write(BitFramingReg,n|0x80);    
    }

    i = 2000;
    do {
        n = mfrc522_read(ComIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitIRq));

    mfrc522_write(BitFramingReg, mfrc522_read(BitFramingReg) & (~0x80));

    if (i != 0) {
        if (!(mfrc522_read(ErrorReg) & 0x1B)) {
            status = 0;
            if (n & irqEn & 0x01) {
                status = 1;
            }

            if (command == PCD_TRANSCEIVE) {
                n = mfrc522_read(FIFOLevelReg);
                lastBits = mfrc522_read(ControlReg) & 0x07;
                if (lastBits) {
                    *back_len = (uint32_t)(n-1)*8 + (uint32_t)lastBits;
                } else {
                    *back_len = (uint32_t)n*8;
                }

                if (n == 0) {
                    n = 1;
                }
                if (n > MAX_LEN) {
                    n = MAX_LEN;
                }

                for (i = 0; i < n; i++) {
                    back_data[i] = mfrc522_read(FIFODataReg);
                }
            }
        } else {
            status = 2;
        }
    }

    return status;
}

// Function to convert a byte in an 2-char hexadecimal string
void byte_to_hex(uint8_t byte, char* buffer) {
    static const char hex_digits[] = "0123456789ABCDEF";
    buffer[0] = hex_digits[byte >> 4];
    buffer[1] = hex_digits[byte & 0x0F];
    buffer[2] = '\0';
}

uint8_t mfrc522_anticoll(uint8_t *serial_number) {
    uint8_t status;
    uint8_t i;
    uint8_t serNumCheck = 0;
    uint32_t len;

    uart_print("Anticoll: Starting\r\n");

    mfrc522_write(BitFramingReg, 0x00);
    serial_number[0] = PICC_ANTICOLL;
    serial_number[1] = 0x20;

    uart_print("Anticoll: Before to_card\r\n");
    status = mfrc522_to_card(PCD_TRANSCEIVE, serial_number, 2, serial_number, &len);
    
    char buffer[3];
    uart_print("Anticoll: After to_card, status: ");
    byte_to_hex(status, buffer);
    uart_print(buffer);
    uart_print(", len: ");
    // Per unLen, possiamo usare una semplice conversione in decimale
    char len_buffer[6];  // Sufficiente per un uint16_t
    itoa(len, len_buffer, 10);
    uart_print(len_buffer);
    uart_print("\r\n");

    if (status == 0) {
        uart_print("Anticoll: to_card success\r\n");
        if (len == 40) {
            for (i = 0; i < 4; i++) {
                serNumCheck ^= serial_number[i];
                uart_print("Serial Number[");
                char index_buffer[2];
                itoa(i, index_buffer, 10);
                uart_print(index_buffer);
                uart_print("]: ");
                byte_to_hex(serial_number[i], buffer);
                uart_print(buffer);
                uart_print("\r\n");
            }
            uart_print("Calculated SerNumCheck: ");
            byte_to_hex(serNumCheck, buffer);
            uart_print(buffer);
            uart_print("\r\n");
            uart_print("Received Checksum: ");
            byte_to_hex(serial_number[4], buffer);
            uart_print(buffer);
            uart_print("\r\n");
            
            if (serNumCheck == serial_number[4]) {
                uart_print("Anticoll: Checksum correct\r\n");
                return 0;
            } else {
                uart_print("Anticoll: Checksum failed\r\n");
                return 2;
            }
        } else {
            uart_print("Anticoll: Incorrect data length\r\n");
            return 2;
        }
    } else {
        uart_print("Anticoll: to_card failed\r\n");
    }

    return status;
}
