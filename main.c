//
//  PicoCalc Terminal
//  Copyright Blair leduc. See LICENSE for details.
//

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include <drivers/picocalc.h>
#include <drivers/audio.h>
#include <drivers/display.h>
#include <drivers/font.h>
#include <drivers/keyboard.h>
#include <drivers/lcd.h>
#include <drivers/southbridge.h>

#include "version.h"
#include "terminal.h"
#include "menu.h"

// Global variables
bool user_interrupt = false;

int uart_port = 1;
int baudrate = 115200;
int databits = 8;
int stopbits = 1;
uart_parity_t parity = UART_PARITY_NONE;

volatile bool connected = false;


//
// Process a single character from the serial input
// to be displayed on the screen
//

void process_one_char(void)
{
    if (serial_char_available())
    {
        char ch = serial_getchar();
        if (ch)
        {
            // Process the character
            display_emit(ch);
        }
    }
}

//
// Callbacks for display
//

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


//
// System Initialization
//  

void init(void)
{
    sb_init();
    audio_init();
    display_init();
    keyboard_init();
    terminal_init();

    display_set_bell_callback(bell);
    display_set_report_callback(report);

    // UART0
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(uart0, false, false);

    // Turn on FIFO's
    uart_set_fifo_enabled(uart0, true);

    // Disable interrupts
    uart_set_irqs_enabled(uart0, false, false);

    // Disable CR/LF translation
    uart_set_translate_crlf(uart0, false);

    // Initialize UART0
    uart_init(uart0, baudrate);

    // UART1
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(4, GPIO_FUNC_UART);
    gpio_set_function(5, GPIO_FUNC_UART);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(uart1, false, false);

    // Turn on FIFO's
    uart_set_fifo_enabled(uart1, true);

    // Disable interrupts
    uart_set_irqs_enabled(uart1, false, false);

    // Disable CR/LF translation
    uart_set_translate_crlf(uart1, false);

    // Initialize UART1
    uart_init(uart1, baudrate);
}


//
// Core 1 entry point: Poll keyboard in background
//

void core1_entry()
{
    while (1)
    {
        if (connected)
        {
            keyboard_poll();
            sleep_ms(100);
        }
    }
}


//
// Main program loop
//
int main()
{
    char buffer[32];

    init();
    multicore_launch_core1(core1_entry);

    while (1)
    {
        user_interrupt = false;

        terminal_printf("\033[?25l\033[?4264l\033[2J\033[m\033[1;20H^");
        terminal_printf("\033[2;18HUSB-C");

        terminal_printf("\033[15;1H< TX");
        terminal_printf("\033[16;1H< RX");
        
        terminal_printf("\033[19;1H< GND");

        snprintf(buffer, sizeof(buffer), "Serial Terminal v%s", PICOCALC_TERMINAL_VERSION);
        terminal_printf("\033[5;%dH\033[1m%s\033[m", (40 - strlen(buffer)) >> 1, buffer);

        terminal_printf("\033[?4264h\033[24;21HUp and down to select");
        terminal_printf("\033[25;14HLeft and right to change the setting");
        terminal_printf("\033[26;24HEnter to connect");

        terminal_printf("\033[29;15HGPIO: GP4=TX, GP5=RX, 3.3V ONLY!");

        terminal_printf("\033[32;8HCopyright Blair Leduc. See LICENSE for details.\033[H\033[?4264l");

        menu();

        uart_set_baudrate(uart_port ? uart1 : uart0, baudrate);
        uart_set_format(uart_port ? uart1 : uart0, databits, stopbits, parity);

        terminal_printf("\033[?4264h\033[2J\033[H\033[mConnected at %s %d %d%s%d",
                 uart_port ? "GPIO" : "USB-C",
                 baudrate,
                 databits,
                 (parity == UART_PARITY_NONE)
                     ? "N"
                 : (parity == UART_PARITY_EVEN)
                     ? "E"
                     : "O",
                 stopbits);
        terminal_printf("\033[2HPress [Brk] to change.\033[4H\033[?25h");

        connected = true;

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
        }

        connected = false;
    }
}
