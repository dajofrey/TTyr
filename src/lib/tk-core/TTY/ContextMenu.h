#ifndef TK_CORE_CONTEXT_MENU_H
#define TK_CORE_CONTEXT_MENU_H

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

    typedef struct tk_core_ContextMenu {
        struct tk_core_ContextMenu *Parent_p;
        nh_encoding_UTF32String Name;
        bool active;
        bool hit;
        nh_api_PixelPosition Position;
        int cCol, cRow;
        nh_core_List Items;
    } tk_core_ContextMenu;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    void tk_core_freeContextMenu(
        tk_core_ContextMenu *Menu_p
    );

    void tk_core_updateContextMenuHit(
        tk_core_ContextMenu *Menu_p, tk_core_ContextMenu *Parent_p, int x, int y, bool activate
    );

    tk_core_ContextMenu *tk_core_isContextMenuHit(
        tk_core_ContextMenu *Menu_p, tk_core_ContextMenu *Parent_p, bool recursive, int x, int y
    );

    tk_core_ContextMenu *tk_core_createMouseMenu(
        int x, int y 
    );

    TK_CORE_RESULT tk_core_handleMouseMenuPress(
        tk_core_ContextMenu *Root_p, tk_core_ContextMenu *Menu_p
    );

    TK_CORE_RESULT tk_core_drawContextMenuRecursively(
        tk_core_ContextMenu *Menu_p, tk_core_Row *Grid_p
    );

/** @} */

#endif 
