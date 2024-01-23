#ifndef TTYR_TTY_TTY_H
#define TTYR_TTY_TTY_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "Macro.h"
#include "../Common/Includes.h"
#include "../Common/Config.h"

#include "../../../../external/Netzhaut/src/lib/nhcore/System/System.h"

#define CTRL_KEY(k) ((k) & 0x1f)

#endif

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct ttyr_tty_Clipboard {
        nh_Array Lines;
    } ttyr_tty_Clipboard;

    typedef struct ttyr_tty_Borders {
        NH_BOOL on;
    } ttyr_tty_Borders;

    typedef struct ttyr_tty_Topbars {
        NH_BOOL on;
    } ttyr_tty_Topbars;

    typedef struct ttyr_tty_Preview {
        NH_BOOL blink;
        double blinkFrequency;
        nh_SystemTime LastBlink;
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
        nh_List Views;
        nh_List Prototypes;
        nh_List Windows;
        NH_BOOL alt;
        NH_BOOL ctrl;
        NH_BOOL close;
        nh_wsi_KeyboardEvent LastEvent;
        NH_BOOL hasFocus;
        NH_BYTE namespace_p[255];
        nh_RingBuffer Events;
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
