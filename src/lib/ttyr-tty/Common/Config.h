#ifndef TTYR_TTY_CONFIG_H
#define TTYR_TTY_CONFIG_H

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

    typedef struct ttyr_tty_MenuConfig {
        bool program;
        bool split;
        bool append;
        bool window;
        bool tab;
        bool close;
        bool debug;
    } ttyr_tty_MenuConfig;

    typedef struct ttyr_tty_TitlebarConfig {
        bool on;
        ttyr_tty_Color Foreground;
        ttyr_tty_Color Background;
    } ttyr_tty_TitlebarConfig;

    typedef struct ttyr_tty_TopbarConfig {
        bool on;
    } ttyr_tty_TopbarConfig;

    typedef struct ttyr_tty_ShellConfig {
        bool maxScroll;
    } ttyr_tty_ShellConfig;

    typedef struct ttyr_tty_Config {
        char *name_p;
        ttyr_tty_TitlebarConfig Titlebar;
        ttyr_tty_TopbarConfig Topbar;
        ttyr_tty_ShellConfig Shell;
        ttyr_tty_MenuConfig Menu;
        int windows;
        int tabs;
    } ttyr_tty_Config;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    ttyr_tty_Config ttyr_tty_getConfig(
    );

    const char *ttyr_tty_getSettingName(
        unsigned int setting
    );

/** @} */

#endif
