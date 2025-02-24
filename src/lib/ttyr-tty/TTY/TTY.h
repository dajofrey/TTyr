#ifndef TTYR_TTY_TTY_H
#define TTYR_TTY_TTY_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "Macro.h"
#include "Menu.h"
#include "../Common/Includes.h"
#include "../Common/Config.h"

#include "nh-core/System/System.h"

#define CTRL_KEY(k) ((k) & 0x1f)

#endif

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct ttyr_tty_Clipboard {
        nh_core_Array Lines;
    } ttyr_tty_Clipboard;

    typedef struct ttyr_tty_Borders {
        bool on;
    } ttyr_tty_Borders;

    typedef struct ttyr_tty_Topbars {
        bool on;
    } ttyr_tty_Topbars;

    typedef struct ttyr_tty_Preview {
        bool blink;
        double blinkFrequency;
        nh_core_SystemTime LastBlink;
    } ttyr_tty_Preview;

    typedef struct ttyr_tty_Titlebar {
        nh_LocalTime Time;
        nh_RAM RAM;
    } ttyr_tty_Titlebar;

    typedef struct ttyr_tty_TTY {
        ttyr_tty_Clipboard Clipboard;
        ttyr_tty_Borders Borders;
        ttyr_tty_Topbars Topbars;
        ttyr_tty_Preview Preview;
        ttyr_tty_Titlebar Titlebar;
        ttyr_tty_MacroWindow *Window_p;
        ttyr_tty_Tile *InsertTile_p;
        ttyr_tty_Menu WindowSwitchMenu;
        ttyr_tty_Menu TabSwitchMenu;
        nh_core_List Views;
        nh_core_List Prototypes;
        nh_core_List Windows;
        bool alt;
        bool alt2;
        bool ctrl;
        bool close;
        bool ignoreNext;
        nh_api_KeyboardEvent LastEvent;
        nh_core_SystemTime LastAlt;
        bool hasFocus;
        char namespace_p[255];
        nh_core_RingBuffer Events;
    } ttyr_tty_TTY;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    TTYR_TTY_RESULT ttyr_tty_resetClipboard(
    );

    nh_encoding_UTF32String *ttyr_tty_newClipboardLine(
    );

    ttyr_tty_Clipboard *ttyr_tty_getClipboard(
    );

/** @} */

#endif
