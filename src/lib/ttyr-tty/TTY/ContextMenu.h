#ifndef TTYR_TTY_CONTEXT_MENU_H
#define TTYR_TTY_CONTEXT_MENU_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "View.h"

#include "nh-core/Util/List.h"
#include "nh-encoding/Encodings/UTF32.h"

#include "../Common/Includes.h"

#endif

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct ttyr_tty_ContextMenu {
        struct ttyr_tty_ContextMenu *Parent_p;
        nh_encoding_UTF32String Name;
        bool active;
        bool hit;
        nh_api_PixelPosition Position;
        int cCol, cRow;
        nh_core_List Items;
    } ttyr_tty_ContextMenu;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    void ttyr_tty_freeContextMenu(
        ttyr_tty_ContextMenu *Menu_p
    );

    void ttyr_tty_updateContextMenuHit(
        ttyr_tty_ContextMenu *Menu_p, ttyr_tty_ContextMenu *Parent_p, int x, int y, bool activate
    );

    ttyr_tty_ContextMenu *ttyr_tty_isContextMenuHit(
        ttyr_tty_ContextMenu *Menu_p, ttyr_tty_ContextMenu *Parent_p, bool recursive, int x, int y
    );

    ttyr_tty_ContextMenu *ttyr_tty_createMouseMenu(
        int x, int y 
    );

    TTYR_TTY_RESULT ttyr_tty_handleMouseMenuPress(
        ttyr_tty_ContextMenu *Root_p, ttyr_tty_ContextMenu *Menu_p
    );

    TTYR_TTY_RESULT ttyr_tty_drawContextMenuRecursively(
        ttyr_tty_ContextMenu *Menu_p, ttyr_tty_Row *Grid_p
    );

/** @} */

#endif 
