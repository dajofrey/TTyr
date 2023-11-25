// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES =========================================================================================

#include "Result.h"

// RESULTS =========================================================================================

const char *TTYR_TTY_RESULTS_PP[] = 
{
    "TTYR_TTY_SUCCESS",                    
    "TTYR_TTY_ERROR_NULL_POINTER",         
    "TTYR_TTY_ERROR_BAD_STATE", 
    "TTYR_TTY_ERROR_MEMORY_ALLOCATION",
    "TTYR_TTY_ERROR_TERMINFO_DATA_CANNOT_BE_FOUND",
    "TTYR_TTY_ERROR_UNKNOWN_TERMINAL_TYPE",
    "TTYR_TTY_ERROR_TERMINAL_IS_HARDCOPY",
    "TTYR_TTY_ERROR_UNKNOWN_COMMAND",
    "TTYR_TTY_ERROR_INVALID_ARGUMENT",
};

unsigned int TTYR_TTY_RESULTS_PP_COUNT = sizeof(TTYR_TTY_RESULTS_PP) / sizeof(TTYR_TTY_RESULTS_PP[0]);

