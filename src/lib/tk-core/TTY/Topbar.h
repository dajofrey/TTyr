#ifndef TTYR_CORE_TOPBAR_H
#define TTYR_CORE_TOPBAR_H

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
#include "../Common/Config.h"

typedef struct tk_core_MicroWindow tk_core_MicroWindow;

#endif

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct tk_core_TopbarSuggestions {
        nh_core_SystemTime Time;
	tk_core_Program *ProgramSuggestion_p;
	char *CommandSuggestion_p;
	double interval;
	bool update;
	int commandIndex;
	int programIndex;
    } tk_core_TopbarSuggestions;

    typedef struct tk_core_Topbar {
	tk_core_TopbarSuggestions Suggestions;
        tk_core_TopbarMessage Message;
        int state;
        int cursorX;
        int quitPosition;
        bool hasFocus;
	bool refresh;
        bool quitHover;
        nh_encoding_UTF32String Command;
        nh_core_Array History;
    } tk_core_Topbar;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    tk_core_Topbar tk_core_initTopbar(
    );

    TTYR_CORE_RESULT tk_core_freeTopbar(
        tk_core_Topbar *Topbar_p
    );

    void tk_core_toggleTopbar(
        tk_core_Topbar *Topbar_p
    );

    TTYR_CORE_RESULT tk_core_getTopbarCursor(
        tk_core_Topbar *Topbar_p, int *x_p, int *y_p, bool topTile
    );

    TTYR_CORE_RESULT tk_core_handleTopbarInput(
        tk_core_Tile *Tile_p, nh_api_WSIEvent Event 
    );

    TTYR_CORE_RESULT tk_core_drawTopbarRow(
        tk_core_Tile *Tile_p, tk_core_Glyph *Glyphs_p, int cols, int row, bool standardIO
    );

    TTYR_CORE_RESULT tk_core_updateTopbar(
        tk_core_Topbar *Topbar_p
    );

/** @} */

#endif 
