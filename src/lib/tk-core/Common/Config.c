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

// FUNCTIONS =======================================================================================

static TK_CORE_RESULT tk_core_getSetting(
    tk_core_Config *Config_p, char *namespace_p, int index, char *setting_p)
{
    nh_core_List *Values_p = nh_core_getGlobalConfigSetting(namespace_p, -1, setting_p);
    TK_CHECK_NULL(Values_p)

    switch (index) {
        case 0 :
            if (Values_p->size != 1) {return TK_CORE_ERROR_BAD_STATE;}
            Config_p->Shell.maxScroll = atoi(Values_p->pp[0]);
            break;
        case 1 :
            if (Values_p->size != 1) {return TK_CORE_ERROR_BAD_STATE;}
            Config_p->windows = atoi(Values_p->pp[0]);
            break;
        case 2 :
            if (Values_p->size != 1) {return TK_CORE_ERROR_BAD_STATE;}
            Config_p->tabs = atoi(Values_p->pp[0]);
            break;
        case 3 :
            if (Values_p->size != 1) {return TK_CORE_ERROR_BAD_STATE;}
            Config_p->Menu.program = atoi(Values_p->pp[0]);
            break;
        case 4 :
            if (Values_p->size != 1) {return TK_CORE_ERROR_BAD_STATE;}
            Config_p->Menu.split = atoi(Values_p->pp[0]);
            break;
        case 5 :
            if (Values_p->size != 1) {return TK_CORE_ERROR_BAD_STATE;}
            Config_p->Menu.append = atoi(Values_p->pp[0]);
            break;
        case 6 :
            if (Values_p->size != 1) {return TK_CORE_ERROR_BAD_STATE;}
            Config_p->Menu.window = atoi(Values_p->pp[0]);
            break;
        case 7 :
            if (Values_p->size != 1) {return TK_CORE_ERROR_BAD_STATE;}
            Config_p->Menu.tab = atoi(Values_p->pp[0]);
            break;
        case 8 :
            if (Values_p->size != 1) {return TK_CORE_ERROR_BAD_STATE;}
            Config_p->Menu.close = atoi(Values_p->pp[0]);
            break;
        case 9 :
            if (Values_p->size != 1) {return TK_CORE_ERROR_BAD_STATE;}
            Config_p->Menu.debug = atoi(Values_p->pp[0]);
            break;
        case 10 :
            if (Values_p->size != 1) {return TK_CORE_ERROR_BAD_STATE;}
            Config_p->Titlebar.on = atoi(Values_p->pp[0]);
            break;
        case 11 :
            if (Values_p->size != 4) {return TK_CORE_ERROR_BAD_STATE;} 
            Config_p->Titlebar.Foreground.r = ((float)atoi(Values_p->pp[0]))/255.0f; 
            Config_p->Titlebar.Foreground.g = ((float)atoi(Values_p->pp[1]))/255.0f; 
            Config_p->Titlebar.Foreground.b = ((float)atoi(Values_p->pp[2]))/255.0f; 
            Config_p->Titlebar.Foreground.a = ((float)atoi(Values_p->pp[3]))/255.0f; 
            break;
        case 12 :
            if (Values_p->size != 4) {return TK_CORE_ERROR_BAD_STATE;} 
            Config_p->Titlebar.Background.r = ((float)atoi(Values_p->pp[0]))/255.0f; 
            Config_p->Titlebar.Background.g = ((float)atoi(Values_p->pp[1]))/255.0f; 
            Config_p->Titlebar.Background.b = ((float)atoi(Values_p->pp[2]))/255.0f; 
            Config_p->Titlebar.Background.a = ((float)atoi(Values_p->pp[3]))/255.0f; 
            break;
        case 13 :
            if (Values_p->size != 1) {return TK_CORE_ERROR_BAD_STATE;}
            Config_p->Topbar.on = atoi(Values_p->pp[0]);
            break;
    }

    return TK_CORE_SUCCESS;
}

static tk_core_Config tk_core_getStaticConfig()
{
    tk_core_Config Config;
    memset(&Config, 0, sizeof(tk_core_Config));

    static const char *options_pp[] = {
        "tk-core.shell.maxScroll",
        "tk-core.windows",
        "tk-core.tabs",
        "tk-core.menu.program",
        "tk-core.menu.split",
        "tk-core.menu.append",
        "tk-core.menu.window",
        "tk-core.menu.tab",
        "tk-core.menu.close",
        "tk-core.menu.debug",
        "tk-core.titlebar.on",
        "tk-core.titlebar.foreground",
        "tk-core.titlebar.background",
        "tk-core.topbar.on",
    };

    int options = sizeof(options_pp)/sizeof(options_pp[0]);

    tk_core_TTY *TTY_p = nh_core_getWorkloadArg();
    TK_CHECK_NULL_2(Config, TTY_p)

    for (int i = 0; i < options; ++i) {
        TK_CHECK_2(Config, tk_core_getSetting(&Config, TTY_p->namespace_p, i, options_pp[i]))
    }

    return Config;
}

tk_core_Config tk_core_updateConfig( 
    void *TTY_p) 
{ 
    ((tk_core_TTY*)TTY_p)->Config = tk_core_getStaticConfig(); 
    if (((tk_core_TTY*)TTY_p)->Views.size > 0 && ((tk_core_TTY*)TTY_p)->Views.pp != NULL && ((tk_core_View*)((tk_core_TTY*)TTY_p)->Views.pp[0])->standardIO) {
        ((tk_core_TTY*)TTY_p)->Config.Titlebar.on = false;
        ((tk_core_TTY*)TTY_p)->Config.Topbar.on = false;
    }
    return ((tk_core_TTY*)TTY_p)->Config; 
} 
