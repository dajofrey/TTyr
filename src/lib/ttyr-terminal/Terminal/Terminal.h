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
#include "../../ttyr-core/TTY/View.h"

#include "nh-core/Util/Time.h"

#endif

/** @addtogroup lib_nhterminal_structs
 *  @{
 */

    typedef struct ttyr_terminal_Terminal {
        ttyr_core_TTY *TTY_p;
        char namespace_p[255];
        ttyr_terminal_Config Config;
        ttyr_terminal_Grid Grid;
        ttyr_terminal_Grid Grid2;
        ttyr_terminal_Graphics Graphics;
        ttyr_terminal_Graphics Graphics2;
        ttyr_core_View *View_p;
        bool ctrl;
        bool leftMouse;
        nh_core_SystemTime LastClick;
    } ttyr_terminal_Terminal;

/** @} */

#endif 
