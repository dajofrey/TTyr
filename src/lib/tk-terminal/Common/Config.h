#ifndef TK_TERMINAL_CONFIG_H
#define TK_TERMINAL_CONFIG_H

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "Includes.h"
#include <stddef.h>

typedef struct tk_terminal_Config {
    int fontSize;
    float blinkFrequency;
    float animationFreq;
    tk_core_Color Foreground;
    tk_core_Color Backgrounds_p[8];
    tk_core_Color Accents_p[8];
    int accents;
    int backgrounds;
    int style;
} tk_terminal_Config;

tk_terminal_Config tk_terminal_updateConfig(
    void *Terminal_p
);

tk_terminal_Config tk_terminal_getConfig(
    void *Terminal_p
);

#endif
