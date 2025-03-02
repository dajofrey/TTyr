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

    TTYR_CORE_RESULT ttyr_core_refreshCursor(
        ttyr_core_TTY *TTY_p
    );

    TTYR_CORE_RESULT ttyr_core_refreshGrid1(
        ttyr_core_TTY *TTY_p
    ); 

    TTYR_CORE_RESULT ttyr_core_refreshGrid2(
        ttyr_core_TTY *TTY_p
    ); 

    TTYR_CORE_RESULT ttyr_core_refreshGrid1Row(
        nh_core_List *Tiles_p, ttyr_core_View *View_p, int row
    );

    TTYR_CORE_RESULT ttyr_core_getCursorPosition(
        ttyr_core_Tile *MacroTile_p, ttyr_core_Tile *MicroTile_p, bool standardIO, int *x_p, int *y_p
    );
    
/** @} */

#endif 
