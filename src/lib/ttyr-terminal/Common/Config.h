#ifndef TTYR_TERMINAL_CONFIG_H
#define TTYR_TERMINAL_CONFIG_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "Includes.h"

#include <stddef.h>

#endif

/** @addtogroup lib_nhterminal_enums
 *  @{
 */

    typedef enum TTYR_TERMINAL_SETTING_E {
        TTYR_TERMINAL_SETTING_FONT_SIZE,
        TTYR_TERMINAL_SETTING_BLINK_FREQUENCY,
        TTYR_TERMINAL_SETTING_FOREGROUND,
        TTYR_TERMINAL_SETTING_BACKGROUND,
        TTYR_TERMINAL_SETTING_ACCENT,
        TTYR_TERMINAL_SETTING_BORDER,
    } TTYR_TERMINAL_SETTING_E;

/** @} */

/** @addtogroup lib_nhterminal_structs
 *  @{
 */

    typedef struct ttyr_terminal_Config {
        int fontSize;
        float blinkFrequency;
        ttyr_core_Color Foreground;
        ttyr_core_Color Background;
        ttyr_core_Color Accents_p[8];
        int accents;
        int border;
    } ttyr_terminal_Config;

/** @} */

/** @addtogroup lib_nhterminal_vars
 *  @{
 */

    extern const char *TTYR_TERMINAL_SETTING_NAMES_PP[];
    extern size_t TTYR_TERMINAL_SETTING_NAMES_PP_COUNT;

/** @} */

/** @addtogroup lib_nhterminal_functions
 *  @{
 */

    const char *ttyr_terminal_getSettingName(
        TTYR_TERMINAL_SETTING_E setting
    );

    ttyr_terminal_Config ttyr_terminal_getConfig(
    );

/** @} */

#endif
