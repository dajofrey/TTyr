#ifndef TTYR_TTY_TOPBAR_H
#define TTYR_TTY_TOPBAR_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "Tiling.h"
#include "Program.h"
#include "TopBarMessage.h"

#include "../Common/Includes.h"

typedef struct ttyr_tty_MicroWindow ttyr_tty_MicroWindow;

#endif

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct ttyr_tty_TopBarSuggestions {
        nh_SystemTime Time;
	ttyr_tty_Program *ProgramSuggestion_p;
	NH_BYTE *CommandSuggestion_p;
	double interval;
	NH_BOOL update;
	int commandIndex;
	int programIndex;
    } ttyr_tty_TopBarSuggestions;

    typedef struct ttyr_tty_TopBar {
	ttyr_tty_TopBarSuggestions Suggestions;
        ttyr_tty_TopBarMessage Message;
        int state;
        int cursorX;
        int quitPosition;
        NH_BOOL hasFocus;
	NH_BOOL refresh;
        NH_BOOL quitHover;
        nh_encoding_UTF32String Command;
        nh_Array History;
    } ttyr_tty_TopBar;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    ttyr_tty_TopBar ttyr_tty_initTopBar(
    );

    TTYR_TTY_RESULT ttyr_tty_freeTopBar(
        ttyr_tty_TopBar *TopBar_p
    );

    void ttyr_tty_toggleTopBar(
        ttyr_tty_TopBar *TopBar_p
    );

    TTYR_TTY_RESULT ttyr_tty_getTopBarCursor(
        ttyr_tty_TopBar *TopBar_p, int *x_p, int *y_p, NH_BOOL topTile
    );

    TTYR_TTY_RESULT ttyr_tty_handleTopBarInput(
        ttyr_tty_Tile *Tile_p, nh_wsi_Event Event 
    );

    TTYR_TTY_RESULT ttyr_tty_drawTopBarRow(
        ttyr_tty_Tile *Tile_p, ttyr_tty_Glyph *Glyphs_p, int cols, int row, NH_BOOL standardIO
    );

    TTYR_TTY_RESULT ttyr_tty_updateTopBar(
        ttyr_tty_TopBar *TopBar_p
    );

/** @} */

#endif 
