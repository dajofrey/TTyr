#ifndef TTYR_CORE_TTY_MACRO_H
#define TTYR_CORE_TTY_MACRO_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "Tiling.h"
#include "Topbar.h"
#include "Micro.h"
#include "ContextMenu.h"
#include "Menu.h"

#include "../Common/Includes.h"

#include "nh-core/Util/String.h"
#include "nh-core/Util/List.h"

#endif

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct ttyr_core_MacroTab {
        ttyr_core_Topbar Topbar;
        ttyr_core_MicroWindow MicroWindow;
    } ttyr_core_MacroTab;

    typedef struct ttyr_core_MacroTile {
        int current;       /**<Current focused macro tab.*/
        nh_core_List MacroTabs; /**<List of macro tabs.*/
    } ttyr_core_MacroTile;

    typedef struct ttyr_core_MacroWindow {
        ttyr_core_Tiling Tiling;
        ttyr_core_ContextMenu *MouseMenu_p;
        bool refreshGrid1;
        bool refreshGrid2;
        bool refreshCursor;
        bool refreshTitlebar;
        bool close;
        ttyr_core_Tile *RootTile_p;
        ttyr_core_Tile *LastFocus_p;
        ttyr_core_Tile *Tile_p;
    } ttyr_core_MacroWindow;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    ttyr_core_MacroWindow *ttyr_core_insertAndFocusWindow(
        void *TTY_p, int index
    );
    
    TTYR_CORE_RESULT ttyr_core_destroyWindows(
        void *TTY_p
    );

    TTYR_CORE_RESULT ttyr_core_updateMacroWindow(
        ttyr_core_MacroWindow *Window_p
    );

    TTYR_CORE_RESULT ttyr_core_handleMacroWindowInput(
        ttyr_core_MacroWindow *Tab_p, nh_api_WSIEvent Event 
    );

    ttyr_core_Tile *ttyr_core_createMacroTile(
        ttyr_core_Tile *Parent_p, nh_core_List **MicroTabs_pp, int index
    );

    void ttyr_core_destroyMacroTile(
        ttyr_core_MacroTile *Tile_p
    );

    TTYR_CORE_RESULT ttyr_core_drawTileRow(
        ttyr_core_Tile *Tile_p, ttyr_core_Row *Row_p, int row
    );

    void ttyr_core_drawEmptyGlyph(
        ttyr_core_Glyph *Glyph_p, bool reverse
    );

/** @} */

#endif // TTYR_CORE_TTY_MACRO_H
