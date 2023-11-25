#ifndef TTYR_TTY_PROGRAM_H
#define TTYR_TTY_PROGRAM_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "../Common/Includes.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/Util/List.h"

typedef struct ttyr_tty_TTY ttyr_tty_TTY;

#endif

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    TTYR_TTY_RESULT ttyr_tty_addProgram(
        ttyr_tty_TTY *TTY_p, ttyr_tty_ProgramPrototype *Prototype_p, bool once
    );

    ttyr_tty_Program *ttyr_tty_createProgramInstance(
        ttyr_tty_ProgramPrototype *Prototype_p, bool once
    );

    TTYR_TTY_RESULT ttyr_tty_destroyProgramInstance(
        ttyr_tty_Program *Program_p
    );

/** @} */

#endif 

