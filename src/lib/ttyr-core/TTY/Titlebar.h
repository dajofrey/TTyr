#ifndef TTYR_CORE_TITLEBAR_H
#define TTYR_CORE_TITLEBAR_H

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

    void ttyr_core_handleTitlebarHit( 
        nh_api_MouseEvent Event, int cCol
    );

    void ttyr_core_checkTitlebar(
        ttyr_core_Titlebar *Titlebar_p, bool *refresh_p
    );

    TTYR_CORE_RESULT ttyr_core_drawTitlebar(
        ttyr_core_TTY *TTY_p
    );

/** @} */

#endif 
