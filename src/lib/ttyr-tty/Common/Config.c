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

static const char *TTYR_TTY_SETTING_NAMES_PP[] = {
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
    "ttyr.tty.titlebar.foreground",
    "ttyr.tty.titlebar.background",
};

static size_t TTYR_TTY_SETTING_NAMES_PP_COUNT = 
    sizeof(TTYR_TTY_SETTING_NAMES_PP) / sizeof(TTYR_TTY_SETTING_NAMES_PP[0]);

const char *ttyr_tty_getSettingName(
    unsigned int setting)
{
    return TTYR_TTY_SETTING_NAMES_PP[setting];
}

// FUNCTIONS =======================================================================================

static TTYR_TTY_RESULT ttyr_tty_getSetting(
    ttyr_tty_Config *Config_p, char *namespace_p, int index)
{
    nh_core_List *Values_p = nh_core_getGlobalConfigSetting(namespace_p, -1, TTYR_TTY_SETTING_NAMES_PP[index]);
    TTYR_CHECK_NULL(Values_p)

    switch (index) {
        case 0 :
            if (Values_p->size != 1) {return TTYR_TTY_ERROR_BAD_STATE;}
            Config_p->Shell.maxScroll = atoi(Values_p->pp[0]);
            break;
        case 1 :
            if (Values_p->size != 1) {return TTYR_TTY_ERROR_BAD_STATE;}
            Config_p->windows = atoi(Values_p->pp[0]);
            break;
        case 2 :
            if (Values_p->size != 1) {return TTYR_TTY_ERROR_BAD_STATE;}
            Config_p->tabs = atoi(Values_p->pp[0]);
            break;
        case 3 :
            if (Values_p->size != 1) {return TTYR_TTY_ERROR_BAD_STATE;}
            Config_p->Menu.program = atoi(Values_p->pp[0]);
            break;
        case 4 :
            if (Values_p->size != 1) {return TTYR_TTY_ERROR_BAD_STATE;}
            Config_p->Menu.split = atoi(Values_p->pp[0]);
            break;
        case 5 :
            if (Values_p->size != 1) {return TTYR_TTY_ERROR_BAD_STATE;}
            Config_p->Menu.append = atoi(Values_p->pp[0]);
            break;
        case 6 :
            if (Values_p->size != 1) {return TTYR_TTY_ERROR_BAD_STATE;}
            Config_p->Menu.window = atoi(Values_p->pp[0]);
            break;
        case 7 :
            if (Values_p->size != 1) {return TTYR_TTY_ERROR_BAD_STATE;}
            Config_p->Menu.tab = atoi(Values_p->pp[0]);
            break;
        case 8 :
            if (Values_p->size != 1) {return TTYR_TTY_ERROR_BAD_STATE;}
            Config_p->Menu.close = atoi(Values_p->pp[0]);
            break;
        case 9 :
            if (Values_p->size != 1) {return TTYR_TTY_ERROR_BAD_STATE;}
            Config_p->Menu.debug = atoi(Values_p->pp[0]);
            break;
        case 10 :
            if (Values_p->size != 1) {return TTYR_TTY_ERROR_BAD_STATE;}
            Config_p->Titlebar.on = atoi(Values_p->pp[0]);
            break;
        case 11 :
            if (Values_p->size != 4) {return TTYR_TTY_ERROR_BAD_STATE;} 
            Config_p->Titlebar.Foreground.r = ((float)atoi(Values_p->pp[0]))/255.0f; 
            Config_p->Titlebar.Foreground.g = ((float)atoi(Values_p->pp[1]))/255.0f; 
            Config_p->Titlebar.Foreground.b = ((float)atoi(Values_p->pp[2]))/255.0f; 
            Config_p->Titlebar.Foreground.a = ((float)atoi(Values_p->pp[3]))/255.0f; 
            break;
        case 12 :
            if (Values_p->size != 4) {return TTYR_TTY_ERROR_BAD_STATE;} 
            Config_p->Titlebar.Background.r = ((float)atoi(Values_p->pp[0]))/255.0f; 
            Config_p->Titlebar.Background.g = ((float)atoi(Values_p->pp[1]))/255.0f; 
            Config_p->Titlebar.Background.b = ((float)atoi(Values_p->pp[2]))/255.0f; 
            Config_p->Titlebar.Background.a = ((float)atoi(Values_p->pp[3]))/255.0f; 
            break;
    }

    return TTYR_TTY_SUCCESS;
}

static ttyr_tty_Config ttyr_tty_getStaticConfig()
{
    ttyr_tty_Config Config;
    memset(&Config, 0, sizeof(ttyr_tty_Config));

    ttyr_tty_TTY *TTY_p = nh_core_getWorkloadArg();
    TTYR_CHECK_NULL_2(Config, TTY_p)

    for (int i = 0; i < TTYR_TTY_SETTING_NAMES_PP_COUNT; ++i) {
        TTYR_CHECK_2(Config, ttyr_tty_getSetting(&Config, TTY_p->namespace_p, i))
    }

    return Config;
}

ttyr_tty_Config ttyr_tty_getConfig()
{
    ttyr_tty_Config Config = ttyr_tty_getStaticConfig();
    ttyr_tty_TTY *TTY_p = nh_core_getWorkloadArg();
    if (((ttyr_tty_View*)TTY_p->Views.pp[0])->standardIO) {
        Config.Titlebar.on = false;
    }
    return Config;
}
