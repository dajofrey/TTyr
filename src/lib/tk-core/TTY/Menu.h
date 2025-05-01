#ifndef TTYR_CORE_MENU_H
#define TTYR_CORE_MENU_H

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "View.h"
#include "nh-core/Util/List.h"
#include "nh-encoding/Encodings/UTF32.h"
#include "../Common/Includes.h"

typedef struct tk_core_Menu {
    nh_api_PixelPosition Position;
    int width, height;
} tk_core_Menu;

TTYR_CORE_RESULT tk_core_drawPrompt(
    tk_core_Row *Grid_p
);

TTYR_CORE_RESULT tk_core_drawMicroWindowMenu(
    tk_core_Row *Grid_p
);

#endif 
