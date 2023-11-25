#ifndef TTYR_TTY_SHELL_SOCKET_H
#define TTYR_TTY_SHELL_SOCKET_H

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

    typedef struct ttyr_tty_ShellSocket {
        struct sockaddr_un address;
        int fd;
        uint16_t port;
    } ttyr_tty_ShellSocket;

/** @} */

/** @addtogroup lib_nhtty_typedefs
 *  @{
 */

    typedef TTYR_TTY_RESULT (*ttyr_tty_sendCommandToShell_f)(
        int pid, TTYR_TTY_PROGRAM_E type 
    );

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    TTYR_TTY_RESULT ttyr_tty_createShellSocket(
        ttyr_tty_ShellSocket *Socket_p, int pid
    );

    void ttyr_tty_closeShellSocket(
        ttyr_tty_ShellSocket *Socket_p, int pid
    );

    TTYR_TTY_RESULT ttyr_tty_handleShellSocket(
        ttyr_tty_ShellSocket *Socket_p
    );

/** @} */

#endif 
