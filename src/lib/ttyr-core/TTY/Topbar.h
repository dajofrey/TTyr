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

typedef struct ttyr_core_MicroWindow ttyr_core_MicroWindow;

#endif

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct ttyr_core_TopbarSuggestions {
        nh_core_SystemTime Time;
	ttyr_core_Program *ProgramSuggestion_p;
	char *CommandSuggestion_p;
	double interval;
	bool update;
	int commandIndex;
	int programIndex;
    } ttyr_core_TopbarSuggestions;

    typedef struct ttyr_core_Topbar {
	ttyr_core_TopbarSuggestions Suggestions;
        ttyr_core_TopbarMessage Message;
        int state;
        int cursorX;
        int quitPosition;
        bool hasFocus;
	bool refresh;
        bool quitHover;
        nh_encoding_UTF32String Command;
        nh_core_Array History;
    } ttyr_core_Topbar;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    ttyr_core_Topbar ttyr_core_initTopbar(
    );

    TTYR_CORE_RESULT ttyr_core_freeTopbar(
        ttyr_core_Topbar *Topbar_p
    );

    void ttyr_core_toggleTopbar(
        ttyr_core_Topbar *Topbar_p
    );

    TTYR_CORE_RESULT ttyr_core_getTopbarCursor(
        ttyr_core_Topbar *Topbar_p, int *x_p, int *y_p, bool topTile
    );

    TTYR_CORE_RESULT ttyr_core_handleTopbarInput(
        ttyr_core_Tile *Tile_p, nh_api_WSIEvent Event 
    );

    TTYR_CORE_RESULT ttyr_core_drawTopbarRow(
        ttyr_core_Tile *Tile_p, ttyr_core_Glyph *Glyphs_p, int cols, int row, bool standardIO
    );

    TTYR_CORE_RESULT ttyr_core_updateTopbar(
        ttyr_core_Topbar *Topbar_p
    );

/** @} */

#endif 
