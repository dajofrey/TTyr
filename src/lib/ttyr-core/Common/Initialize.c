// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Initialize.h"
#include "Macros.h"
#include "Data/default.conf.inc"

#include "nh-core/Config/Config.h"
#include "nh-core/Logger/Logger.h"
#include "nh-core/System/Memory.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// DEBUG ===========================================================================================

TTYR_CORE_RESULT ttyr_core_initialize()
{
    nh_core_appendConfig(ttyr_default_conf_inc, ttyr_default_conf_inc_len, false);
    return TTYR_CORE_SUCCESS;
}
