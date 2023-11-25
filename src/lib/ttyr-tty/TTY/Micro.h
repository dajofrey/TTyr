#ifndef TTYR_TTY_MICRO_H
#define TTYR_TTY_MICRO_H

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

    typedef struct ttyr_tty_MicroTile {
        ttyr_tty_Program *Program_p;
    } ttyr_tty_MicroTile;

    typedef struct ttyr_tty_MicroTab {
        ttyr_tty_ProgramPrototype *Prototype_p; /**<Type of programs spawned by this micro tab.*/
        ttyr_tty_Tile *RootTile_p;              /**<Root tile.*/
        ttyr_tty_Tile *Tile_p;                  /**<Current focused tile.*/
        ttyr_tty_Tile *LastFocus_p;
    } ttyr_tty_MicroTab;

    typedef struct ttyr_tty_MicroWindow {
        int current;     /**<Current focused micro tab.*/
        nh_List *Tabs_p; /**<Pointer to micro tabs.*/
        ttyr_tty_View *View_p;
    } ttyr_tty_MicroWindow;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    ttyr_tty_Tile *ttyr_tty_createMicroTile(
        ttyr_tty_Tile *Parent_p, ttyr_tty_Program *Program_p, int index
    );

    void ttyr_tty_destroyMicroTile(
        ttyr_tty_MicroTile *Tile_p
    );

    nh_List *ttyr_tty_createMicroTabs(
        ttyr_tty_TTY *TTY_p
    );
    
    TTYR_TTY_RESULT ttyr_tty_appendMicroTab(
        ttyr_tty_MicroWindow *Window_p, ttyr_tty_ProgramPrototype *Prototype_p, NH_BOOL once
    );

    TTYR_TTY_RESULT ttyr_tty_getMicroTiles(
        ttyr_tty_MicroWindow *Window_p, nh_List *List_p
    );

    ttyr_tty_MicroWindow ttyr_tty_initMicroWindow(
        nh_List *MicroTabs_p
    );

    TTYR_TTY_RESULT ttyr_tty_destroyMicroWindow(
        ttyr_tty_MicroWindow *Window_p
    );

    TTYR_TTY_RESULT ttyr_tty_drawMicroWindow(
        ttyr_tty_MicroWindow *Window_p, ttyr_tty_Glyph *Glyphs_p, int cols, int rows, int row, NH_BOOL standardIO
    );

    ttyr_tty_Program *ttyr_tty_getCurrentProgram(
        ttyr_tty_MicroWindow *Window_p
    );

/** @} */

#endif // TTYR_TTY_MICRO_H
