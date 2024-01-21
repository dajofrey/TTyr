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
        NH_BOOL program;
        NH_BOOL split;
        NH_BOOL append;
        NH_BOOL window;
        NH_BOOL tab;
        NH_BOOL close;
        NH_BOOL debug;
    } ttyr_tty_MenuConfig;

    typedef struct ttyr_tty_TitlebarConfig {
        NH_BOOL on;
        nh_Color Foreground;
        nh_Color Background;
    } ttyr_tty_TitlebarConfig;

    typedef struct ttyr_tty_ShellConfig {
        NH_BOOL maxScroll;
    } ttyr_tty_ShellConfig;

    typedef struct ttyr_tty_Config {
        NH_BYTE *name_p;
        ttyr_tty_TitlebarConfig Titlebar;
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

    const NH_BYTE *ttyr_tty_getSettingName(
        unsigned int setting
    );

/** @} */

#endif
