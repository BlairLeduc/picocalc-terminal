//
//  PicoCalc Terminal
//  Copyright Blair leduc. See LICENSE for details.
//

#include <stdio.h>

#include <drivers/keyboard.h>

#include "terminal.h"
#include "menu.h"

static menu_item_t menu_items[] = {
    {"Port", &uart_port, uart_ports, sizeof(uart_ports) / sizeof(uart_ports[0]), 1},
    {"Baud Rate", &baudrate, baudrates, sizeof(baudrates) / sizeof(baudrates[0]), 7},
    {"Data Bits", &databits, databits_options, sizeof(databits_options) / sizeof(databits_options[0]), 1},
    {"Parity", (int *)&parity, parity_options, sizeof(parity_options) / sizeof(parity_options[0]), 0},
    {"Stop Bits", &stopbits, stopbits_options, sizeof(stopbits_options) / sizeof(stopbits_options[0]), 0},
};

void menu(void)
{
    int row = 8;
    for (size_t i = 0; i < sizeof(menu_items) / sizeof(menu_items[0]); i++)
    {
        terminal_printf("\033[%d;10H%s: \033[%d;24H\033[%dm%6s\033[m",
               row, menu_items[i].name,
               row, i > 0 ? 0 : 4, menu_items[i].values[menu_items[i].selected].name);
        row += 2;
    }

    char ch = 0;
    int item = 0;
    while (ch != '\n' && ch != '\r')
    {
        ch = terminal_getchar();
        if (ch == KEY_LEFT)
        {
            // Decrease value
            if (menu_items[item].selected > 0)
            {
                *menu_items[item].value = menu_items[item].values[--menu_items[item].selected].value;
                terminal_printf("\033[%d;24H\033[4m%6s\033[m", 8 + item * 2, menu_items[item].values[menu_items[item].selected].name);
            }
        }
        else if (ch == KEY_RIGHT)
        {
            // Increase value
            if (menu_items[item].selected < menu_items[item].num_values - 1)
            {
                *menu_items[item].value = menu_items[item].values[++menu_items[item].selected].value;
                terminal_printf("\033[%d;24H\033[4m%6s\033[m", 8 + item * 2, menu_items[item].values[menu_items[item].selected].name);
            }
        }
        else if (ch == KEY_UP)
        {
            // Previous item
            if (item > 0)
            {
                terminal_printf("\033[%d;24H%6s", 8 + item * 2, menu_items[item].values[menu_items[item].selected].name);
                item--;
                terminal_printf("\033[%d;24H\033[4m%6s\033[m", 8 + item * 2, menu_items[item].values[menu_items[item].selected].name);
            }
        }
        else if (ch == KEY_DOWN)
        {
            // Next item
            if (item < (int)(sizeof(menu_items) / sizeof(menu_items[0])) - 1)
            {
                terminal_printf("\033[%d;24H%6s", 8 + item * 2, menu_items[item].values[menu_items[item].selected].name);
                item++;
                terminal_printf("\033[%d;24H\033[4m%6s\033[m", 8 + item * 2, menu_items[item].values[menu_items[item].selected].name);
            }
        }
    }
}