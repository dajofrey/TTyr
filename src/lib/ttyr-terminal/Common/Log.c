// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Log.h"
#include "Macros.h"

#include "../../../../external/Netzhaut/src/lib/nhcore/System/Logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// FLOW LOGGING ====================================================================================

TTYR_TERMINAL_RESULT _ttyr_terminal_logBegin(
    const NH_BYTE *file_p, const NH_BYTE *function_p)
{
//    if (!NH_CONFIG.Flags.Log.Flow.core) {return NH_SUCCESS;}
//    return _nh_begin(file_p, function_p);
}

TTYR_TERMINAL_RESULT _ttyr_terminal_logEnd(
    const NH_BYTE *file_p, const NH_BYTE *function_p)
{
//    if (!NH_CONFIG.Flags.Log.Flow.core) {return NH_SUCCESS;}
//    return _nh_end(file_p, function_p);
}

TTYR_TERMINAL_RESULT _ttyr_terminal_logDiagnosticEnd(
    const NH_BYTE *file_p, const NH_BYTE *function_p, TTYR_TERMINAL_RESULT result, int line)
{
//    if (!NH_CONFIG.Flags.Log.Flow.core) {return result;}
//    _nh_diagnosticEnd(file_p, function_p, NH_CORE_RESULTS_PP[result], line, result == NH_SUCCESS);
    return result;
}

