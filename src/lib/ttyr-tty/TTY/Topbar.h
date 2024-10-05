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
#include "TopbarMessage.h"

#include "nh-core/Util/Array.h"
#include "nh-core/Util/Time.h"
#include "nh-encoding/Encodings/UTF32.h"

#include "../Common/Includes.h"

typedef struct ttyr_tty_MicroWindow ttyr_tty_MicroWindow;

#endif

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct ttyr_tty_TopbarSuggestions {
        nh_core_SystemTime Time;
	ttyr_tty_Program *ProgramSuggestion_p;
	char *CommandSuggestion_p;
	double interval;
	bool update;
	int commandIndex;
	int programIndex;
    } ttyr_tty_TopbarSuggestions;

    typedef struct ttyr_tty_Topbar {
	ttyr_tty_TopbarSuggestions Suggestions;
        ttyr_tty_TopbarMessage Message;
        int state;
        int cursorX;
        int quitPosition;
        bool hasFocus;
	bool refresh;
        bool quitHover;
        nh_encoding_UTF32String Command;
        nh_core_Array History;
    } ttyr_tty_Topbar;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    ttyr_tty_Topbar ttyr_tty_initTopbar(
    );

    TTYR_TTY_RESULT ttyr_tty_freeTopbar(
        ttyr_tty_Topbar *Topbar_p
    );

    void ttyr_tty_toggleTopbar(
        ttyr_tty_Topbar *Topbar_p
    );

    TTYR_TTY_RESULT ttyr_tty_getTopbarCursor(
        ttyr_tty_Topbar *Topbar_p, int *x_p, int *y_p, bool topTile
    );

    TTYR_TTY_RESULT ttyr_tty_handleTopbarInput(
        ttyr_tty_Tile *Tile_p, nh_api_WSIEvent Event 
    );

    TTYR_TTY_RESULT ttyr_tty_drawTopbarRow(
        ttyr_tty_Tile *Tile_p, ttyr_tty_Glyph *Glyphs_p, int cols, int row, bool standardIO
    );

    TTYR_TTY_RESULT ttyr_tty_updateTopbar(
        ttyr_tty_Topbar *Topbar_p
    );

/** @} */

#endif 
