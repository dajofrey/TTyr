// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES =========================================================================================

#include "Result.h"

// RESULTS =========================================================================================

const char *TTYR_TERMINAL_RESULTS_PP[] = 
{
    "TTYR_TERMINAL_SUCCESS",                    
    "TTYR_TERMINAL_ERROR_NULL_POINTER",         
    "TTYR_TERMINAL_ERROR_BAD_STATE", 
    "TTYR_TERMINAL_ERROR_MEMORY_ALLOCATION",
    "TTYR_TERMINAL_ERROR_TERMINFO_DATA_CANNOT_BE_FOUND",
    "TTYR_TERMINAL_ERROR_UNKNOWN_TERMINAL_TYPE",
    "TTYR_TERMINAL_ERROR_TERMINAL_IS_HARDCOPY",
    "TTYR_TERMINAL_ERROR_UNKNOWN_COMMAND",
    "TTYR_TERMINAL_ERROR_INVALID_ARGUMENT",
};

unsigned int TTYR_TERMINAL_RESULTS_PP_COUNT = sizeof(TTYR_TERMINAL_RESULTS_PP) / sizeof(TTYR_TERMINAL_RESULTS_PP[0]);

