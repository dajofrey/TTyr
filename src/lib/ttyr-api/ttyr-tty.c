// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "ttyr-tty.h"
#include "../ttyr-tty/TTY/TTY.h"

#include "nhcore/Loader/Loader.h"

// ADD =============================================================================================

static bool initialized = false;
static const char name_p[] = "ttyr-tty";
static const char *dependencies_pp[16] = {
    "nhencoding",
};

static void ttyr_api_initialize() {
    if (!initialized && NH_LOADER_P != NULL) {
        NH_LOADER_P->addModule_f(name_p, dependencies_pp, 1);
        initialized = true;
    }
}

ttyr_tty_TTY *ttyr_api_openTTY(
    NH_BYTE *config_p)
{
    ttyr_api_initialize();
    ttyr_tty_openTTY_f openTTY_f = !NH_LOADER_P ? NULL : NH_LOADER_P->loadExternalSymbol_f(name_p, "ttyr_tty_openTTY");
    return openTTY_f ? openTTY_f(config_p) : NULL;
}

TTYR_TTY_RESULT ttyr_api_closeTTY(
    ttyr_tty_TTY *TTY_p)
{
    ttyr_api_initialize();
    ttyr_tty_closeTTY_f closeTTY_f = !NH_LOADER_P ? NULL : NH_LOADER_P->loadExternalSymbol_f(name_p, "ttyr_tty_closeTTY");
    return closeTTY_f ? closeTTY_f(TTY_p) : TTYR_TTY_ERROR_BAD_STATE;
}

TTYR_TTY_RESULT ttyr_api_claimStandardIO(
    ttyr_tty_TTY *TTY_p)
{
    ttyr_api_initialize();
    ttyr_tty_cmd_claimStandardIO_f claimStandardIO_f = !NH_LOADER_P ? NULL : NH_LOADER_P->loadExternalSymbol_f(name_p, "ttyr_tty_cmd_claimStandardIO");
    return claimStandardIO_f ? claimStandardIO_f(TTY_p) : TTYR_TTY_ERROR_BAD_STATE;
}

TTYR_TTY_RESULT ttyr_api_unclaimStandardIO(
    ttyr_tty_TTY *TTY_p)
{
    ttyr_api_initialize();
    ttyr_tty_cmd_unclaimStandardIO_f unclaimStandardIO_f = !NH_LOADER_P ? NULL : NH_LOADER_P->loadExternalSymbol_f(name_p, "ttyr_tty_cmd_unclaimStandardIO");
    return unclaimStandardIO_f ? unclaimStandardIO_f(TTY_p) : TTYR_TTY_ERROR_BAD_STATE;
}

TTYR_TTY_RESULT ttyr_api_sendEvent(
    ttyr_tty_TTY *TTY_p, nh_wsi_Event Event)
{
    ttyr_api_initialize();
    ttyr_tty_cmd_sendEvent_f sendEvent_f = !NH_LOADER_P ? NULL : NH_LOADER_P->loadExternalSymbol_f(name_p, "ttyr_tty_cmd_sendEvent");
    return sendEvent_f ? sendEvent_f(TTY_p, Event) : TTYR_TTY_ERROR_BAD_STATE;
}

