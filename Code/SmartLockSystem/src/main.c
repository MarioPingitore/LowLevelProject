#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include "headers/pinConfig.h"
#include "headers/LCD.h"
#include "headers/UART.h"
#include "headers/SERVO.h."
#include "headers/SPI.h."
#include "headers/ULTRASONIC.h"
#include "headers/RFID.h"

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#ifndef BAUD
#define BAUD 9600
#endif

#ifndef MYUBRR
#define MYUBRR (F_CPU/(16UL * BAUD))-1
#endif

#define MAX_LEN 16
#define MAX_TAGS 100
#define TAG_LENGTH 4

uint8_t master_tag[TAG_LENGTH];
uint8_t authorized_tags[MAX_TAGS][TAG_LENGTH];
uint8_t num_tags = 0;
uint8_t master_mode = 0;
uint8_t master_tag_saved = 0;
volatile uint32_t system_timer = 0;
uint32_t master_mode_start_time = 0;


// Function prototypes
void save_master_tag(uint8_t *tag);
uint8_t is_master_tag(uint8_t *tag);
void add_tag(uint8_t *tag);
void remove_tag(uint8_t *tag);
uint8_t is_authorized(uint8_t *tag);
void print_tag(uint8_t *tag);


void save_master_tag(uint8_t *tag) {
    memcpy(master_tag, tag, TAG_LENGTH);
    master_tag_saved = 1;  // set the flag when the master tag is saved
    uart_print("Master tag saved\r\n");
}

uint8_t is_master_tag(uint8_t *tag) {
    return memcmp(master_tag, tag, TAG_LENGTH) == 0;
}

void add_tag(uint8_t *tag) {
    if (num_tags < MAX_TAGS) {
        memcpy(authorized_tags[num_tags], tag, TAG_LENGTH);
        num_tags++;
        uart_print("Tag added\r\n");
        lcd_command(0x01);
        lcd_string("Tag Added");
        _delay_ms(2000);  // Show message for 2 seconds
        master_mode = 0;  // Exit from master mode
        uart_print("Exited master mode\r\n");
        lcd_command(0x01);
        lcd_string("Normal Mode");
    } else {
        uart_print("Tag memory full\r\n");
        lcd_command(0x01);
        lcd_string("Memory Full");
        _delay_ms(2000);
    }
}

void remove_tag(uint8_t *tag) {
    for (uint8_t i = 0; i < num_tags; i++) {
        if (memcmp(authorized_tags[i], tag, TAG_LENGTH) == 0) {
            memcpy(authorized_tags[i], authorized_tags[num_tags - 1],
             TAG_LENGTH);
            num_tags--;
            uart_print("Tag removed\r\n");
            lcd_command(0x01);
            lcd_string("Tag Removed");
            _delay_ms(2000);  
            master_mode = 0;  
            uart_print("Exited master mode\r\n");
            lcd_command(0x01);
            lcd_string("Normal Mode");
            return;
        }
    }
    uart_print("Tag not found\r\n");
    lcd_command(0x01);
    lcd_string("Tag Not Found");
    _delay_ms(2000);
}

uint8_t is_authorized(uint8_t *tag) {
    for (uint8_t i = 0; i < num_tags; i++) {
        if (memcmp(authorized_tags[i], tag, TAG_LENGTH) == 0) {
            return 1;
        }
    }
    return 0;
}



void print_tag(uint8_t *tag) {
    // Buffer to memorize the hexadecimal rappresentation of a byte
    char buffer[3];  
    for (int i = 0; i < TAG_LENGTH; i++) {
        byte_to_hex(tag[i], buffer);
        uart_print(buffer);
        uart_print(" ");
    }
    uart_print("\r\n");
}


//timer
ISR(TIMER0_COMPA_vect) {
    system_timer++;
}

void timer_init() {
    TCCR0A = (1 << WGM01);  // CTC mode
    TCCR0B = (1 << CS01) | (1 << CS00);  // Prescaler 64
    OCR0A = 249;  // Per 1ms con clock a 16MHz
    TIMSK0 = (1 << OCIE0A);  // Enable interrupt pn compare match
}

uint32_t millis() {
    uint32_t m;
    cli();  // Disable  interrupts
    m = system_timer;
    sei();  // Re-enable  interrupts
    return m;
}


