#ifndef UART_H_
#define UART_H_

void uart_init(uint16_t ubrr);
void uart_transmit(uint8_t data);
void uart_print(const char *s);

#endif  //UART