//
//  PicoCalc Terminal
//  Copyright Blair leduc. See LICENSE for details.
//

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/stdio/driver.h"
#include "hardware/structs/uart.h"
#include "hardware/uart.h"
#include "version.h"
#include <drivers/picocalc.h>
#include <drivers/audio.h>
#include <drivers/display.h>
#include <drivers/font.h>
#include <drivers/keyboard.h>
#include <drivers/lcd.h>
#include <drivers/southbridge.h>

bool user_interrupt = false;
bool power_off_requested = false;

int uart_port = 0;
int baudrate = 115200;
int databits = 8;
int stopbits = 1;
uart_parity_t parity = UART_PARITY_NONE;

#define UART_BUFFER_SIZE 4096

static volatile uint8_t rx_buffer[UART_BUFFER_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;

typedef struct
{
    const char *name;
    int value;
} menu_value_t;

typedef struct
{
    const char *name;
    int *value;
    const menu_value_t *values;
    size_t num_values;
    int selected;
} menu_item_t;

static const menu_value_t baudrates[] = {
    {"1200", 1200},
    {"2400", 2400},
    {"4800", 4800},
    {"9600", 9600},
    {"19200", 19200},
    {"38400", 38400},
    {"57600", 57600},
    {"115200", 115200},
};

static const menu_value_t uart_ports[] = {
    {"USB-C", 0},
    {"GPIO", 1},
};

static menu_item_t menu_items[] = {
    {"Port", &uart_port, uart_ports, sizeof(uart_ports) / sizeof(uart_ports[0]), 0},
    {"Baud Rate", &baudrate, baudrates, sizeof(baudrates) / sizeof(baudrates[0]), 7},
};

void process_one_char(void)
{
    if (rx_head != rx_tail)
    {
        char ch = rx_buffer[rx_tail];
        rx_tail = (rx_tail + 1) & (UART_BUFFER_SIZE - 1);

        // Process the character
        display_emit(ch);
    }
}

void serial_putc(char ch)
{
    uart_putc_raw(uart_port ? uart1 : uart0, ch);
}

void serial_puts(const char *str)
{
    while (*str)
    {
        uart_putc_raw(uart_port ? uart1 : uart0, *str++);
    }
}

void menu(void)
{
    int row = 5;
    for (size_t i = 0; i < sizeof(menu_items) / sizeof(menu_items[0]); i++)
    {
        printf("\033[%d;0H%s: \033[%d;20H\033[%dm%6s\033[0m\n", row++, menu_items[i].name, row, i > 0 ? 0 : 7, menu_items[i].values[menu_items[i].selected].name);
    }

    row += 2;
    printf("\033[%d;0HPress up, down to select, left, right to change,\nEnter to connect\n", row++);
    printf("GP4 - TX, GP5 - RX\n", row++);

    char ch = 0;
    int item = 0;
    while (ch != '\n' && ch != '\r')
    {
        ch = getchar();
        if (ch == KEY_LEFT)
        {
            // Decrease value
            if (menu_items[item].selected > 0)
            {
                *menu_items[item].value = menu_items[item].values[--menu_items[item].selected].value;
                printf("\033[%d;20H\033[7m%6s\033[0m", 6 + item, menu_items[item].values[menu_items[item].selected].name);
            }
        }
        else if (ch == KEY_RIGHT)
        {
            // Increase value
            if (menu_items[item].selected < menu_items[item].num_values - 1)
            {
                *menu_items[item].value = menu_items[item].values[++menu_items[item].selected].value;
                printf("\033[%d;20H\033[7m%6s\033[0m", 6 + item, menu_items[item].values[menu_items[item].selected].name);
            }
        }
        else if (ch == KEY_UP)
        {
            // Previous item
            if (item > 0)
            {
                printf("\033[%d;20H%6s", 6 + item, menu_items[item].values[menu_items[item].selected].name);
                item--;
                printf("\033[%d;20H\033[7m%6s\033[0m", 6 + item, menu_items[item].values[menu_items[item].selected].name);
            }
        }
        else if (ch == KEY_DOWN)
        {
            // Next item
            if (item < (int)(sizeof(menu_items) / sizeof(menu_items[0])) - 1)
            {
                printf("\033[%d;20H%6s", 6 + item, menu_items[item].values[menu_items[item].selected].name);
                item++;
                printf("\033[%d;20H\033[7m%6s\033[0m", 6 + item, menu_items[item].values[menu_items[item].selected].name);
            }
        }
    }
}

void report(const char *message)
{
    while (*message)
    {
        serial_putc(*message++);
    }
}

void bell(void)
{
    audio_play_sound_blocking(PITCH_A4, PITCH_A4, NOTE_SIXTEENTH);
}

void init(void)
{
    sb_init();
    audio_init();

    display_init(NULL, NULL);
    lcd_set_font(&font_5x10); // 64 columns
    display_set_bell_callback(bell);
    display_set_report_callback(report);

    keyboard_init(picocalc_chars_available_notify);

    stdio_set_driver_enabled(&picocalc_stdio_driver, true);
    stdio_set_translate_crlf(&picocalc_stdio_driver, true);

    // UART0
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);
    gpio_set_function(4, GPIO_FUNC_UART);
    gpio_set_function(5, GPIO_FUNC_UART);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(uart0, false, false);
    uart_set_hw_flow(uart1, false, false);

    // Turn on FIFO's
    uart_set_fifo_enabled(uart0, true);
    uart_set_fifo_enabled(uart1, true);

    // Disable interrupts
    uart_set_irqs_enabled(uart0, false, false);
    uart_set_irqs_enabled(uart1, false, false);
}

