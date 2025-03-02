// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES =========================================================================================

#include "Result.h"

// RESULTS =========================================================================================

const char *TTYR_CORE_RESULTS_PP[] = 
{
    "TTYR_CORE_SUCCESS",                    
    "TTYR_CORE_ERROR_NULL_POINTER",         
    "TTYR_CORE_ERROR_BAD_STATE", 
    "TTYR_CORE_ERROR_MEMORY_ALLOCATION",
    "TTYR_CORE_ERROR_TERMINFO_DATA_CANNOT_BE_FOUND",
    "TTYR_CORE_ERROR_UNKNOWN_TERMINAL_TYPE",
    "TTYR_CORE_ERROR_TERMINAL_IS_HARDCOPY",
    "TTYR_CORE_ERROR_UNKNOWN_COMMAND",
    "TTYR_CORE_ERROR_INVALID_ARGUMENT",
};

unsigned int TTYR_CORE_RESULTS_PP_COUNT = sizeof(TTYR_CORE_RESULTS_PP) / sizeof(TTYR_CORE_RESULTS_PP[0]);

