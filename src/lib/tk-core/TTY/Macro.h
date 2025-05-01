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

    typedef struct tk_core_MacroTab {
        tk_core_Topbar Topbar;
        tk_core_MicroWindow MicroWindow;
    } tk_core_MacroTab;

    typedef struct tk_core_MacroTile {
        int current;       /**<Current focused macro tab.*/
        nh_core_List MacroTabs; /**<List of macro tabs.*/
    } tk_core_MacroTile;

    typedef struct tk_core_MacroWindow {
        tk_core_Tiling Tiling;
        tk_core_ContextMenu *MouseMenu_p;
        bool refreshGrid1;
        bool refreshGrid2;
        bool refreshCursor;
        bool refreshTitlebar;
        bool close;
        tk_core_Tile *RootTile_p;
        tk_core_Tile *LastFocus_p;
        tk_core_Tile *Tile_p;
    } tk_core_MacroWindow;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    tk_core_MacroWindow *tk_core_insertAndFocusWindow(
        void *TTY_p, int index
    );
    
    TTYR_CORE_RESULT tk_core_destroyWindows(
        void *TTY_p
    );

    TTYR_CORE_RESULT tk_core_updateMacroWindow(
        tk_core_MacroWindow *Window_p
    );

    TTYR_CORE_RESULT tk_core_handleMacroWindowInput(
        tk_core_Config *Config_p, tk_core_MacroWindow *Tab_p, nh_api_WSIEvent Event 
    );

    tk_core_Tile *tk_core_createMacroTile(
        tk_core_Tile *Parent_p, nh_core_List **MicroTabs_pp, int index
    );

    void tk_core_destroyMacroTile(
        tk_core_MacroTile *Tile_p
    );

    TTYR_CORE_RESULT tk_core_drawTileRow(
        tk_core_Tile *Tile_p, tk_core_Row *Row_p, int row
    );

    void tk_core_drawEmptyGlyph(
        tk_core_Glyph *Glyph_p, bool reverse
    );

/** @} */

#endif // TTYR_CORE_TTY_MACRO_H
