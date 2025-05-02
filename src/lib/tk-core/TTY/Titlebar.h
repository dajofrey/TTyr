#ifndef TK_CORE_TITLEBAR_H
#define TK_CORE_TITLEBAR_H

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

    void tk_core_handleTitlebarHit( 
        nh_api_MouseEvent Event, int cCol
    );

    void tk_core_checkTitlebar(
        tk_core_Config *Config_p, tk_core_Titlebar *Titlebar_p, bool *refresh_p
    );

    TK_CORE_RESULT tk_core_drawTitlebar(
        tk_core_TTY *TTY_p
    );

/** @} */

#endif 
