#ifndef TTYR_TERMINAL_LOG_H
#define TTYR_TERMINAL_LOG_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "Includes.h"

#endif

/** @addtogroup lib_nhterminal_functions
 *  @{
 */

    TTYR_TERMINAL_RESULT _ttyr_terminal_logBegin(
        const NH_BYTE *file_p, const NH_BYTE *function_p
    );

    TTYR_TERMINAL_RESULT _ttyr_terminal_logEnd(
        const NH_BYTE *file_p, const NH_BYTE *function_p
    );

    TTYR_TERMINAL_RESULT _ttyr_terminal_logDiagnosticEnd(
        const NH_BYTE *file_p, const NH_BYTE *function_p, TTYR_TERMINAL_RESULT result, int line
    );

/** @} */

#endif 
