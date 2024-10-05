#ifndef TTYR_TTY_STANDARD_IO_H
#define TTYR_TTY_STANDARD_IO_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "View.h"

#include "../Common/Includes.h"

#endif

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    TTYR_TTY_RESULT ttyr_tty_getStandardOutputWindowSize(
        int *cols_p, int *rows_p
    );
    
    TTYR_TTY_RESULT ttyr_tty_readStandardInput(
        ttyr_tty_TTY *TTY_p
    );

    TTYR_TTY_RESULT ttyr_tty_writeCursorToStandardOutput(
        int x, int y
    );

    TTYR_TTY_RESULT ttyr_tty_writeToStandardOutput(
        ttyr_tty_Row *Rows_p, int cols, int rows
    );

    TTYR_TTY_RESULT ttyr_tty_claimStandardIO(
        ttyr_tty_TTY *TTY_p
    );
    
    TTYR_TTY_RESULT ttyr_tty_unclaimStandardIO(
        ttyr_tty_TTY *TTY_p
    );

    bool ttyr_tty_claimsStandardIO(
        ttyr_tty_TTY *TTY_p
    );

/** @} */

#endif 
