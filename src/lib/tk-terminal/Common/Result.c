// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES =========================================================================================

#include "Result.h"

// RESULTS =========================================================================================

const char *TK_TERMINAL_RESULTS_PP[] = 
{
    "TK_TERMINAL_SUCCESS",                    
    "TK_TERMINAL_ERROR_NULL_POINTER",         
    "TK_TERMINAL_ERROR_BAD_STATE", 
    "TK_TERMINAL_ERROR_MEMORY_ALLOCATION",
    "TK_TERMINAL_ERROR_TERMINFO_DATA_CANNOT_BE_FOUND",
    "TK_TERMINAL_ERROR_UNKNOWN_TERMINAL_TYPE",
    "TK_TERMINAL_ERROR_TERMINAL_IS_HARDCOPY",
    "TK_TERMINAL_ERROR_UNKNOWN_COMMAND",
    "TK_TERMINAL_ERROR_INVALID_ARGUMENT",
};

unsigned int TK_TERMINAL_RESULTS_PP_COUNT = sizeof(TK_TERMINAL_RESULTS_PP) / sizeof(TK_TERMINAL_RESULTS_PP[0]);

