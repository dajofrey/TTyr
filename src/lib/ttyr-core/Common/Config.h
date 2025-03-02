#ifndef TTYR_CORE_CONFIG_H
#define TTYR_CORE_CONFIG_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "Includes.h"

#include <stddef.h>

#endif

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct ttyr_core_MenuConfig {
        bool program;
        bool split;
        bool append;
        bool window;
        bool tab;
        bool close;
        bool debug;
    } ttyr_core_MenuConfig;

    typedef struct ttyr_core_TitlebarConfig {
        bool on;
        ttyr_core_Color Foreground;
        ttyr_core_Color Background;
    } ttyr_core_TitlebarConfig;

    typedef struct ttyr_core_TopbarConfig {
        bool on;
    } ttyr_core_TopbarConfig;

    typedef struct ttyr_core_ShellConfig {
        unsigned int maxScroll;
    } ttyr_core_ShellConfig;

    typedef struct ttyr_core_Config {
        char *name_p;
        ttyr_core_TitlebarConfig Titlebar;
        ttyr_core_TopbarConfig Topbar;
        ttyr_core_ShellConfig Shell;
        ttyr_core_MenuConfig Menu;
        int windows;
        int tabs;
    } ttyr_core_Config;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    ttyr_core_Config ttyr_core_getConfig(
    );

    const char *ttyr_core_getSettingName(
        unsigned int setting
    );

/** @} */

#endif
