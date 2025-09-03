//
//  PicoCalc Terminal
//  Copyright Blair leduc. See LICENSE for details.
//

#pragma once

#define UART_QUEUE_SIZE 4096

void queues_init(void);

void process_display_queue(void (*action)(char c));
void putc_to_display(char c);
void puts_to_display(const char *str);

void process_serial_queue(void (*action)(char c));
void putc_to_serial(char c);
void puts_to_serial(const char *str);
