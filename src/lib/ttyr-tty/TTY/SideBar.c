// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "SideBar.h"
#include "Program.h"
#include "TTY.h"

#include "../Common/Macros.h"

#include "../../../../external/Netzhaut/src/lib/nhcore/System/Memory.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/System/Thread.h"

#include "../../../../external/Netzhaut/src/lib/nhencoding/Encodings/UTF32.h"
#include "../../../../external/Netzhaut/src/lib/nhencoding/Encodings/UTF8.h"

#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

// INPUT ===========================================================================================

TTYR_TTY_RESULT ttyr_tty_handleSideBarHit(
    NH_WSI_MOUSE_E mouse, int row)
{
TTYR_TTY_BEGIN()

    ttyr_tty_TTY *TTY_p = nh_core_getWorkloadArg();

    switch (mouse) {
        case NH_WSI_MOUSE_LEFT :
            ttyr_tty_insertAndFocusWindow(TTY_p, row);
            break;
        case NH_WSI_MOUSE_RIGHT :
            TTYR_TTY_MACRO_TILE(TTY_p->Window_p->Tile_p)->current = row;
            break;
    }
    
TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

// DRAW ============================================================================================

TTYR_TTY_RESULT ttyr_tty_drawSideBarRow(
    ttyr_tty_Glyph *Glyphs_p, int row, int rows)
{
TTYR_TTY_BEGIN()

    ttyr_tty_TTY *TTY_p = nh_core_getWorkloadArg();
    ttyr_tty_Config Config = ttyr_tty_getConfig();

    if (row < Config.windows) {
        Glyphs_p[0].codepoint = ' ';
        Glyphs_p[0].mark = TTYR_TTY_MARK_ACCENT;
        Glyphs_p[0].Attributes.reverse = nh_core_getListIndex(&TTY_p->Windows, TTY_p->Window_p) == row;
    } else {
        Glyphs_p[0].codepoint = ' ';
        Glyphs_p[0].mark = TTYR_TTY_MARK_ACCENT;
        Glyphs_p[0].Attributes.reverse = NH_TRUE;
    }

    Glyphs_p[1].mark = TTYR_TTY_MARK_ACCENT | TTYR_TTY_MARK_LINE_VERTICAL;
    Glyphs_p[1].Attributes.reverse = NH_FALSE;

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

