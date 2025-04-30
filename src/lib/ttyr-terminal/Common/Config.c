// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Config.h"
#include "Macros.h"

#include "../Terminal/Terminal.h"

#include "nh-core/Config/Config.h"
#include "nh-core/System/Memory.h"
#include "nh-core/System/Thread.h"

#include <string.h>
#include <stdlib.h>

// FUNCTIONS =======================================================================================

static TTYR_TERMINAL_RESULT ttyr_terminal_getSetting(
    ttyr_terminal_Config *Config_p, char namespace_p[255], int setting, char *setting_p)
{
    nh_core_List *Setting_p = nh_core_getGlobalConfigSetting(namespace_p, -1, setting_p);
    TTYR_TERMINAL_CHECK_NULL(Setting_p)

    switch (setting) {
        case 0 :
            if (Setting_p->size != 1) {return TTYR_TERMINAL_ERROR_BAD_STATE;}
            Config_p->fontSize = atoi(Setting_p->pp[0]);
            break;
        case 1 :
            if (Setting_p->size != 1) {return TTYR_TERMINAL_ERROR_BAD_STATE;}
            Config_p->blinkFrequency = atof(Setting_p->pp[0]);
            break;
        case 2 :
            if (Setting_p->size != 4) {return TTYR_TERMINAL_ERROR_BAD_STATE;}
            Config_p->Foreground.r = ((float)atoi(Setting_p->pp[0]))/255.0f;
            Config_p->Foreground.g = ((float)atoi(Setting_p->pp[1]))/255.0f;
            Config_p->Foreground.b = ((float)atoi(Setting_p->pp[2]))/255.0f;
            Config_p->Foreground.a = ((float)atoi(Setting_p->pp[3]))/255.0f;
            break;
        case 3 :
            Config_p->backgrounds = 0;
            if (Setting_p->size < 4 || (Setting_p->size % 4) != 0) {return TTYR_TERMINAL_ERROR_BAD_STATE;}
            for (int i = 0, j = 0; j < Setting_p->size; i++, j += 4) {
                Config_p->Backgrounds_p[i].r = ((float)atof(Setting_p->pp[j+0]))/255.0f;
                Config_p->Backgrounds_p[i].g = ((float)atof(Setting_p->pp[j+1]))/255.0f;
                Config_p->Backgrounds_p[i].b = ((float)atof(Setting_p->pp[j+2]))/255.0f;
                Config_p->Backgrounds_p[i].a = ((float)atof(Setting_p->pp[j+3]))/255.0f;
                Config_p->backgrounds++;
            }
            break;
        case 4 :
            Config_p->accents = 0;
            if (Setting_p->size < 4 || (Setting_p->size % 4) != 0) {return TTYR_TERMINAL_ERROR_BAD_STATE;}
            for (int i = 0, j = 0; j < Setting_p->size; i++, j += 4) {
                Config_p->Accents_p[i].r = ((float)atof(Setting_p->pp[j+0]))/255.0f;
                Config_p->Accents_p[i].g = ((float)atof(Setting_p->pp[j+1]))/255.0f;
                Config_p->Accents_p[i].b = ((float)atof(Setting_p->pp[j+2]))/255.0f;
                Config_p->Accents_p[i].a = ((float)atof(Setting_p->pp[j+3]))/255.0f;
                Config_p->accents++;
            }
            break;
        case 5 :
            if (Setting_p->size != 1) {return TTYR_TERMINAL_ERROR_BAD_STATE;}
            Config_p->style = atoi(Setting_p->pp[0]);
            break;
    }

    return TTYR_TERMINAL_SUCCESS;
}

static ttyr_terminal_Config ttyr_terminal_getStaticConfig()
{
    ttyr_terminal_Config Config;
    memset(&Config, 0, sizeof(ttyr_terminal_Config));

    static const char *options_pp[] = {
        "ttyr-terminal.font.size",
        "ttyr-terminal.blink.frequency",
        "ttyr-terminal.color.foreground",
        "ttyr-terminal.color.background",
        "ttyr-terminal.color.accent",
        "ttyr-terminal.style",
    };

    int options = sizeof(options_pp)/sizeof(options_pp[0]);
 
    ttyr_terminal_Terminal *Terminal_p = nh_core_getWorkloadArg();
    TTYR_TERMINAL_CHECK_NULL_2(Config, Terminal_p)

    for (int i = 0; i < options; ++i) {
        TTYR_TERMINAL_CHECK_2(Config, ttyr_terminal_getSetting(&Config, Terminal_p->namespace_p, i, options_pp[i]))
    }

    return Config;
}

ttyr_terminal_Config ttyr_terminal_updateConfig(
    void *Terminal_p) 
{
    ((ttyr_terminal_Terminal*)Terminal_p)->Config = ttyr_terminal_getStaticConfig(); 
    return ((ttyr_terminal_Terminal*)Terminal_p)->Config;
}
