#ifndef TTYR_CORE_VIEW_H
#define TTYR_CORE_VIEW_H

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

    typedef struct ttyr_core_Forward {
        nh_core_RingBuffer Tiles;
        nh_core_RingBuffer Boxes;
        nh_core_RingBuffer Events;
    } ttyr_core_Forward;

    typedef struct ttyr_core_View {
        ttyr_core_Forward Forward;
        bool standardIO;
        int cols, rows, rows2;
        int previousCols, previousRows;
        nh_api_PixelSize Size;
        nh_api_PixelSize TileSize;
        nh_api_PixelSize PreviousSize;
        ttyr_core_Row Row;
        ttyr_core_Row *Grid1_p; /**<Grid for program/titlebar drawing.*/
        ttyr_core_Row *Grid2_p; /**<Grid for context-menu drawing.*/
        void *p;
#ifdef __unix__
        struct termios Termios;
#endif
    } ttyr_core_View;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    TTYR_CORE_RESULT ttyr_core_getViewSize(
        ttyr_core_View *View_p
    ); 

    TTYR_CORE_RESULT ttyr_core_translateMousePosition(
        ttyr_core_View *View_p, nh_api_MouseEvent Mouse, int *col_p, int *row_p
    );

    TTYR_CORE_RESULT ttyr_core_updateView(
        ttyr_core_View *View_p, bool *updated_p, bool macro
    );

    ttyr_core_View *ttyr_core_createView(
        ttyr_core_TTY *TTY_p, void *p, bool standardIO
    );
    
    TTYR_CORE_RESULT ttyr_core_destroyView(
        ttyr_core_TTY *TTY_p, ttyr_core_View *View_p
    );

// FORWARD FUNCTIONS
// These functions forward specific TTY internal data to the view for rendering etc..

    TTYR_CORE_RESULT ttyr_core_forwardCursor(
        ttyr_core_View *View_p, int x, int y
    );

    TTYR_CORE_RESULT ttyr_core_forwardGrid1(
        ttyr_core_View *View_p
    );

    TTYR_CORE_RESULT ttyr_core_forwardGrid2(
        ttyr_core_View *View_p
    );

    TTYR_CORE_RESULT ttyr_core_forwardEvent(
        ttyr_core_View *View_p, nh_api_WSIEvent Event
    );

/** @} */

#endif 
