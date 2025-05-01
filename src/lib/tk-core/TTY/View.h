#ifndef TTYR_CORE_VIEW_H
#define TTYR_CORE_VIEW_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "../Common/Includes.h"
#include "../Common/Config.h"

#include "nh-core/Util/RingBuffer.h"

#ifdef __unix__
    #include <termios.h>
#endif

#endif

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct tk_core_Forward {
        nh_core_RingBuffer Tiles;
        nh_core_RingBuffer Boxes;
        nh_core_RingBuffer Events;
    } tk_core_Forward;

    typedef struct tk_core_View {
        tk_core_Forward Forward;
        bool standardIO;
        int cols, rows, rows2;
        int previousCols, previousRows;
        nh_api_PixelSize Size;
        nh_api_PixelSize TileSize;
        nh_api_PixelSize PreviousSize;
        tk_core_Row Row;
        tk_core_Row *Grid1_p; /**<Grid for program/titlebar drawing.*/
        tk_core_Row *Grid2_p; /**<Grid for context-menu drawing.*/
        void *p;
#ifdef __unix__
        struct termios Termios;
#endif
    } tk_core_View;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    TTYR_CORE_RESULT tk_core_getViewSize(
        tk_core_View *View_p
    ); 

    TTYR_CORE_RESULT tk_core_translateMousePosition(
        tk_core_View *View_p, nh_api_MouseEvent Mouse, int *col_p, int *row_p
    );

    TTYR_CORE_RESULT tk_core_updateView(
        tk_core_Config *Config_p, tk_core_View *View_p, bool *updated_p, bool macro
    );

    tk_core_View *tk_core_createView(
        tk_core_TTY *TTY_p, void *p, bool standardIO
    );
    
    TTYR_CORE_RESULT tk_core_destroyView(
        tk_core_TTY *TTY_p, tk_core_View *View_p
    );

// FORWARD FUNCTIONS
// These functions forward specific TTY internal data to the view for rendering etc..

    TTYR_CORE_RESULT tk_core_forwardCursor(
        tk_core_Config *Config_p, tk_core_View *View_p, int x, int y
    );

    TTYR_CORE_RESULT tk_core_forwardGrid1(
        tk_core_Config *Config_p, tk_core_View *View_p
    );

    TTYR_CORE_RESULT tk_core_forwardGrid2(
        tk_core_View *View_p
    );

    TTYR_CORE_RESULT tk_core_forwardEvent(
        tk_core_View *View_p, nh_api_WSIEvent Event
    );

/** @} */

#endif 
