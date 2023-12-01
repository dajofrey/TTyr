#ifndef TTYR_TTY_DRAW_H
#define TTYR_TTY_DRAW_H

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

    TTYR_TTY_RESULT ttyr_tty_refreshCursor(
        ttyr_tty_TTY *TTY_p
    );

    TTYR_TTY_RESULT ttyr_tty_refreshGrid1(
        ttyr_tty_TTY *TTY_p
    ); 

    TTYR_TTY_RESULT ttyr_tty_refreshGrid2(
        ttyr_tty_TTY *TTY_p
    ); 

    TTYR_TTY_RESULT ttyr_tty_refreshGrid1Row(
        nh_List *Tiles_p, ttyr_tty_View *View_p, int row
    );

    TTYR_TTY_RESULT ttyr_tty_getCursorPosition(
        ttyr_tty_Tile *MacroTile_p, ttyr_tty_Tile *MicroTile_p, NH_BOOL standardIO, int *x_p, int *y_p
    );
    
/** @} */

#endif 
