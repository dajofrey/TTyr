#ifndef TTYR_TERMINAL_CONFIG_H
#define TTYR_TERMINAL_CONFIG_H

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "Includes.h"
#include <stddef.h>

typedef struct ttyr_terminal_Config {
    int fontSize;
    float blinkFrequency;
    ttyr_core_Color Foreground;
    ttyr_core_Color Backgrounds_p[8];
    ttyr_core_Color Accents_p[8];
    int accents;
    int backgrounds;
    int style;
} ttyr_terminal_Config;

ttyr_terminal_Config ttyr_terminal_updateConfig(
    void *Terminal_p
);

ttyr_terminal_Config ttyr_terminal_getConfig(
    void *Terminal_p
);

#endif
