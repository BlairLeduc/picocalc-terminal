//
//  PicoCalc Terminal
//  Copyright Blair leduc. See LICENSE for details.
//

#include <pico/util/queue.h>

#include "queues.h"
#include <drivers/display.h>

static queue_t uart_queue;
static queue_t display_queue;

void queues_init(void)
{
    queue_init(&uart_queue, sizeof(char), UART_QUEUE_SIZE);
    queue_init(&display_queue, sizeof(uint32_t), UART_QUEUE_SIZE);
}

void process_display_queue(void (*action)(char c))
{
    uint32_t item;

    queue_remove_blocking(&display_queue, &item);
    action(item & 0xFF);
}

void putc_to_display(char c)
{
    // uint32_t item = c;
    // queue_add_blocking(&display_queue, &item);
    display_emit(c);
}

void puts_to_display(const char *str)
{
    while (*str)
    {
        putc_to_display(*str++);
    }
}

void process_serial_queue(void (*action)(char c))
{
    uint32_t item;

    if (queue_try_remove(&uart_queue, &item))
    {
        action(item & 0xFF);
    }
}

void putc_to_serial(char c)
{
    queue_add_blocking(&uart_queue, &c);
}

void puts_to_serial(const char *str)
{
    while (*str)
    {
        putc_to_serial(*str++);
    }
}

