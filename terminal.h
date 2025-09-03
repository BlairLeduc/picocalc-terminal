//
//  PicoCalc Terminal
//  Copyright Blair leduc. See LICENSE for details.
//

#pragma once

#include <hardware/uart.h>

#define UART_QUEUE_SIZE 4096

extern int uart_port;
extern int baudrate;
extern int databits;
extern int stopbits;
extern uart_parity_t parity;

