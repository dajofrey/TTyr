// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "ttyr-terminal.h"
#include "ttyr-api.h"

#include "nh-core/Loader/Loader.h"

// TYPEDEFS ========================================================================================

typedef ttyr_terminal_Terminal *(*ttyr_terminal_openTerminal_f)( 
    char *config_p, ttyr_tty_TTY *TTY_p 
); 

typedef TTYR_TERMINAL_RESULT (*ttyr_terminal_cmd_setViewport_f)( 
    ttyr_terminal_Terminal *Terminal_p, nh_api_Viewport *Viewport_p 
); 

// FUNCTIONS =======================================================================================

static bool added = false;
static const char name_p[] = "ttyr-terminal";
static const char *dependencies_pp[16] = {
    "nh-gfx",
    "ttyr-tty",
};

static bool ttyr_api_add() {
    if (NH_LOADER_P == NULL) {
        return false;
    }
    if (!added) {
        NH_LOADER_P->addModule_f(name_p, TTYR_API_PATH_P, dependencies_pp, 1);
        added = true;
    }
    return added;
}

ttyr_terminal_Terminal *ttyr_api_openTerminal(
    char *config_p, ttyr_tty_TTY *TTY_p)
{
    if (!ttyr_api_add()) {return NULL;}

    ttyr_terminal_openTerminal_f openTerminal_f = !NH_LOADER_P || !TTY_p ? NULL : NH_LOADER_P->loadExternalSymbol_f(name_p, "ttyr_terminal_openTerminal");
    return openTerminal_f ? openTerminal_f(config_p, TTY_p) : NULL;
}

TTYR_TERMINAL_RESULT ttyr_api_setViewport(
    ttyr_terminal_Terminal *Terminal_p, nh_api_Viewport *Viewport_p)
{
    if (!ttyr_api_add()) {return TTYR_TERMINAL_ERROR_BAD_STATE;}

    ttyr_terminal_cmd_setViewport_f setViewport_f = !NH_LOADER_P || !Terminal_p || !Viewport_p ? NULL : NH_LOADER_P->loadExternalSymbol_f(name_p, "ttyr_terminal_cmd_setViewport");
    return setViewport_f ? setViewport_f(Terminal_p, Viewport_p) : TTYR_TERMINAL_ERROR_NULL_POINTER;
}
