#ifndef TTYR_CORE_MICRO_H
#define TTYR_CORE_MICRO_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "Program.h"
#include "Tiling.h"
#include "View.h"

#include "../Common/Includes.h"

#endif

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct ttyr_core_MicroTile {
        ttyr_core_Program *Program_p;
    } ttyr_core_MicroTile;

    typedef struct ttyr_core_MicroTab {
        ttyr_core_Interface *Prototype_p; /**<Type of programs spawned by this micro tab.*/
        ttyr_core_Tile *RootTile_p;       /**<Root tile.*/
        ttyr_core_Tile *Tile_p;           /**<Current focused tile.*/
        ttyr_core_Tile *LastFocus_p;
    } ttyr_core_MicroTab;

    typedef struct ttyr_core_MicroWindow {
        int current;     /**<Current focused micro tab.*/
        nh_core_List *Tabs_p; /**<Pointer to micro tabs.*/
        ttyr_core_View *View_p;
    } ttyr_core_MicroWindow;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    ttyr_core_Tile *ttyr_core_createMicroTile(
        ttyr_core_Tile *Parent_p, ttyr_core_Program *Program_p, int index
    );

    void ttyr_core_destroyMicroTile(
        ttyr_core_MicroTile *Tile_p
    );

    nh_core_List *ttyr_core_createMicroTabs(
        ttyr_core_TTY *TTY_p
    );
    
    TTYR_CORE_RESULT ttyr_core_appendMicroTab(
        ttyr_core_MicroWindow *Window_p, ttyr_core_Interface *Prototype_p, bool once
    );

    TTYR_CORE_RESULT ttyr_core_getMicroTiles(
        ttyr_core_MicroWindow *Window_p, nh_core_List *List_p
    );

    ttyr_core_MicroWindow ttyr_core_initMicroWindow(
        nh_core_List *MicroTabs_p
    );

    TTYR_CORE_RESULT ttyr_core_destroyMicroWindow(
        ttyr_core_MicroWindow *Window_p
    );

    TTYR_CORE_RESULT ttyr_core_drawMicroWindow(
        ttyr_core_MicroWindow *Window_p, ttyr_core_Glyph *Glyphs_p, int cols, int rows, int row, bool standardIO
    );

    ttyr_core_Program *ttyr_core_getCurrentProgram(
        ttyr_core_MicroWindow *Window_p
    );

/** @} */

#endif // TTYR_CORE_MICRO_H
