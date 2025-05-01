// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Socket.h"

#include "../Common/Macros.h"

#include "../TTY/Program.h"
#include "../TTY/TTY.h"

#include "nh-core/System/Memory.h"
#include "nh-core/System/Thread.h"
#include "nh-core/Util/File.h"

#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <stdlib.h>

// SOCKET ==========================================================================================

static void tk_core_setUDSFilePath(
    char path_p[255], int pid)
{
    memset(path_p, 0, 255);
    sprintf(path_p, "/tmp/nhtty_%d.uds", pid); // UNIX Domain Socket file.
}

// https://openbook.rheinwerk-verlag.de/linux_unix_programmierung/Kap11-017.htm
TTYR_CORE_RESULT tk_core_createShellSocket(
    tk_core_ShellSocket *Socket_p, int pid)
{
    char path_p[255] = {0};
    tk_core_setUDSFilePath(path_p, pid);

    if ((Socket_p->fd = socket(AF_LOCAL, SOCK_STREAM | SOCK_NONBLOCK, 0)) <= 0) {
        return TTYR_CORE_ERROR_BAD_STATE;
    }

    unlink(path_p);

    memset(&Socket_p->address, 0, sizeof(struct sockaddr_un));
    Socket_p->address.sun_family = AF_LOCAL;
    strcpy(Socket_p->address.sun_path, path_p);

    if (bind(Socket_p->fd, (struct sockaddr *) &Socket_p->address, sizeof(Socket_p->address)) != 0) {
        return TTYR_CORE_ERROR_BAD_STATE;
    }

    listen(Socket_p->fd, 5);

    return TTYR_CORE_SUCCESS;
}

void tk_core_closeShellSocket(
    tk_core_ShellSocket *Socket_p, int pid)
{
    char path_p[255] = {0};
    tk_core_setUDSFilePath(path_p, pid);

    close(Socket_p->fd);
    unlink(path_p);
}

TTYR_CORE_RESULT tk_core_handleShellSocket(
    tk_core_ShellSocket *Socket_p)
{
    socklen_t addrlen = sizeof(struct sockaddr_in);
    int new_socket = accept(Socket_p->fd, (struct sockaddr *)&Socket_p->address, &addrlen);
   
    if (new_socket <= 0) {
        return TTYR_CORE_SUCCESS; // Nothing to do.
    }

    int buffer = 255;
    char buffer_p[buffer];
    memset(buffer_p, 0, buffer);
    int size = recv(new_socket, buffer_p, buffer-1, 0);
    if (size > 0) {
        TTYR_CORE_PROGRAM_E type = atoi(buffer_p);
//        tk_core_addProgram(nh_core_getWorkloadArg(), tk_core_createProgramPrototype(type), true);
    }

    close(new_socket);

    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT tk_core_sendCommandToShell(
    int pid, TTYR_CORE_PROGRAM_E type)
{
    char path_p[255] = {0};
    tk_core_setUDSFilePath(path_p, pid);

    if (!nh_fileExistsOnMachine(path_p, NULL)) {return TTYR_CORE_ERROR_BAD_STATE;}
 
    int fd;
    if ((fd=socket(PF_LOCAL, SOCK_STREAM, 0)) <= 0) {
        return TTYR_CORE_ERROR_BAD_STATE;
    }

    struct sockaddr_un address;
    address.sun_family = AF_LOCAL;
    strcpy(address.sun_path, path_p);

    if (connect(fd, (struct sockaddr *)&address, sizeof(address)) != 0) {
        return TTYR_CORE_ERROR_BAD_STATE;
    }

    char type_p[255] = {0};
    sprintf(type_p, "%d", type);

    send(fd, type_p, strlen(type_p), 0);
    close(fd);

    return TTYR_CORE_SUCCESS;
}

