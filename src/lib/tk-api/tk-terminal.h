#ifndef TK_TERMINAL_API_H
#define TK_TERMINAL_API_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "tk-core.h"
#include <stdbool.h>

#endif

/** @defgroup api_nhterminal nhterminal
 *  @brief Open terminals for TTY presentation.
 *
 *  This module allows the presentation of TTY logic created by the 
 *  nhtty module without the use of proprietary terminals such as xterm 
 *  or konsole.
 */

/** @addtogroup api_nhterminal
 *  @{
 */

/** @addtogroup api_nhdocs
 *  @{
 */

// ENUMS ===========================================================================================

    /**
     * Return values for functions.
     */
    typedef enum TK_TERMINAL_RESULT
    {
        TK_TERMINAL_SUCCESS, /**<Indicates that something worked as planned.*/
        TK_TERMINAL_ERROR_NULL_POINTER,
        TK_TERMINAL_ERROR_BAD_STATE,
        TK_TERMINAL_ERROR_MEMORY_ALLOCATION,
        TK_TERMINAL_ERROR_TERMINFO_DATA_CANNOT_BE_FOUND,
        TK_TERMINAL_ERROR_UNKNOWN_TERMINAL_TYPE,
        TK_TERMINAL_ERROR_TERMINAL_IS_HARDCOPY,
        TK_TERMINAL_ERROR_UNKNOWN_COMMAND,
        TK_TERMINAL_ERROR_INVALID_ARGUMENT,

    } TK_TERMINAL_RESULT;

// TYPEDEFS ========================================================================================

    typedef struct tk_terminal_Terminal tk_terminal_Terminal;

// FUNCTIONS =======================================================================================

    /**
     * @brief Open a TTY based terminal. 
     *
     * The TTY parameter handles TTY logic while the resulting terminal
     * handles the TTY presentation.
     *
     * @param TTY_p Pointer to TTY. Must not be NULL.
     * @return NULL on failure. A pointer to a newly created terminal on success.
     */
    tk_terminal_Terminal *tk_api_openTerminal(
        char *config_p, tk_core_TTY *TTY_p
    );

    /**
     * @brief Set the viewport of a terminal.
     *
     * Without having a viewport to render into, the terminal won't know where
     * to present the input received by the associated TTY logic.
     *
     * @param Terminal_p Pointer to terminal. Must not be NULL.
     * @param Viewport_p Pointer to viewport. Must not be NULL.
     * @return @ref TK_TERMINAL_SUCCESS on success.
     */
    TK_TERMINAL_RESULT tk_api_setViewport(
        tk_terminal_Terminal *Terminal_p, nh_api_Viewport *Viewport_p
    );

/** @} */

#endif // TK_TERMINAL_API_H
