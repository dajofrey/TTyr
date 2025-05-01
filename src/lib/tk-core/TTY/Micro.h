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

    typedef struct tk_core_MicroTile {
        tk_core_Program *Program_p;
    } tk_core_MicroTile;

    typedef struct tk_core_MicroTab {
        tk_core_Interface *Prototype_p; /**<Type of programs spawned by this micro tab.*/
        tk_core_Tile *RootTile_p;       /**<Root tile.*/
        tk_core_Tile *Tile_p;           /**<Current focused tile.*/
        tk_core_Tile *LastFocus_p;
    } tk_core_MicroTab;

    typedef struct tk_core_MicroWindow {
        int current;     /**<Current focused micro tab.*/
        nh_core_List *Tabs_p; /**<Pointer to micro tabs.*/
        tk_core_View *View_p;
    } tk_core_MicroWindow;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    tk_core_Tile *tk_core_createMicroTile(
        tk_core_Tile *Parent_p, tk_core_Program *Program_p, int index
    );

    void tk_core_destroyMicroTile(
        tk_core_MicroTile *Tile_p
    );

    nh_core_List *tk_core_createMicroTabs(
        tk_core_TTY *TTY_p
    );
    
    TTYR_CORE_RESULT tk_core_appendMicroTab(
        tk_core_MicroWindow *Window_p, tk_core_Interface *Prototype_p, bool once
    );

    TTYR_CORE_RESULT tk_core_getMicroTiles(
        tk_core_MicroWindow *Window_p, nh_core_List *List_p
    );

    tk_core_MicroWindow tk_core_initMicroWindow(
        nh_core_List *MicroTabs_p
    );

    TTYR_CORE_RESULT tk_core_destroyMicroWindow(
        tk_core_MicroWindow *Window_p
    );

    TTYR_CORE_RESULT tk_core_drawMicroWindow(
        tk_core_Config *Config_p, tk_core_MicroWindow *Window_p, tk_core_Glyph *Glyphs_p, int cols, int rows, int row, bool standardIO
    );

    tk_core_Program *tk_core_getCurrentProgram(
        tk_core_MicroWindow *Window_p
    );

/** @} */

#endif // TTYR_CORE_MICRO_H
