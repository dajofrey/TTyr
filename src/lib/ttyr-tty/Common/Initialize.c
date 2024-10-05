// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Initialize.h"
#include "Macros.h"
#include "Data/ttyr-tty.conf.inc"

#include "nh-core/Config/Config.h"
#include "nh-core/System/Logger.h"
#include "nh-core/System/Memory.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// DEBUG ===========================================================================================

TTYR_TTY_RESULT ttyr_tty_initialize()
{
    nh_core_appendConfig(ttyr_tty_conf_inc, ttyr_tty_conf_inc_len, false);
    return TTYR_TTY_SUCCESS;
}

