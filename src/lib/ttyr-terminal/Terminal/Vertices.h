#ifndef TTYR_TERMINAL_TERMINAL_VERTICES_H
#define TTYR_TERMINAL_TERMINAL_VERTICES_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "Grid.h"
#include "Graphics.h"

#endif

/** @addtogroup lib_nhterminal_functions
 *  @{
 */

    TTYR_TERMINAL_RESULT ttyr_terminal_getBackgroundVertices(
        ttyr_terminal_GraphicsState *State_p, ttyr_terminal_Grid *Grid_p, ttyr_tty_Glyph *Glyph_p, int col, 
        int row, float vertices_p[12]
    );

    TTYR_TERMINAL_RESULT ttyr_terminal_getBoxVertices(
        ttyr_terminal_GraphicsState *State_p, ttyr_terminal_Grid *Grid_p, ttyr_terminal_Box *Box_p, bool inner
    );

    TTYR_TERMINAL_RESULT ttyr_terminal_getOutlineVertices(
        ttyr_terminal_GraphicsState *State_p, ttyr_terminal_Grid *Grid_p, ttyr_terminal_Box *Box_p, bool inner
    );

    TTYR_TERMINAL_RESULT ttyr_terminal_getForegroundVertices(
        ttyr_terminal_GraphicsState *State_p, ttyr_terminal_Grid *Grid_p, ttyr_tty_Glyph *Glyph_p, int col, 
        int row, float *vertices_p
    );

/** @} */

#endif // TTYR_TERMINAL_TERMINAL_VERTICES_H 
