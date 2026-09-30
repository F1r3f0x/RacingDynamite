#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include "main.h"

/* Menu Engine Structures */
typedef struct {
    uint32_t unknown[64];
} MenuViewport;

/* Function Prototypes */
void Menu_InitCarViewport(void);

#endif /* MENU_H */
