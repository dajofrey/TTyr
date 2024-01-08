#ifndef TTYR_TTY_TITLEBAR_H
#define TTYR_TTY_TITLEBAR_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "TTY.h"
#include "../Common/Includes.h"

#endif

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    void ttyr_tty_handleTitlebarHit( 
        nh_wsi_MouseEvent Event, int cCol
    );

    void ttyr_tty_checkTitlebar(
        ttyr_tty_Titlebar *Titlebar_p, NH_BOOL *refresh_p
    );

    TTYR_TTY_RESULT ttyr_tty_drawTitlebar(
        ttyr_tty_TTY *TTY_p
    );

/** @} */

#endif 