void main_core1()
{
    char ch;

    while (1)
    {
        // Main loop for core 1

        while (!(uart_get_hw(uart_port ? uart1 : uart0)->fr & UART_UARTFR_RXFE_BITS))
        {
            ch = uart_get_hw(uart_port ? uart1 : uart0)->dr & 0x7F;

            uint16_t next_head = (rx_head + 1) & (UART_BUFFER_SIZE - 1);
            rx_buffer[rx_head] = ch;
            rx_head = next_head;
        }

        tight_loop_contents();
    }
}

int main()
{
    stdio_init_all();
    init();
    multicore_launch_core1(main_core1);

    while (1)
    {
        user_interrupt = false;

        printf("\033[2J\033[H\033[mTerminal %s\n", PICOCALC_TERMINAL_VERSION);
        printf("Copyright Blair Leduc. See LICENSE for details.\n");
        printf("Connect with USB-C port or GPIO pins, 3.3V only!\n");
        printf("\n");

        menu();

        uart_init(uart_port ? uart1 : uart0, baudrate);
        uart_set_format(uart_port ? uart1 : uart0, databits, stopbits, parity);

        printf("\033[2J\033[H\033[mConnected at %s %d %d%s%d\n",
               uart_port ? "GPIO" : "USB-C",
               baudrate,
               databits,
               (parity == UART_PARITY_NONE)
                   ? "N"
               : (parity == UART_PARITY_EVEN)
                   ? "E"
                   : "O",
               stopbits);
        printf("Press [Brk] to change.\n\n");

        while (!user_interrupt)
        {
            process_one_char();

            if (keyboard_key_available())
            {
                char ch = keyboard_get_key();
                switch (ch)
                {
                case KEY_DEL:
                    serial_putc(0177);
                    break;
                case KEY_ESC:
                    serial_putc(033);
                    break;
                case KEY_HOME:
                    serial_puts("\033[H");
                    break;
                case KEY_END:
                    serial_puts("\033[F");
                    break;
                case KEY_UP:
                    serial_puts("\033[A");
                    break;
                case KEY_DOWN:
                    serial_puts("\033[B");
                    break;
                case KEY_RIGHT:
                    serial_puts("\033[C");
                    break;
                case KEY_LEFT:
                    serial_puts("\033[D");
                    break;
                case KEY_F1:
                    serial_puts("\033OP");
                    break;
                case KEY_F2:
                    serial_puts("\033OQ");
                    break;
                case KEY_F3:
                    serial_puts("\033OR");
                    break;
                case KEY_F4:
                    serial_puts("\033OS");
                    break;
                case KEY_F5:
                    serial_puts("\033[15~");
                    break;
                case KEY_F6:
                    serial_puts("\033[17~");
                    break;
                case KEY_F7:
                    serial_puts("\033[18~");
                    break;
                case KEY_F8:
                    serial_puts("\033[19~");
                    break;
                case KEY_F9:
                    serial_puts("\033[20~");
                    break;
                case KEY_F10:
                    serial_puts("\033[21~");
                    break;
                default:
                    serial_putc(ch);
                }
            }

            tight_loop_contents();
        }

        uart_deinit(uart_port ? uart1 : uart0);
    }
}
