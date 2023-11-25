// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Initialize.h"
#include "IndexMap.h"
#include "Macros.h"
#include "Data/ttyr-tty.conf.inc"

#include "../../../../external/Netzhaut/src/lib/nhcore/Config/Config.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/System/Logger.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/System/Memory.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// DEBUG ===========================================================================================

TTYR_TTY_RESULT ttyr_tty_initialize()
{
TTYR_TTY_BEGIN()

    nh_core_appendConfig(ttyr_tty_conf_inc, ttyr_tty_conf_inc_len, NH_FALSE);
    ttyr_tty_createIndexMap();

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

