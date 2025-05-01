#ifndef TTYR_CORE_STANDARD_IO_H
#define TTYR_CORE_STANDARD_IO_H

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

    TTYR_CORE_RESULT tk_core_getStandardOutputWindowSize(
        int *cols_p, int *rows_p
    );
    
    TTYR_CORE_RESULT tk_core_readStandardInput(
        tk_core_TTY *TTY_p
    );

    TTYR_CORE_RESULT tk_core_writeCursorToStandardOutput(
        int x, int y
    );

    TTYR_CORE_RESULT tk_core_writeToStandardOutput(
        tk_core_Row *Rows_p, int cols, int rows
    );

    TTYR_CORE_RESULT tk_core_claimStandardIO(
        tk_core_TTY *TTY_p
    );
    
    TTYR_CORE_RESULT tk_core_unclaimStandardIO(
        tk_core_TTY *TTY_p
    );

    bool tk_core_claimsStandardIO(
        tk_core_TTY *TTY_p
    );

/** @} */

#endif 
