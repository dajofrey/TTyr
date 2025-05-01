// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "tk-core.h"
#include "tk-api.h"

#include "nh-core/Loader/Loader.h"

#include <stdio.h>
#include <stdlib.h>

// FUNCTIONS ========================================================================================

static bool added = false;
static const char name_p[] = "tk-core";
static const char *dependencies_pp[16] = {
    "nh-encoding",
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

tk_core_TTY *tk_api_openTTY(
    char *config_p, tk_core_Interface *Interface_p)
{
    if (!tk_api_add()) {return NULL;}
    typedef tk_core_TTY *(*tk_core_openTTY_f)( char *config_p, tk_core_Interface *Interface_p); 
    nh_core_Loader *Loader_p = nh_api_getLoader();
    tk_core_openTTY_f openTTY_f = !Loader_p ? NULL : Loader_p->loadExternalSymbol_f(name_p, "tk_core_openTTY");
    return openTTY_f ? openTTY_f(config_p, Interface_p) : NULL;
}

TTYR_CORE_RESULT tk_api_closeTTY(
    tk_core_TTY *TTY_p)
{
    if (!tk_api_add()) {return TTYR_CORE_ERROR_BAD_STATE;}
    typedef TTYR_CORE_RESULT (*tk_core_closeTTY_f)(tk_core_TTY *TTY_p);
    nh_core_Loader *Loader_p = nh_api_getLoader();
    tk_core_closeTTY_f closeTTY_f = !Loader_p ? NULL : Loader_p->loadExternalSymbol_f(name_p, "tk_core_closeTTY");
    return closeTTY_f ? closeTTY_f(TTY_p) : TTYR_CORE_ERROR_BAD_STATE;
}

TTYR_CORE_RESULT tk_api_claimStandardIO(
    tk_core_TTY *TTY_p)
{
    if (!tk_api_add()) {return TTYR_CORE_ERROR_BAD_STATE;}
    typedef TTYR_CORE_RESULT (*tk_core_cmd_claimStandardIO_f)(tk_core_TTY *TTY_p);
    nh_core_Loader *Loader_p = nh_api_getLoader();
    tk_core_cmd_claimStandardIO_f claimStandardIO_f = !Loader_p ? NULL : Loader_p->loadExternalSymbol_f(name_p, "tk_core_cmd_claimStandardIO");
    return claimStandardIO_f ? claimStandardIO_f(TTY_p) : TTYR_CORE_ERROR_BAD_STATE;
}

TTYR_CORE_RESULT tk_api_unclaimStandardIO(
    tk_core_TTY *TTY_p)
{
    if (!tk_api_add()) {return TTYR_CORE_ERROR_BAD_STATE;}
    typedef TTYR_CORE_RESULT (*tk_core_cmd_unclaimStandardIO_f)(tk_core_TTY *TTY_p); 
    nh_core_Loader *Loader_p = nh_api_getLoader();
    tk_core_cmd_unclaimStandardIO_f unclaimStandardIO_f = !Loader_p ? NULL : Loader_p->loadExternalSymbol_f(name_p, "tk_core_cmd_unclaimStandardIO");
    return unclaimStandardIO_f ? unclaimStandardIO_f(TTY_p) : TTYR_CORE_ERROR_BAD_STATE;
}

TTYR_CORE_RESULT tk_api_sendEvent(
    tk_core_TTY *TTY_p, nh_api_WSIEvent Event)
{
    if (!tk_api_add()) {return TTYR_CORE_ERROR_BAD_STATE;}
    typedef TTYR_CORE_RESULT (*tk_core_cmd_sendEvent_f)(tk_core_TTY *TTY_p, nh_api_WSIEvent Event);
    nh_core_Loader *Loader_p = nh_api_getLoader();
    tk_core_cmd_sendEvent_f sendEvent_f = !Loader_p ? NULL : Loader_p->loadExternalSymbol_f(name_p, "tk_core_cmd_sendEvent");
    return sendEvent_f ? sendEvent_f(TTY_p, Event) : TTYR_CORE_ERROR_BAD_STATE;
}