int main(void) {
    uart_init(MYUBRR);
    spi_init();
    mfrc522_init();
    lcd_init();
    servo_init();
    ultrasonic_init();
    timer_init();
    sei();  // Enable global interrupts

    uart_print("RFID Access Control System Initialized\r\n");
    uart_print("Waiting for master tag...\r\n");

    uint8_t status;
    uint8_t tag_type[MAX_LEN];
    uint8_t serial_number[5];
    uint32_t last_action_time = 0;
    uint8_t last_tag[5] = {0};
    uint8_t door_state = 0;  // 0: close, 1: opening, 2: open, 3: closing
    uint32_t door_action_time = 0;
    uint32_t display_toggle_time = 0;
    uint8_t display_state = 0;  // 0: "Access Control", 1: "Waiting for master"

    while (1) {
        if (!master_tag_saved) {
            // Toggle display every 3 seconds until master tag is registered
            if (millis() - display_toggle_time > 3000) {
                lcd_command(0x01);  // Clear display
                if (display_state == 0) {
                    lcd_string("Access Control");
                    lcd_command(0xC0);  // Go to second line
                    lcd_string("System Ready");
                    display_state = 1;
                } else {
                    lcd_string("Waiting for");
                    lcd_command(0xC0);  // Go to second line
                    lcd_string("Master Tag");
                    display_state = 0;
                }
                display_toggle_time = millis();
            }
        }

        // Check for master mode - timeout
        if (master_mode && (millis() - master_mode_start_time > 5000)) {
            master_mode = 0;
            lcd_command(0x01);  // Clear display
            lcd_string("Master Mode");
            lcd_command(0xC0);  // Go to second line
            lcd_string("Timeout");
            uart_print("Master mode timeout\r\n");
            _delay_ms(2000);  // Show timeout message for 2 seconds
            lcd_command(0x01);
            lcd_string("Access Control");
            lcd_command(0xC0);
            lcd_string("Scan Your Tag");
        }

        uint16_t distance = get_distance();        
        if (distance < 60) {  // If an object (person) is in a 6cm proximity
            status = mfrc522_request(PICC_REQIDL, tag_type);
            if (status == 0) {
                status = mfrc522_anticoll(serial_number);
                if (status == 0) {
                    uart_print("Tag detected: ");
                    print_tag(serial_number);
                    // Check if is a new tag
                    if (memcmp(serial_number, last_tag, 5) != 0) {
                        memcpy(last_tag, serial_number, 5);
                        if (!master_tag_saved) {    // If is a new card, is saved as Master tag
                            save_master_tag(serial_number);
                            lcd_command(0x01);  // Clear display
                            lcd_string("Master Tag");
                            lcd_command(0xC0);  // Go to second line
                            lcd_string("Registered");
                            uart_print("Master tag registered\r\n");
                            last_action_time = millis();
                        } else if (is_master_tag(serial_number)) {
                            master_mode = !master_mode;
                            lcd_command(0x01);  // Clear display
                            lcd_string(master_mode ? "Master Mode" : "Normal Mode");
                            uart_print(master_mode ? "Entered master mode\r\n" : "Exited master mode\r\n");
                            if (master_mode) {
                                master_mode_start_time = millis();
                            }
                            last_action_time = millis();
                        } else if (master_mode) {
                            if (is_authorized(serial_number)) {
                                remove_tag(serial_number);
                            } else {
                                add_tag(serial_number);
                            }
                            master_mode_start_time = millis();  // Reset master mode timer
                            last_action_time = millis();
                        } else {
                            if (is_authorized(serial_number)) {
                                lcd_command(0x01);
                                lcd_string("Access Granted");
                                lcd_command(0xC0);
                                lcd_string("Opening Door");
                                servo_set_position(4799);  // Open door
                                door_state = 1;
                                door_action_time = millis();
                                last_action_time = millis();
                            } else {
                                lcd_command(0x01);
                                lcd_string("Access Denied");
                                last_action_time = millis();
                            }
                        }
                    }
                }
            }
        } else {
            // If there is no tag, rest last_tag
            memset(last_tag, 0, 5);
        }

        // Handling of the door status
        switch (door_state) {
            case 1:  // Opening
                if (millis() - door_action_time > 3000) {
                    lcd_command(0x01);
                    lcd_string("Opening door");
                    door_state = 2;
                    door_action_time = millis();
                }
                break;
            case 2:  // Open
                if (millis() - door_action_time > 5000) {
                    lcd_command(0x01);
                    lcd_string("Closing door");
                    servo_set_position(1499);  // Closing the door
                    door_state = 3;
                    door_action_time = millis();
                }
                break;
            case 3:  // Closing
                if (millis() - door_action_time > 3000) {
                    lcd_command(0x01);
                    lcd_string("Door closed");
                    door_state = 0;
                    last_action_time = millis();
                }
                break;
        }

        // If more than 7 second are elapsed from the last action performed, return to the default state
        if (master_tag_saved && millis() - last_action_time > 7000 && door_state == 0 && !master_mode) {
        lcd_command(0x01);
        lcd_string("Access Control");
        lcd_command(0xC0);
        lcd_string("Scan Your Tag");
        last_action_time = millis();
        }

        _delay_ms(100);  // LLittle delay before the next iteration
    }

    return 0;
}