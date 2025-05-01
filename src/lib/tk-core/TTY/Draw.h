#ifndef TTYR_CORE_DRAW_H
#define TTYR_CORE_DRAW_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "TTY.h"
#include "View.h"

#endif

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    TTYR_CORE_RESULT tk_core_refreshCursor(
        tk_core_TTY *TTY_p
    );

    TTYR_CORE_RESULT tk_core_refreshGrid1(
        tk_core_TTY *TTY_p
    ); 

    TTYR_CORE_RESULT tk_core_refreshGrid2(
        tk_core_TTY *TTY_p
    ); 

    TTYR_CORE_RESULT tk_core_refreshGrid1Row(
        tk_core_Config *Config_p, nh_core_List *Tiles_p, tk_core_View *View_p, int row
    );

    TTYR_CORE_RESULT tk_core_getCursorPosition(
        tk_core_Config *Config_p, tk_core_Tile *MacroTile_p, tk_core_Tile *MicroTile_p, bool standardIO, int *x_p, int *y_p
    );
    
/** @} */

#endif 
