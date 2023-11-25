#ifndef TTYR_TTY_LOG_H
#define TTYR_TTY_LOG_H

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

//    TTYR_TTY_RESULT ttyr_tty_logTerminalMode(
//        ttyr_tty_Shell *Shell_p, int mode
//    );
//
//    TTYR_TTY_RESULT ttyr_tty_logCSI(
//        ttyr_tty_Shell *Shell_p, NH_BYTE type
//    );

    TTYR_TTY_RESULT _ttyr_tty_logBegin(
        const NH_BYTE *file_p, const NH_BYTE *function_p
    );

    TTYR_TTY_RESULT _ttyr_tty_logEnd(
        const NH_BYTE *file_p, const NH_BYTE *function_p
    );

    TTYR_TTY_RESULT _ttyr_tty_logDiagnosticEnd(
        const NH_BYTE *file_p, const NH_BYTE *function_p, TTYR_TTY_RESULT result, int line
    );

/** @} */

#endif 
