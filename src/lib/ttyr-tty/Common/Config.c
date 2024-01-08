// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Config.h"
#include "IndexMap.h"
#include "Macros.h"

#include "../TTY/TTY.h"

#include "../../../../external/Netzhaut/src/lib/nhcore/Config/Config.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/System/Memory.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/System/Thread.h"

#include <string.h>
#include <stdlib.h>

// NAMES ===========================================================================================

const NH_BYTE *TTYR_TTY_SETTING_NAMES_PP[] = {
    "ttyr.tty.shell.maxScroll",
    "ttyr.tty.windows",
    "ttyr.tty.tabs",
    "ttyr.tty.menu.program",
    "ttyr.tty.menu.split",
    "ttyr.tty.menu.append",
    "ttyr.tty.menu.window",
    "ttyr.tty.menu.tab",
    "ttyr.tty.menu.close",
    "ttyr.tty.menu.debug",
    "ttyr.tty.titlebar.on",
    "ttyr.tty.titlebar.color",
};

size_t TTYR_TTY_SETTING_NAMES_PP_COUNT = 
    sizeof(TTYR_TTY_SETTING_NAMES_PP) / sizeof(TTYR_TTY_SETTING_NAMES_PP[0]);

const NH_BYTE *ttyr_tty_getSettingName(
    TTYR_TTY_SETTING_E setting)
{
TTYR_TTY_BEGIN()
TTYR_TTY_END(TTYR_TTY_SETTING_NAMES_PP[setting])
}

// FUNCTIONS =======================================================================================

static TTYR_TTY_RESULT ttyr_tty_getSetting(
    ttyr_tty_Config *Config_p, NH_BYTE *namespace_p, NH_BYTE *setting_p)
{
TTYR_TTY_BEGIN()

    unsigned int *index_p = nh_core_getFromHashMap(&TTYR_TTY_INDEXMAP.SettingNames, setting_p);
    if (index_p == NULL) {TTYR_TTY_END(TTYR_TTY_ERROR_BAD_STATE)}

    nh_List *Values_p = nh_core_getGlobalConfigSetting(namespace_p, -1, setting_p);
    TTYR_TTY_CHECK_NULL(Values_p)

    switch (*index_p) {
        case 0 :
            if (Values_p->size != 1) {TTYR_TTY_END(TTYR_TTY_ERROR_BAD_STATE)}
            Config_p->Shell.maxScroll = atoi(Values_p->pp[0]);
            break;
        case 1 :
            if (Values_p->size != 1) {TTYR_TTY_END(TTYR_TTY_ERROR_BAD_STATE)}
            Config_p->windows = atoi(Values_p->pp[0]);
            break;
        case 2 :
            if (Values_p->size != 1) {TTYR_TTY_END(TTYR_TTY_ERROR_BAD_STATE)}
            Config_p->tabs = atoi(Values_p->pp[0]);
            break;
        case 3 :
            if (Values_p->size != 1) {TTYR_TTY_END(TTYR_TTY_ERROR_BAD_STATE)}
            Config_p->Menu.program = atoi(Values_p->pp[0]);
            break;
        case 4 :
            if (Values_p->size != 1) {TTYR_TTY_END(TTYR_TTY_ERROR_BAD_STATE)}
            Config_p->Menu.split = atoi(Values_p->pp[0]);
            break;
        case 5 :
            if (Values_p->size != 1) {TTYR_TTY_END(TTYR_TTY_ERROR_BAD_STATE)}
            Config_p->Menu.append = atoi(Values_p->pp[0]);
            break;
        case 6 :
            if (Values_p->size != 1) {TTYR_TTY_END(TTYR_TTY_ERROR_BAD_STATE)}
            Config_p->Menu.window = atoi(Values_p->pp[0]);
            break;
        case 7 :
            if (Values_p->size != 1) {TTYR_TTY_END(TTYR_TTY_ERROR_BAD_STATE)}
            Config_p->Menu.tab = atoi(Values_p->pp[0]);
            break;
        case 8 :
            if (Values_p->size != 1) {TTYR_TTY_END(TTYR_TTY_ERROR_BAD_STATE)}
            Config_p->Menu.close = atoi(Values_p->pp[0]);
            break;
        case 9 :
            if (Values_p->size != 1) {TTYR_TTY_END(TTYR_TTY_ERROR_BAD_STATE)}
            Config_p->Menu.debug = atoi(Values_p->pp[0]);
            break;
        case 10 :
            if (Values_p->size != 1) {TTYR_TTY_END(TTYR_TTY_ERROR_BAD_STATE)}
            Config_p->Titlebar.on = atoi(Values_p->pp[0]);
            break;
        case 11 :
            if (Values_p->size != 4) {TTYR_TTY_END(TTYR_TTY_ERROR_BAD_STATE)} 
            Config_p->Titlebar.Color.r = ((float)atoi(Values_p->pp[0]))/255.0f; 
            Config_p->Titlebar.Color.g = ((float)atoi(Values_p->pp[1]))/255.0f; 
            Config_p->Titlebar.Color.b = ((float)atoi(Values_p->pp[2]))/255.0f; 
            Config_p->Titlebar.Color.a = ((float)atoi(Values_p->pp[3]))/255.0f; 
            break;
    }

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

ttyr_tty_Config ttyr_tty_getConfig()
{
TTYR_TTY_BEGIN()

    ttyr_tty_Config Config;
    memset(&Config, 0, sizeof(ttyr_tty_Config));

    ttyr_tty_TTY *TTY_p = nh_core_getWorkloadArg();
    TTYR_TTY_CHECK_NULL_2(Config, TTY_p)

    for (int i = 0; i < TTYR_TTY_SETTING_NAMES_PP_COUNT; ++i) {
        TTYR_TTY_CHECK_2(Config, ttyr_tty_getSetting(&Config, TTY_p->namespace_p, TTYR_TTY_SETTING_NAMES_PP[i]))
    }

TTYR_TTY_END(Config)
}

