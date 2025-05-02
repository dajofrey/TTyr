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
#include "nh-core/System/Memory.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// FUNCTIONS =======================================================================================

TK_CORE_RESULT tk_core_initialize()
{
    nh_core_appendConfig(termoskanne_default_conf_inc, termoskanne_default_conf_inc_len, true);
    return TK_CORE_SUCCESS;
}
