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

    TTYR_CORE_RESULT ttyr_core_getStandardOutputWindowSize(
        int *cols_p, int *rows_p
    );
    
    TTYR_CORE_RESULT ttyr_core_readStandardInput(
        ttyr_core_TTY *TTY_p
    );

    TTYR_CORE_RESULT ttyr_core_writeCursorToStandardOutput(
        int x, int y
    );

    TTYR_CORE_RESULT ttyr_core_writeToStandardOutput(
        ttyr_core_Row *Rows_p, int cols, int rows
    );

    TTYR_CORE_RESULT ttyr_core_claimStandardIO(
        ttyr_core_TTY *TTY_p
    );
    
    TTYR_CORE_RESULT ttyr_core_unclaimStandardIO(
        ttyr_core_TTY *TTY_p
    );

    bool ttyr_core_claimsStandardIO(
        ttyr_core_TTY *TTY_p
    );

/** @} */

#endif 
