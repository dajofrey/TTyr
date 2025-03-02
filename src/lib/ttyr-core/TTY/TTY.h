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

    typedef struct ttyr_core_Clipboard {
        nh_core_Array Lines;
    } ttyr_core_Clipboard;

    typedef struct ttyr_core_Borders {
        bool on;
    } ttyr_core_Borders;

    typedef struct ttyr_core_Topbars {
        bool on;
    } ttyr_core_Topbars;

    typedef struct ttyr_core_Preview {
        bool blink;
        double blinkFrequency;
        nh_core_SystemTime LastBlink;
    } ttyr_core_Preview;

    typedef struct ttyr_core_Titlebar {
        nh_LocalTime Time;
        nh_RAM RAM;
    } ttyr_core_Titlebar;

    typedef struct ttyr_core_TTY {
        ttyr_core_Clipboard Clipboard;
        ttyr_core_Borders Borders;
        ttyr_core_Topbars Topbars;
        ttyr_core_Preview Preview;
        ttyr_core_Titlebar Titlebar;
        ttyr_core_MacroWindow *Window_p;
        ttyr_core_Tile *InsertTile_p;
        ttyr_core_Menu WindowSwitchMenu;
        ttyr_core_Menu TabSwitchMenu;
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
    } ttyr_core_TTY;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    TTYR_CORE_RESULT ttyr_core_resetClipboard(
    );

    nh_encoding_UTF32String *ttyr_core_newClipboardLine(
    );

    ttyr_core_Clipboard *ttyr_core_getClipboard(
    );

/** @} */

#endif
