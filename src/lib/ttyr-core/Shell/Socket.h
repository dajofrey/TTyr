#ifndef TTYR_CORE_SHELL_SOCKET_H
#define TTYR_CORE_SHELL_SOCKET_H

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

    typedef struct ttyr_core_ShellSocket {
        struct sockaddr_un address;
        int fd;
        uint16_t port;
    } ttyr_core_ShellSocket;

/** @} */

/** @addtogroup lib_nhtty_typedefs
 *  @{
 */

    typedef TTYR_CORE_RESULT (*ttyr_core_sendCommandToShell_f)(
        int pid, TTYR_CORE_PROGRAM_E type 
    );

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    TTYR_CORE_RESULT ttyr_core_createShellSocket(
        ttyr_core_ShellSocket *Socket_p, int pid
    );

    void ttyr_core_closeShellSocket(
        ttyr_core_ShellSocket *Socket_p, int pid
    );

    TTYR_CORE_RESULT ttyr_core_handleShellSocket(
        ttyr_core_ShellSocket *Socket_p
    );

/** @} */

#endif 
