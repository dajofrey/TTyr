#ifndef TTYR_TTY_MENU_H
#define TTYR_TTY_MENU_H

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "View.h"
#include "nh-core/Util/List.h"
#include "nh-encoding/Encodings/UTF32.h"
#include "../Common/Includes.h"

typedef struct ttyr_tty_Menu {
    nh_api_PixelPosition Position;
    int width, height;
} ttyr_tty_Menu;

TTYR_TTY_RESULT ttyr_tty_drawWindowSwitchMenu(
    ttyr_tty_Row *Grid_p
);

TTYR_TTY_RESULT ttyr_tty_drawTabSwitchMenu(
    ttyr_tty_Row *Grid_p
);

#endif 
