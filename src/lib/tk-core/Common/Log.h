#ifndef TK_CORE_LOG_H
#define TK_CORE_LOG_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "Result.h"

#include "../Shell/Shell.h"

#endif

/** @addtogroup lib_nhtty_functions
 *  @{
 */

//    TK_CORE_RESULT tk_core_logTerminalMode(
//        tk_core_Shell *Shell_p, int mode
//    );
//
//    TK_CORE_RESULT tk_core_logCSI(
//        tk_core_Shell *Shell_p, char type
//    );

    TK_CORE_RESULT _tk_core_logBegin(
        const char *file_p, const char *function_p
    );

    TK_CORE_RESULT _tk_core_logEnd(
        const char *file_p, const char *function_p
    );

    TK_CORE_RESULT _tk_core_logDiagnosticEnd(
        const char *file_p, const char *function_p, TK_CORE_RESULT result, int line
    );

/** @} */

#endif 
