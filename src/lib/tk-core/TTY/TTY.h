#ifndef TTYR_CORE_TTY_H
#define TTYR_CORE_TTY_H

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

    typedef struct tk_core_Clipboard {
        nh_core_Array Lines;
    } tk_core_Clipboard;

    typedef struct tk_core_Borders {
        bool on;
    } tk_core_Borders;

    typedef struct tk_core_Topbars {
        bool on;
    } tk_core_Topbars;

    typedef struct tk_core_Preview {
        bool blink;
        double blinkFrequency;
        nh_core_SystemTime LastBlink;
    } tk_core_Preview;

    typedef struct tk_core_Titlebar {
        nh_LocalTime Time;
        nh_RAM RAM;
    } tk_core_Titlebar;

    typedef struct tk_core_TTY {
        tk_core_Config Config;
        tk_core_Clipboard Clipboard;
        tk_core_Borders Borders;
        tk_core_Topbars Topbars;
        tk_core_Preview Preview;
        tk_core_Titlebar Titlebar;
        tk_core_MacroWindow *Window_p;
        tk_core_Tile *InsertTile_p;
        tk_core_Menu WindowSwitchMenu;
        tk_core_Menu TabSwitchMenu;
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
    } tk_core_TTY;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    TTYR_CORE_RESULT tk_core_resetClipboard(
    );

    nh_encoding_UTF32String *tk_core_newClipboardLine(
    );

    tk_core_Clipboard *tk_core_getClipboard(
    );

/** @} */

#endif
