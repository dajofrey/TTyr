#ifndef TTYR_TERMINAL_TERMINAL_H
#define TTYR_TERMINAL_TERMINAL_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "Grid.h"
#include "Graphics.h"

#include "../Common/Config.h"
#include "../Common/Includes.h"
#include "../../tk-core/TTY/View.h"

#include "nh-core/Util/Time.h"

#endif

/** @addtogroup lib_nhterminal_structs
 *  @{
 */

    typedef struct tk_terminal_Terminal {
        nh_gfx_Text Text;
        tk_core_TTY *TTY_p;
        char namespace_p[255];
        tk_terminal_Config Config;
        tk_terminal_Grid Grid;
        tk_terminal_Grid Grid2;
        tk_terminal_Grid BorderGrid;
        tk_terminal_Graphics Graphics;
        tk_terminal_Graphics Graphics2;
        tk_core_View *View_p;
        bool ctrl;
        bool leftMouse;
        nh_core_SystemTime LastClick;
    } tk_terminal_Terminal;

/** @} */

#endif 
