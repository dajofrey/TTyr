// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "ttyr-core.h"
#include "ttyr-api.h"

#include "nh-core/Loader/Loader.h"

#include <stdio.h>
#include <stdlib.h>

// TYPEDEFS ========================================================================================

typedef ttyr_core_TTY *(*ttyr_core_openTTY_f)( 
    char *config_p, ttyr_core_Interface *Interface_p 
); 

typedef TTYR_CORE_RESULT (*ttyr_core_closeTTY_f)( 
    ttyr_core_TTY *TTY_p 
); 

typedef TTYR_CORE_RESULT (*ttyr_core_cmd_claimStandardIO_f)( 
    ttyr_core_TTY *TTY_p 
); 

typedef TTYR_CORE_RESULT (*ttyr_core_cmd_unclaimStandardIO_f)( 
    ttyr_core_TTY *TTY_p 
); 

typedef TTYR_CORE_RESULT (*ttyr_core_cmd_sendEvent_f)( 
    ttyr_core_TTY *TTY_p, nh_api_WSIEvent Event 
); 
 
// ADD =============================================================================================

static bool added = false;
static const char name_p[] = "ttyr-core";
static const char *dependencies_pp[16] = {
    "nh-encoding",
};

static bool ttyr_api_add() {
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

ttyr_core_TTY *ttyr_api_openTTY(
    char *config_p, ttyr_core_Interface *Interface_p)
{
    if (!ttyr_api_add()) {return NULL;}
    nh_core_Loader *Loader_p = nh_api_getLoader();
    ttyr_core_openTTY_f openTTY_f = !Loader_p ? NULL : Loader_p->loadExternalSymbol_f(name_p, "ttyr_core_openTTY");
    return openTTY_f ? openTTY_f(config_p, Interface_p) : NULL;
}

TTYR_CORE_RESULT ttyr_api_closeTTY(
    ttyr_core_TTY *TTY_p)
{
    if (!ttyr_api_add()) {return TTYR_CORE_ERROR_BAD_STATE;}
    nh_core_Loader *Loader_p = nh_api_getLoader();
    ttyr_core_closeTTY_f closeTTY_f = !Loader_p ? NULL : Loader_p->loadExternalSymbol_f(name_p, "ttyr_core_closeTTY");
    return closeTTY_f ? closeTTY_f(TTY_p) : TTYR_CORE_ERROR_BAD_STATE;
}

TTYR_CORE_RESULT ttyr_api_claimStandardIO(
    ttyr_core_TTY *TTY_p)
{
    if (!ttyr_api_add()) {return TTYR_CORE_ERROR_BAD_STATE;}
    nh_core_Loader *Loader_p = nh_api_getLoader();
    ttyr_core_cmd_claimStandardIO_f claimStandardIO_f = !Loader_p ? NULL : Loader_p->loadExternalSymbol_f(name_p, "ttyr_core_cmd_claimStandardIO");
    return claimStandardIO_f ? claimStandardIO_f(TTY_p) : TTYR_CORE_ERROR_BAD_STATE;
}

TTYR_CORE_RESULT ttyr_api_unclaimStandardIO(
    ttyr_core_TTY *TTY_p)
{
    if (!ttyr_api_add()) {return TTYR_CORE_ERROR_BAD_STATE;}
    nh_core_Loader *Loader_p = nh_api_getLoader();
    ttyr_core_cmd_unclaimStandardIO_f unclaimStandardIO_f = !Loader_p ? NULL : Loader_p->loadExternalSymbol_f(name_p, "ttyr_core_cmd_unclaimStandardIO");
    return unclaimStandardIO_f ? unclaimStandardIO_f(TTY_p) : TTYR_CORE_ERROR_BAD_STATE;
}

TTYR_CORE_RESULT ttyr_api_sendEvent(
    ttyr_core_TTY *TTY_p, nh_api_WSIEvent Event)
{
    if (!ttyr_api_add()) {return TTYR_CORE_ERROR_BAD_STATE;}
    nh_core_Loader *Loader_p = nh_api_getLoader();
    ttyr_core_cmd_sendEvent_f sendEvent_f = !Loader_p ? NULL : Loader_p->loadExternalSymbol_f(name_p, "ttyr_core_cmd_sendEvent");
    return sendEvent_f ? sendEvent_f(TTY_p, Event) : TTYR_CORE_ERROR_BAD_STATE;
}
