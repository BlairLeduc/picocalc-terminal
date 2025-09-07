//
//  PicoCalc Terminal
//  Copyright Blair leduc. See LICENSE for details.
//

#pragma once

#include <hardware/uart.h>

// Configuration
#define UART_BUFFER_SIZE 65536   // Size of the UART queue (64K)
#define UART_POLL_INTERVAL_US 50 // Polling interval in microseconds (good for 230400 baud and below)
#define TERMINAL_PRINTF_BUFFER_SIZE 256 // Size of the buffer for terminal_printf

// UART Configuration
extern int uart_port;
extern int baudrate;
extern int databits;
extern int stopbits;
extern uart_parity_t parity;

// Connection state
extern volatile bool connected;

#define SELECTED_UART (uart_port ? uart1 : uart0)

// Function prototypes
void terminal_init(void);

bool serial_char_available(void);
char serial_getchar(void);
void serial_addchar(char ch);
void serial_putc(char ch);
void serial_puts(const char *s);

void terminal_printf(const char *fmt, ...);
char terminal_getchar(void);
