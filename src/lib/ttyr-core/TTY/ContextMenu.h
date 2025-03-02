#ifndef TTYR_CORE_CONTEXT_MENU_H
#define TTYR_CORE_CONTEXT_MENU_H

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

    typedef struct ttyr_core_ContextMenu {
        struct ttyr_core_ContextMenu *Parent_p;
        nh_encoding_UTF32String Name;
        bool active;
        bool hit;
        nh_api_PixelPosition Position;
        int cCol, cRow;
        nh_core_List Items;
    } ttyr_core_ContextMenu;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    void ttyr_core_freeContextMenu(
        ttyr_core_ContextMenu *Menu_p
    );

    void ttyr_core_updateContextMenuHit(
        ttyr_core_ContextMenu *Menu_p, ttyr_core_ContextMenu *Parent_p, int x, int y, bool activate
    );

    ttyr_core_ContextMenu *ttyr_core_isContextMenuHit(
        ttyr_core_ContextMenu *Menu_p, ttyr_core_ContextMenu *Parent_p, bool recursive, int x, int y
    );

    ttyr_core_ContextMenu *ttyr_core_createMouseMenu(
        int x, int y 
    );

    TTYR_CORE_RESULT ttyr_core_handleMouseMenuPress(
        ttyr_core_ContextMenu *Root_p, ttyr_core_ContextMenu *Menu_p
    );

    TTYR_CORE_RESULT ttyr_core_drawContextMenuRecursively(
        ttyr_core_ContextMenu *Menu_p, ttyr_core_Row *Grid_p
    );

/** @} */

#endif 
