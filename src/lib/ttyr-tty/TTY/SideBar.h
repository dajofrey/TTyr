#ifndef TTYR_TTY_SIDEBAR_H
#define TTYR_TTY_SIDEBAR_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "../Common/Includes.h"

#endif

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    TTYR_TTY_RESULT ttyr_tty_handleSideBarHit(
        NH_WSI_MOUSE_E mouse, int row
    );

    TTYR_TTY_RESULT ttyr_tty_drawSideBarRow(
        ttyr_tty_Glyph *Glyphs_p, int row, int rows
    );

/** @} */

#endif 
