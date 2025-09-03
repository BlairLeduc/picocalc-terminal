//
//  PicoCalc Terminal
//  Copyright Blair leduc. See LICENSE for details.
//

#include "io.h"
#include "queues.h"

#include <pico/stdio/driver.h>

#include <drivers/keyboard.h>
#include <drivers/southbridge.h>

static bool key_control = false; // control key state
static bool key_shift = false;   // shift key state
static bool key_alt = false;     // alt key state

bool io_getchar(char *ch)
{
    uint16_t key = 0;
    uint8_t key_state = 0;
    uint8_t key_code = 0;

    key = sb_read_keyboard();
    key_state = (key >> 8) & 0xFF;
    key_code = key & 0xFF;

    if (key_state != 0)
    {
        if (key_state == KEY_STATE_PRESSED)
        {
            if (key_code == KEY_MOD_CTRL)
            {
                key_control = true;
            }
            else if (key_code == KEY_MOD_SHL || key_code == KEY_MOD_SHR)
            {
                key_shift = true;
            }
            else if (key_code == KEY_MOD_ALT)
            {
                key_alt = true;
            }
            // else if (key_code == KEY_BREAK)
            // {
            //     user_interrupt = true; // set user interrupt flag
            // }
            // else if (key_code == KEY_POWER)
            // {
            //     power_off_requested = true; // set power off requested flag
            // }
            else if (key_code == KEY_CAPS_LOCK)
            {
                // do nothing, processed in the south bridge
            }
            else
            {
                // If a key is released, we return the key code
                // This allows us to handle the key release in the main loop
                uint8_t key = key_code;
                if (key >= 'a' && key <= 'z') // Ctrl and Shift handling
                {
                    if (key_control)
                    {
                        key &= 0x1F; // convert to control character
                    }
                    if (key_shift)
                    {
                        key &= ~0x20;
                    }
                }
                else if (key == KEY_ENTER) // enter key is returned as LF
                {
                    key = KEY_RETURN; // convert LF to CR
                }

                *ch = key;
                return true;
            }
        }

        if (key_state == KEY_STATE_RELEASED)
        {
            if (key_code == KEY_MOD_CTRL)
            {
                key_control = false;
            }
            else if (key_code == KEY_MOD_SHL || key_code == KEY_MOD_SHR)
            {
                key_shift = false;
            }
        }
    }

    return false;
}

static void io_out_chars(const char *buf, int length)
{
    for (int i = 0; i < length; ++i)
    {
        putc_to_display(buf[i]);
    }
}

static int io_in_chars(char *buf, int length)
{
    char ch;
    int n = 0;

    while (n < length)
    {
        if (!io_getchar(&ch))
        {
            return n;
        }
        buf[n++] = ch;
    }
    return n;
}

stdio_driver_t io_stdio_driver = {0};

void io_init(void)
{
    io_stdio_driver.out_chars = io_out_chars;
    io_stdio_driver.in_chars = io_in_chars;

    stdio_set_driver_enabled(&io_stdio_driver, true);
    stdio_set_translate_crlf(&io_stdio_driver, true);
}