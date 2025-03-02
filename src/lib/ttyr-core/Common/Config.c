// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Config.h"
#include "Macros.h"

#include "../TTY/TTY.h"

#include "nh-core/Config/Config.h"
#include "nh-core/System/Memory.h"
#include "nh-core/System/Thread.h"

#include <string.h>
#include <stdlib.h>

// NAMES ===========================================================================================

static const char *TTYR_CORE_SETTING_NAMES_PP[] = {
    "ttyr-core.shell.maxScroll",
    "ttyr-core.windows",
    "ttyr-core.tabs",
    "ttyr-core.menu.program",
    "ttyr-core.menu.split",
    "ttyr-core.menu.append",
    "ttyr-core.menu.window",
    "ttyr-core.menu.tab",
    "ttyr-core.menu.close",
    "ttyr-core.menu.debug",
    "ttyr-core.titlebar.on",
    "ttyr-core.titlebar.foreground",
    "ttyr-core.titlebar.background",
    "ttyr-core.topbar.on",
};

static size_t TTYR_CORE_SETTING_NAMES_PP_COUNT = 
    sizeof(TTYR_CORE_SETTING_NAMES_PP) / sizeof(TTYR_CORE_SETTING_NAMES_PP[0]);

const char *ttyr_core_getSettingName(
    unsigned int setting)
{
    return TTYR_CORE_SETTING_NAMES_PP[setting];
}

// FUNCTIONS =======================================================================================

static TTYR_CORE_RESULT ttyr_core_getSetting(
    ttyr_core_Config *Config_p, char *namespace_p, int index)
{
    nh_core_List *Values_p = nh_core_getGlobalConfigSetting(namespace_p, -1, TTYR_CORE_SETTING_NAMES_PP[index]);
    TTYR_CHECK_NULL(Values_p)

    switch (index) {
        case 0 :
            if (Values_p->size != 1) {return TTYR_CORE_ERROR_BAD_STATE;}
            Config_p->Shell.maxScroll = atoi(Values_p->pp[0]);
            break;
        case 1 :
            if (Values_p->size != 1) {return TTYR_CORE_ERROR_BAD_STATE;}
            Config_p->windows = atoi(Values_p->pp[0]);
            break;
        case 2 :
            if (Values_p->size != 1) {return TTYR_CORE_ERROR_BAD_STATE;}
            Config_p->tabs = atoi(Values_p->pp[0]);
            break;
        case 3 :
            if (Values_p->size != 1) {return TTYR_CORE_ERROR_BAD_STATE;}
            Config_p->Menu.program = atoi(Values_p->pp[0]);
            break;
        case 4 :
            if (Values_p->size != 1) {return TTYR_CORE_ERROR_BAD_STATE;}
            Config_p->Menu.split = atoi(Values_p->pp[0]);
            break;
        case 5 :
            if (Values_p->size != 1) {return TTYR_CORE_ERROR_BAD_STATE;}
            Config_p->Menu.append = atoi(Values_p->pp[0]);
            break;
        case 6 :
            if (Values_p->size != 1) {return TTYR_CORE_ERROR_BAD_STATE;}
            Config_p->Menu.window = atoi(Values_p->pp[0]);
            break;
        case 7 :
            if (Values_p->size != 1) {return TTYR_CORE_ERROR_BAD_STATE;}
            Config_p->Menu.tab = atoi(Values_p->pp[0]);
            break;
        case 8 :
            if (Values_p->size != 1) {return TTYR_CORE_ERROR_BAD_STATE;}
            Config_p->Menu.close = atoi(Values_p->pp[0]);
            break;
        case 9 :
            if (Values_p->size != 1) {return TTYR_CORE_ERROR_BAD_STATE;}
            Config_p->Menu.debug = atoi(Values_p->pp[0]);
            break;
        case 10 :
            if (Values_p->size != 1) {return TTYR_CORE_ERROR_BAD_STATE;}
            Config_p->Titlebar.on = atoi(Values_p->pp[0]);
            break;
        case 11 :
            if (Values_p->size != 4) {return TTYR_CORE_ERROR_BAD_STATE;} 
            Config_p->Titlebar.Foreground.r = ((float)atoi(Values_p->pp[0]))/255.0f; 
            Config_p->Titlebar.Foreground.g = ((float)atoi(Values_p->pp[1]))/255.0f; 
            Config_p->Titlebar.Foreground.b = ((float)atoi(Values_p->pp[2]))/255.0f; 
            Config_p->Titlebar.Foreground.a = ((float)atoi(Values_p->pp[3]))/255.0f; 
            break;
        case 12 :
            if (Values_p->size != 4) {return TTYR_CORE_ERROR_BAD_STATE;} 
            Config_p->Titlebar.Background.r = ((float)atoi(Values_p->pp[0]))/255.0f; 
            Config_p->Titlebar.Background.g = ((float)atoi(Values_p->pp[1]))/255.0f; 
            Config_p->Titlebar.Background.b = ((float)atoi(Values_p->pp[2]))/255.0f; 
            Config_p->Titlebar.Background.a = ((float)atoi(Values_p->pp[3]))/255.0f; 
            break;
        case 13 :
            if (Values_p->size != 1) {return TTYR_CORE_ERROR_BAD_STATE;}
            Config_p->Topbar.on = atoi(Values_p->pp[0]);
            break;
    }

    return TTYR_CORE_SUCCESS;
}

static ttyr_core_Config ttyr_core_getStaticConfig()
{
    ttyr_core_Config Config;
    memset(&Config, 0, sizeof(ttyr_core_Config));

    ttyr_core_TTY *TTY_p = nh_core_getWorkloadArg();
    TTYR_CHECK_NULL_2(Config, TTY_p)

    for (int i = 0; i < TTYR_CORE_SETTING_NAMES_PP_COUNT; ++i) {
        TTYR_CHECK_2(Config, ttyr_core_getSetting(&Config, TTY_p->namespace_p, i))
    }

    return Config;
}

ttyr_core_Config ttyr_core_getConfig()
{
    ttyr_core_Config Config = ttyr_core_getStaticConfig();
    ttyr_core_TTY *TTY_p = nh_core_getWorkloadArg();
    if (((ttyr_core_View*)TTY_p->Views.pp[0])->standardIO) {
        Config.Titlebar.on = false;
    }
    return Config;
}
