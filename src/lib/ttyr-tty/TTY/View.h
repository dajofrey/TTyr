#ifndef TTYR_TTY_VIEW_H
#define TTYR_TTY_VIEW_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "../Common/Includes.h"
#include "nh-core/Util/RingBuffer.h"

#ifdef __unix__
    #include <termios.h>
#endif

#endif

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct ttyr_tty_Forward {
        nh_core_RingBuffer Tiles;
        nh_core_RingBuffer Boxes;
        nh_core_RingBuffer Events;
    } ttyr_tty_Forward;

    typedef struct ttyr_tty_View {
        ttyr_tty_Forward Forward;
        bool standardIO;
        int cols, rows, rows2;
        int previousCols, previousRows;
        nh_api_PixelSize Size;
        nh_api_PixelSize TileSize;
        nh_api_PixelSize PreviousSize;
        ttyr_tty_Row Row;
        ttyr_tty_Row *Grid1_p; /**<Grid for program/titlebar drawing.*/
        ttyr_tty_Row *Grid2_p; /**<Grid for context-menu drawing.*/
        void *p;
#ifdef __unix__
        struct termios Termios;
#endif
    } ttyr_tty_View;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    TTYR_TTY_RESULT ttyr_tty_getViewSize(
        ttyr_tty_View *View_p
    ); 

    TTYR_TTY_RESULT ttyr_tty_translateMousePosition(
        ttyr_tty_View *View_p, nh_api_MouseEvent Mouse, int *col_p, int *row_p
    );

    TTYR_TTY_RESULT ttyr_tty_updateView(
        ttyr_tty_View *View_p, bool *updated_p, bool macro
    );

    ttyr_tty_View *ttyr_tty_createView(
        ttyr_tty_TTY *TTY_p, void *p, bool standardIO
    );
    
    TTYR_TTY_RESULT ttyr_tty_destroyView(
        ttyr_tty_TTY *TTY_p, ttyr_tty_View *View_p
    );

// FORWARD FUNCTIONS
// These functions forward specific TTY internal data to the view for rendering etc..

    TTYR_TTY_RESULT ttyr_tty_forwardCursor(
        ttyr_tty_View *View_p, int x, int y
    );

    TTYR_TTY_RESULT ttyr_tty_forwardGrid1(
        ttyr_tty_View *View_p
    );

    TTYR_TTY_RESULT ttyr_tty_forwardGrid2(
        ttyr_tty_View *View_p
    );

    TTYR_TTY_RESULT ttyr_tty_forwardEvent(
        ttyr_tty_View *View_p, nh_api_WSIEvent Event
    );

/** @} */

#endif 
