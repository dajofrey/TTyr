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
#include "../../ttyr-tty/TTY/View.h"

#include "../../../../external/Netzhaut/src/lib/nhcore/Util/Time.h"

#endif

/** @addtogroup lib_nhterminal_structs
 *  @{
 */

    typedef struct ttyr_terminal_Terminal {
        ttyr_tty_TTY *TTY_p;
        NH_BYTE namespace_p[255];
        ttyr_terminal_Config Config;
        ttyr_terminal_Grid Grid;
        ttyr_terminal_Grid Grid2;
        ttyr_terminal_Graphics Graphics;
        ttyr_terminal_Graphics Graphics2;
        ttyr_tty_View *View_p;
        NH_BOOL ctrl;
        NH_BOOL leftMouse;
        nh_SystemTime LastClick;
    } ttyr_terminal_Terminal;

/** @} */

/** @addtogroup lib_nhterminal_typedefs
 *  @{
 */

    typedef ttyr_terminal_Terminal *(*ttyr_terminal_openTerminal_f)(
        NH_BYTE *config_p, ttyr_tty_TTY *TTY_p
    ); 

    typedef TTYR_TERMINAL_RESULT (*ttyr_terminal_cmd_setViewport_f)(
        ttyr_terminal_Terminal *Terminal_p, nh_gfx_Viewport *Viewport_p
    ); 

/** @} */

#endif 
