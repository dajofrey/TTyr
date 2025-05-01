#ifndef TTYR_API_H
#define TTYR_API_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "tk-core.h"
#include "tk-terminal.h"

#endif

/** @defgroup api_nhtty nhtty
 *  @brief Environment for running text based programs.
 *
 *  In this API, TTY stands for TeleType and provides an interface for
 *  writing text based programs. Using these programs, it processes input streams from either
 *  standard input or \ref tk_core_sendInput and sends the result to either standard
 *  output or a \ref tk_terminal_Terminal. Text based programs can be either added
 *  by using \ref tk_core_addDefaultProgram or \ref tk_core_addCustomProgram.
 */

/** @addtogroup api_nhtty
 *  @{
 */

    extern char TTYR_API_PATH_P[255];

    void tk_api_initialize(
    );

/** @} */

#endif // TTYR_API_H
