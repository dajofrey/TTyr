// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Log.h"
#include "Macros.h"

#include "nh-core/Logger/Logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// LOG =============================================================================================

//TTYR_TTY_RESULT ttyr_tty_logTerminalMode(
//    ttyr_tty_Shell *Shell_p, int mode)
//{
//TTYR_TTY_BEGIN()
//
//    char *description_p = ttyr_tty_getTTYModeDescription(mode);
//
//    char message_p[255];
//    sprintf(message_p, "TERM %d %s", mode, description_p ? description_p : "(no description)");
//
//    char node_p[255];
//    sprintf(node_p, "nhtty:Shell:%p", Shell_p);
//
//    nh_core_sendLogMessage(node_p, NULL, message_p);
//
//TTYR_TTY_END(TTYR_TTY_SUCCESS)
//}
//
//TTYR_TTY_RESULT ttyr_tty_logCSI(
//    ttyr_tty_Shell *Shell_p, char type)
//{
//TTYR_TTY_BEGIN()
//
//    const char *description_p = ttyr_tty_getCSIDescription(type);
//
//    char message_p[255];
//    sprintf(message_p, "CSI \\e[%s %d %s", Shell_p->CSIEscape.buf, type, description_p ? description_p : "no description");
//
//    char node_p[255];
//    sprintf(node_p, "nhtty:Shell:%p", Shell_p);
//
//    nh_core_sendLogMessage(node_p, NULL, message_p);
//
//TTYR_TTY_END(TTYR_TTY_SUCCESS)
//}

// FLOW LOGGING ====================================================================================

TTYR_TTY_RESULT _ttyr_tty_logBegin(
    const char *file_p, const char *function_p)
{
//    if (!NH_CONFIG.Flags.Log.Flow.core) {return NH_SUCCESS;}
//    return _nh_begin(file_p, function_p);
}

TTYR_TTY_RESULT _ttyr_tty_logEnd(
    const char *file_p, const char *function_p)
{
//    if (!NH_CONFIG.Flags.Log.Flow.core) {return NH_SUCCESS;}
//    return _nh_end(file_p, function_p);
}

TTYR_TTY_RESULT _ttyr_tty_logDiagnosticEnd(
    const char *file_p, const char *function_p, TTYR_TTY_RESULT result, int line)
{
//    if (!NH_CONFIG.Flags.Log.Flow.core) {return result;}
//    _nh_diagnosticEnd(file_p, function_p, NH_CORE_RESULTS_PP[result], line, result == NH_SUCCESS);
    return result;
}

