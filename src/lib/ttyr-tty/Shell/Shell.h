#ifndef TTYR_TTY_SHELL_SHELL_H
#define TTYR_TTY_SHELL_SHELL_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "../Common/Includes.h"

#include <stddef.h>
#include <unistd.h>

#endif

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    ttyr_tty_Interface *ttyr_tty_createShellInterface(
    );
    
/** @} */

#endif 
