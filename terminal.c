//
//  PicoCalc Terminal
//  Copyright Blair leduc. See LICENSE for details.
//

#include <stdio.h>
#include <stdarg.h>

#include "pico/stdio/driver.h"
#include "pico/stdlib.h"
#include "hardware/structs/uart.h"
#include "hardware/timer.h"
#include "hardware/uart.h"

#include "drivers/display.h"
#include "drivers/keyboard.h"

#include "terminal.h"

static struct repeating_timer timer;

//
// Monitor the UART for incoming data
//

bool repeating_timer_callback(struct repeating_timer *t)
{
    char ch;

    if (connected)
    {
        while (!(uart_get_hw(SELECTED_UART)->fr & UART_UARTFR_RXFE_BITS))
        {
            ch = uart_get_hw(SELECTED_UART)->dr & 0xFF;
            if (ch)
            {
                serial_addchar(ch);
            }
        }
    }

    return true;
}

//
//  Serial Input Buffer
//

static volatile uint8_t serial_buffer[UART_BUFFER_SIZE];
static volatile uint32_t serial_head = 0;
static volatile uint32_t serial_tail = 0;

void serial_addchar(char ch)
{
    uint32_t next_head = (serial_head + 1) & (UART_BUFFER_SIZE - 1);
    if (next_head != serial_tail) // only add if buffer is not full
    {
        serial_buffer[serial_head] = ch;
        serial_head = next_head;
    }
}

char serial_getchar(void)
{
    if (serial_head == serial_tail) // buffer is empty
    {
        return 0;
    }

    char ch = serial_buffer[serial_tail];
    serial_tail = (serial_tail + 1) & (UART_BUFFER_SIZE - 1);
    return ch;
}

bool serial_char_available(void)
{
    return serial_head != serial_tail;
}



//
//  Terminal Output
// 

void terminal_printf(const char *fmt, ...) {
    char buf[TERMINAL_PRINTF_BUFFER_SIZE];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (len < 0) return; // Formatting error

    for (int i = 0; i < len && i < sizeof(buf); ++i) {
        display_emit(buf[i]);
    }
}

char terminal_getchar() {
    while (!keyboard_key_available()) {
        keyboard_poll();
        sleep_ms(100);
    }
    return keyboard_get_key();
}


//
// Serial Output
//

void serial_putc(char ch)
{
    uart_putc_raw(SELECTED_UART, ch);
}

void serial_puts(const char *str)
{
    while (*str)
    {
        uart_putc_raw(SELECTED_UART, *str++);
    }
}

//
// Terminal Initialization
//

void terminal_init(void)
{
    // Start a repeating timer to poll the UART for incoming data
    add_repeating_timer_us(-UART_POLL_INTERVAL_US, repeating_timer_callback, NULL, &timer);
}