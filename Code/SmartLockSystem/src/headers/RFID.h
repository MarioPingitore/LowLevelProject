#ifndef RFID_H_
#define RFID_H_

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "headers/pinConfig.h"

void mfrc522_init(void);
void mfrc522_write(uint8_t reg, uint8_t data);
uint8_t mfrc522_read(uint8_t reg);
void mfrc522_reset(void);
void mfrc522_antenna_on(void);
uint8_t mfrc522_request(uint8_t req_mode, uint8_t *tag_type);
uint8_t mfrc522_to_card(uint8_t command, uint8_t *send_data, uint8_t send_len, uint8_t *back_data, uint32_t *back_len);
void byte_to_hex(uint8_t byte, char* buffer); //auxiliary function
uint8_t mfrc522_anticoll(uint8_t *serial_number);

#endif //RFID_H_