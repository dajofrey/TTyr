#ifndef TK_CORE_SHELL_SOCKET_H
#define TK_CORE_SHELL_SOCKET_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "../Common/Includes.h"

#include <stddef.h>
#include <unistd.h>
#include <sys/un.h>

#endif

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct tk_core_ShellSocket {
        struct sockaddr_un address;
        int fd;
        uint16_t port;
    } tk_core_ShellSocket;

/** @} */

/** @addtogroup lib_nhtty_typedefs
 *  @{
 */

    typedef TK_CORE_RESULT (*tk_core_sendCommandToShell_f)(
        int pid, TK_CORE_PROGRAM_E type 
    );

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    TK_CORE_RESULT tk_core_createShellSocket(
        tk_core_ShellSocket *Socket_p, int pid
    );

    void tk_core_closeShellSocket(
        tk_core_ShellSocket *Socket_p, int pid
    );

    TK_CORE_RESULT tk_core_handleShellSocket(
        tk_core_ShellSocket *Socket_p
    );

/** @} */

#endif 
