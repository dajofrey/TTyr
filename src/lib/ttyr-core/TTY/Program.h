#ifndef TTYR_CORE_PROGRAM_H
#define TTYR_CORE_PROGRAM_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "../Common/Includes.h"
#include "nh-core/Util/List.h"

typedef struct ttyr_core_TTY ttyr_core_TTY;

#endif

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    TTYR_CORE_RESULT ttyr_core_addProgram(
        ttyr_core_TTY *TTY_p, ttyr_core_Interface *Prototype_p, bool once
    );

    ttyr_core_Program *ttyr_core_createProgramInstance(
        ttyr_core_Interface *Prototype_p, bool once
    );

    TTYR_CORE_RESULT ttyr_core_destroyProgramInstance(
        ttyr_core_Program *Program_p
    );

/** @} */

#endif 
