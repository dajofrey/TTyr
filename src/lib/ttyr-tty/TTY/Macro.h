#ifndef TTYR_TTY_TTY_MACRO_H
#define TTYR_TTY_TTY_MACRO_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "Tiling.h"
#include "TopBar.h"
#include "Micro.h"
#include "ContextMenu.h"

#include "../Common/Includes.h"

#include "../../../../external/Netzhaut/src/lib/nhcore/Util/String.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/Util/List.h"

#endif

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct ttyr_tty_MacroTab {
        ttyr_tty_TopBar TopBar;
        ttyr_tty_MicroWindow MicroWindow;
    } ttyr_tty_MacroTab;

    typedef struct ttyr_tty_MacroTile {
        int current;       /**<Current focused macro tab.*/
        nh_List MacroTabs; /**<List of macro tabs.*/
    } ttyr_tty_MacroTile;

    typedef struct ttyr_tty_MacroWindow {
        ttyr_tty_Tiling Tiling;
        ttyr_tty_ContextMenu *MouseMenu_p;
        NH_BOOL refreshGrid1;
        NH_BOOL refreshGrid2;
        NH_BOOL refreshCursor;
        NH_BOOL refreshTitlebar;
        NH_BOOL close;
        ttyr_tty_Tile *RootTile_p;
        ttyr_tty_Tile *LastFocus_p;
        ttyr_tty_Tile *Tile_p;
    } ttyr_tty_MacroWindow;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    ttyr_tty_MacroWindow *ttyr_tty_insertAndFocusWindow(
        void *TTY_p, int index
    );
    
    TTYR_TTY_RESULT ttyr_tty_destroyWindows(
        void *TTY_p
    );

    TTYR_TTY_RESULT ttyr_tty_updateMacroWindow(
        ttyr_tty_MacroWindow *Window_p
    );

    TTYR_TTY_RESULT ttyr_tty_handleMacroWindowInput(
        ttyr_tty_MacroWindow *Tab_p, nh_wsi_Event Event 
    );

    ttyr_tty_Tile *ttyr_tty_createMacroTile(
        ttyr_tty_Tile *Parent_p, nh_List **MicroTabs_pp, int index
    );

    void ttyr_tty_destroyMacroTile(
        ttyr_tty_MacroTile *Tile_p
    );

    TTYR_TTY_RESULT ttyr_tty_drawTileRow(
        ttyr_tty_Tile *Tile_p, ttyr_tty_Row *Row_p, int row
    );

    void ttyr_tty_drawEmptyGlyph(
        ttyr_tty_Glyph *Glyph_p, NH_BOOL reverse
    );

/** @} */

#endif // TTYR_TTY_TTY_MACRO_H
