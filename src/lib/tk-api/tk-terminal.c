// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "tk-terminal.h"
#include "tk-api.h"

#include "nh-core/Loader/Loader.h"

// FUNCTIONS =======================================================================================

static bool added = false;
static const char name_p[] = "tk-terminal";
static const char *dependencies_pp[16] = {
    "nh-gfx",
    "tk-core",
};

static bool tk_api_add() {
    nh_core_Loader *Loader_p = nh_api_getLoader();
    if (Loader_p == NULL) {
        return false;
    }
    if (!added) {
        Loader_p->addModule_f(name_p, TTYR_API_PATH_P, dependencies_pp, 1);
        added = true;
    }
    return added;
}

tk_terminal_Terminal *tk_api_openTerminal(
    char *config_p, tk_core_TTY *TTY_p)
{
    typedef tk_terminal_Terminal *(*tk_terminal_openTerminal_f)(char *config_p, tk_core_TTY *TTY_p);
    if (!tk_api_add()) {return NULL;}
    nh_core_Loader *Loader_p = nh_api_getLoader();
    tk_terminal_openTerminal_f openTerminal_f = !Loader_p || !TTY_p ? NULL : Loader_p->loadExternalSymbol_f(name_p, "tk_terminal_openTerminal");
    return openTerminal_f ? openTerminal_f(config_p, TTY_p) : NULL;
}

TTYR_TERMINAL_RESULT tk_api_setViewport(
    tk_terminal_Terminal *Terminal_p, nh_api_Viewport *Viewport_p)
{
    typedef TTYR_TERMINAL_RESULT (*tk_terminal_cmd_setViewport_f)(tk_terminal_Terminal *Terminal_p, nh_api_Viewport *Viewport_p); 
    if (!tk_api_add()) {return TTYR_TERMINAL_ERROR_BAD_STATE;}
    nh_core_Loader *Loader_p = nh_api_getLoader();
    tk_terminal_cmd_setViewport_f setViewport_f = !Loader_p || !Terminal_p || !Viewport_p ? NULL : Loader_p->loadExternalSymbol_f(name_p, "tk_terminal_cmd_setViewport");
    return setViewport_f ? setViewport_f(Terminal_p, Viewport_p) : TTYR_TERMINAL_ERROR_NULL_POINTER;
}
