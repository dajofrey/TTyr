#ifndef TK_CORE_PROGRAM_H
#define TK_CORE_PROGRAM_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "../Common/Includes.h"
#include "nh-core/Util/List.h"

typedef struct tk_core_TTY tk_core_TTY;

#endif

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    TK_CORE_RESULT tk_core_addProgram(
        tk_core_TTY *TTY_p, tk_core_Interface *Prototype_p, bool once
    );

    tk_core_Program *tk_core_createProgramInstance(
        tk_core_Interface *Prototype_p, bool once
    );

    TK_CORE_RESULT tk_core_destroyProgramInstance(
        tk_core_Program *Program_p
    );

/** @} */

#endif 
