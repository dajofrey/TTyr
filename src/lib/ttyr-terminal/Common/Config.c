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

#include "nhcore/Config/Config.h"
#include "nhcore/System/Memory.h"
#include "nhcore/System/Thread.h"

#include <string.h>
#include <stdlib.h>

// NAMES ===========================================================================================

const NH_BYTE *TTYR_TERMINAL_SETTING_NAMES_PP[] = {
    "ttyr.terminal.font.size",
    "ttyr.terminal.blink.frequency",
    "ttyr.terminal.color.foreground",
    "ttyr.terminal.color.background",
    "ttyr.terminal.color.accent",
    "ttyr.terminal.border",
};

size_t TTYR_TERMINAL_SETTING_NAMES_PP_COUNT = 
    sizeof(TTYR_TERMINAL_SETTING_NAMES_PP) / sizeof(TTYR_TERMINAL_SETTING_NAMES_PP[0]);

const NH_BYTE *ttyr_terminal_getSettingName(
    TTYR_TERMINAL_SETTING_E setting)
{
TTYR_TERMINAL_BEGIN()
TTYR_TERMINAL_END(TTYR_TERMINAL_SETTING_NAMES_PP[setting])
}

// FUNCTIONS =======================================================================================

static TTYR_TERMINAL_RESULT ttyr_terminal_getSetting(
    ttyr_terminal_Config *Config_p, NH_BYTE namespace_p[255], int setting)
{
TTYR_TERMINAL_BEGIN()

    nh_List *Setting_p = nh_core_getGlobalConfigSetting(namespace_p, -1, TTYR_TERMINAL_SETTING_NAMES_PP[setting]);
    TTYR_TERMINAL_CHECK_NULL(Setting_p)

    switch (setting) {
        case 0 :
            if (Setting_p->size != 1) {TTYR_TERMINAL_END(TTYR_TERMINAL_ERROR_BAD_STATE)}
            Config_p->fontSize = atoi(Setting_p->pp[0]);
            break;
        case 1 :
            if (Setting_p->size != 1) {TTYR_TERMINAL_END(TTYR_TERMINAL_ERROR_BAD_STATE)}
            Config_p->blinkFrequency = atof(Setting_p->pp[0]);
            break;
        case 2 :
            if (Setting_p->size != 4) {TTYR_TERMINAL_END(TTYR_TERMINAL_ERROR_BAD_STATE)}
            Config_p->Foreground.r = ((float)atoi(Setting_p->pp[0]))/255.0f;
            Config_p->Foreground.g = ((float)atoi(Setting_p->pp[1]))/255.0f;
            Config_p->Foreground.b = ((float)atoi(Setting_p->pp[2]))/255.0f;
            Config_p->Foreground.a = ((float)atoi(Setting_p->pp[3]))/255.0f;
            break;
        case 3 :
            if (Setting_p->size != 4) {TTYR_TERMINAL_END(TTYR_TERMINAL_ERROR_BAD_STATE)}
            Config_p->Background.r = ((float)atoi(Setting_p->pp[0]))/255.0f;
            Config_p->Background.g = ((float)atoi(Setting_p->pp[1]))/255.0f;
            Config_p->Background.b = ((float)atoi(Setting_p->pp[2]))/255.0f;
            Config_p->Background.a = ((float)atoi(Setting_p->pp[3]))/255.0f;
            break;
        case 4 :
            Config_p->accents = 0;
            if (Setting_p->size < 4 || (Setting_p->size % 4) != 0) {TTYR_TERMINAL_END(TTYR_TERMINAL_ERROR_BAD_STATE)}
            for (int i = 0, j = 0; j < Setting_p->size; i++, j += 4) {
                Config_p->Accents_p[i].r = ((float)atof(Setting_p->pp[j+0]))/255.0f;
                Config_p->Accents_p[i].g = ((float)atof(Setting_p->pp[j+1]))/255.0f;
                Config_p->Accents_p[i].b = ((float)atof(Setting_p->pp[j+2]))/255.0f;
                Config_p->Accents_p[i].a = ((float)atof(Setting_p->pp[j+3]))/255.0f;
                Config_p->accents++;
            }
            break;
        case 5 :
            if (Setting_p->size != 1) {TTYR_TERMINAL_END(TTYR_TERMINAL_ERROR_BAD_STATE)}
            Config_p->border = atoi(Setting_p->pp[0]);
            break;
    }

TTYR_TERMINAL_END(TTYR_TERMINAL_SUCCESS)
}

static ttyr_terminal_Config ttyr_terminal_getStaticConfig()
{
TTYR_TERMINAL_BEGIN()

    ttyr_terminal_Config Config;
    memset(&Config, 0, sizeof(ttyr_terminal_Config));

    ttyr_terminal_Terminal *Terminal_p = nh_core_getWorkloadArg();
    TTYR_TERMINAL_CHECK_NULL_2(Config, Terminal_p)

    for (int i = 0; i < TTYR_TERMINAL_SETTING_NAMES_PP_COUNT; ++i) {
        TTYR_TERMINAL_CHECK_2(Config, ttyr_terminal_getSetting(&Config, Terminal_p->namespace_p, i))
    }

TTYR_TERMINAL_END(Config)
}

ttyr_terminal_Config ttyr_terminal_getConfig() 
{ 
    ttyr_terminal_Config Config = ttyr_terminal_getStaticConfig(); 
    return Config; 
}

