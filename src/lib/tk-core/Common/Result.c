// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES =========================================================================================

#include "Result.h"

// RESULTS =========================================================================================

const char *TK_CORE_RESULTS_PP[] = 
{
    "TK_CORE_SUCCESS",                    
    "TK_CORE_ERROR_NULL_POINTER",         
    "TK_CORE_ERROR_BAD_STATE", 
    "TK_CORE_ERROR_MEMORY_ALLOCATION",
    "TK_CORE_ERROR_TERMINFO_DATA_CANNOT_BE_FOUND",
    "TK_CORE_ERROR_UNKNOWN_TERMINAL_TYPE",
    "TK_CORE_ERROR_TERMINAL_IS_HARDCOPY",
    "TK_CORE_ERROR_UNKNOWN_COMMAND",
    "TK_CORE_ERROR_INVALID_ARGUMENT",
};

unsigned int TK_CORE_RESULTS_PP_COUNT = sizeof(TK_CORE_RESULTS_PP) / sizeof(TK_CORE_RESULTS_PP[0]);

