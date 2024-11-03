#ifndef         PIN_ADDCONFIG_H_
#define         PIN_CONFIG_H_

#include <avr/io.h>

// LCD pins
#define LCD_RS PD6
#define LCD_EN PD7
#define LCD_D4 PD2
#define LCD_D5 PD3
#define LCD_D6 PD4
#define LCD_D7 PD5

// SERVO pin
#define SERVO_PIN PB1

// Ultrasonic sensor pins
#define TRIGPIN PC1  // A1
#define ECHOPIN PC2  // A2

// MFRC522 SPI pins
#define RST_PIN     PB2
#define SS_PIN      PB0

// MFRC522 registers
#define CommandReg      0x01
#define ComIEnReg       0x02
#define DivIEnReg       0x03
#define ComIrqReg       0x04
#define DivIrqReg       0x05
#define ErrorReg        0x06
#define Status1Reg      0x07
#define Status2Reg      0x08
#define FIFODataReg     0x09
#define FIFOLevelReg    0x0A
#define WaterLevelReg   0x0B
#define ControlReg      0x0C
#define BitFramingReg   0x0D
#define CollReg         0x0E
#define ModeReg         0x11
#define TxControlReg    0x14
#define TxASKReg        0x15
#define TPrescalerReg   0x2B
#define TReloadRegL     0x2C
#define TReloadRegH     0x2D
#define TModeReg        0x2A
#define VersionReg      0x37

// MFRC522 commands
#define PCD_IDLE        0x00
#define PCD_AUTHENT     0x0E
#define PCD_RECEIVE     0x08
#define PCD_TRANSMIT    0x04
#define PCD_TRANSCEIVE  0x0C
#define PCD_RESETPHASE  0x0F
#define PCD_CALCCRC     0x03

// Mifare_One card command word
#define PICC_REQIDL     0x26
#define PICC_REQALL     0x52
#define PICC_ANTICOLL   0x93
#define PICC_SElECTTAG  0x93
#define PICC_AUTHENT1A  0x60
#define PICC_AUTHENT1B  0x61
#define PICC_READ       0x30
#define PICC_WRITE      0xA0
#define PICC_DECREMENT  0xC0
#define PICC_INCREMENT  0xC1
#define PICC_RESTORE    0xC2
#define PICC_TRANSFER   0xB0
#define PICC_HALT       0x50

#endif		//PIN_CONFIG_H_