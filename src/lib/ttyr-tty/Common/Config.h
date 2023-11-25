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

/** @addtogroup lib_nhtty_enums
 *  @{
 */

    typedef enum TTYR_TTY_SIDEBAR_STATE_E {
        TTYR_TTY_SIDEBAR_STATE_OFF,
        TTYR_TTY_SIDEBAR_STATE_LEFT,
        TTYR_TTY_SIDEBAR_STATE_RIGHT,
    } TTYR_TTY_SIDEBAR_STATE_E;

    typedef enum TTYR_TTY_SETTING_E {
        TTYR_TTY_SETTING_SIDEBAR_TYPE,
        TTYR_TTY_SETTING_SHELL_MAX_SCROLL,
        TTYR_TTY_SETTING_WINDOWS,
        TTYR_TTY_SETTING_TABS,
        TTYR_TTY_SETTING_MENU_PROGRAM,
        TTYR_TTY_SETTING_MENU_SPLIT,
        TTYR_TTY_SETTING_MENU_APPEND,
        TTYR_TTY_SETTING_MENU_WINDOW,
        TTYR_TTY_SETTING_MENU_TAB,
        TTYR_TTY_SETTING_MENU_CLOSE,
        TTYR_TTY_SETTING_MENU_DEBUG,
        TTYR_TTY_SETTING_TITLEBAR_ON,
    } TTYR_TTY_SETTING_E;

/** @} */

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

    typedef struct ttyr_tty_SidebarConfig {
        TTYR_TTY_SIDEBAR_STATE_E state;
    } ttyr_tty_SidebarConfig;

    typedef struct ttyr_tty_TitlebarConfig {
        NH_BOOL on;
    } ttyr_tty_TitlebarConfig;

    typedef struct ttyr_tty_ShellConfig {
        NH_BOOL maxScroll;
    } ttyr_tty_ShellConfig;

    typedef struct ttyr_tty_Config {
        NH_BYTE *name_p;
        ttyr_tty_SidebarConfig Sidebar;
        ttyr_tty_TitlebarConfig Titlebar;
        ttyr_tty_ShellConfig Shell;
        ttyr_tty_MenuConfig Menu;
        int windows;
        int tabs;
    } ttyr_tty_Config;

/** @} */

/** @addtogroup lib_nhtty_vars
 *  @{
 */

    extern const NH_BYTE *TTYR_TTY_SETTING_NAMES_PP[];
    extern size_t TTYR_TTY_SETTING_NAMES_PP_COUNT;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    ttyr_tty_Config ttyr_tty_getConfig(
    );

    const NH_BYTE *ttyr_tty_getSettingName(
        TTYR_TTY_SETTING_E setting
    );

/** @} */

#endif
