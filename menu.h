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

// Function Prototypes
void menu(void);