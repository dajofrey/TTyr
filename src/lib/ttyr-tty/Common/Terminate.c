// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Terminate.h"
#include "Macros.h"
#include "IndexMap.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// DEBUG ===========================================================================================

TTYR_TTY_RESULT ttyr_tty_terminate()
{
TTYR_TTY_BEGIN()

    ttyr_tty_freeIndexMap();

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

