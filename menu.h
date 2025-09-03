//
//  PicoCalc Terminal
//  Copyright Blair leduc. See LICENSE for details.
//

#pragma once

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


// Function Prototypes
void menu(void);