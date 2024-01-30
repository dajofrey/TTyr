// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Micro.h"
#include "Program.h"
#include "TTY.h"
#include "Draw.h"

#include "../Common/Macros.h"

#include "nhcore/System/Memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// MICRO TILES =====================================================================================

ttyr_tty_Tile *ttyr_tty_createMicroTile(
    ttyr_tty_Tile *Parent_p, ttyr_tty_Program *Program_p, int index)
{
    ttyr_tty_MicroTile *Tile_p = nh_core_allocate(sizeof(ttyr_tty_MicroTile));
    TTYR_CHECK_MEM_2(NULL, Tile_p)

    Tile_p->Program_p = Program_p == NULL || Parent_p == NULL ? 
        Program_p : TTYR_TTY_MICRO_TILE(Parent_p)->Program_p;
 
    return ttyr_tty_createTile(Tile_p, TTYR_TTY_TILE_TYPE_MICRO, Parent_p, index);
}

void ttyr_tty_destroyMicroTile(
    ttyr_tty_MicroTile *Tile_p)
{
    if (Tile_p->Program_p) {
        ttyr_tty_destroyProgramInstance(Tile_p->Program_p);
    }
    nh_core_free(Tile_p);
}

TTYR_TTY_RESULT ttyr_tty_getMicroTiles(
    ttyr_tty_MicroWindow *Window_p, nh_List *List_p)
{
    *List_p = nh_core_initList(4);

    for (int i = 0; Window_p->Tabs_p && i < Window_p->Tabs_p->size; ++i) {
        nh_List List = ttyr_tty_getTiles(((ttyr_tty_MicroTab*)Window_p->Tabs_p->pp[i])->RootTile_p);
        nh_core_appendItemsToList(List_p, &List);
        nh_core_freeList(&List, NH_FALSE);
    }

    return TTYR_TTY_SUCCESS;
}

// MICRO TABS ======================================================================================

static ttyr_tty_MicroTab *ttyr_tty_createMicroTab(
    ttyr_tty_Interface *Prototype_p, NH_BOOL once)
{
    ttyr_tty_MicroTab *Tab_p = nh_core_allocate(sizeof(ttyr_tty_MicroTab));
    TTYR_CHECK_MEM_2(NULL, Tab_p)

    ttyr_tty_Tile *Tile_p = ttyr_tty_createMicroTile(NULL, ttyr_tty_createProgramInstance(Prototype_p, once), 0);
    TTYR_CHECK_MEM_2(NULL, Tile_p)

    Tab_p->RootTile_p  = Tile_p;
    Tab_p->LastFocus_p = Tile_p;
    Tab_p->Tile_p      = Tile_p;
    Tab_p->Prototype_p = Prototype_p;

    return Tab_p;
}

nh_List *ttyr_tty_createMicroTabs(
    ttyr_tty_TTY *TTY_p)
{
    nh_List *Tabs_p = nh_core_allocate(sizeof(nh_List));
    TTYR_CHECK_MEM_2(NULL, Tabs_p)

    *Tabs_p = nh_core_initList(8); // Don't change size to TTY_p->Prototypes.size, it might not be initialized.

    for (int i = 0; i < TTY_p->Prototypes.size; ++i) {
        ttyr_tty_Interface *Prototype_p = TTY_p->Prototypes.pp[i];
        ttyr_tty_MicroTab *Tab_p = ttyr_tty_createMicroTab(Prototype_p, NH_FALSE);
        TTYR_CHECK_NULL_2(NULL, Tab_p)
        nh_core_appendToList(Tabs_p, Tab_p);
    }

    return Tabs_p;
}

static void ttyr_tty_freeMicroTab(
    ttyr_tty_MicroTab *Tab_p)
{
    ttyr_tty_closeTile(Tab_p->RootTile_p, Tab_p);
    nh_core_free(Tab_p);
}

TTYR_TTY_RESULT ttyr_tty_appendMicroTab(
    ttyr_tty_MicroWindow *Window_p, ttyr_tty_Interface *Prototype_p, NH_BOOL once)
{
    ttyr_tty_MicroTab *Tab_p = ttyr_tty_createMicroTab(Prototype_p, once);
    TTYR_CHECK_NULL(Tab_p)

    nh_core_appendToList(Window_p->Tabs_p, Tab_p);
    return TTYR_TTY_SUCCESS;
}

// MICRO WINDOW ====================================================================================

ttyr_tty_MicroWindow ttyr_tty_initMicroWindow(
    nh_List *MicroTabs_p)
{
    ttyr_tty_MicroWindow Window;

    Window.Tabs_p = MicroTabs_p;
    Window.current = 0;
    Window.View_p = ttyr_tty_createView(NULL, NULL, NH_FALSE);
    return Window;
}

TTYR_TTY_RESULT ttyr_tty_destroyMicroWindow(
    ttyr_tty_MicroWindow *Window_p)
{
    if (Window_p->Tabs_p) {
        for (int i = 0; i < Window_p->Tabs_p->size; ++i) {
            ttyr_tty_freeMicroTab(Window_p->Tabs_p->pp[i]);
        }
        nh_core_freeList(Window_p->Tabs_p, NH_FALSE);
        nh_core_free(Window_p->Tabs_p);
        Window_p->Tabs_p = NULL;
    }

    TTYR_CHECK(ttyr_tty_destroyView(NULL, Window_p->View_p))
    return TTYR_TTY_SUCCESS;
}

TTYR_TTY_RESULT ttyr_tty_drawMicroWindow(
    ttyr_tty_MicroWindow *Window_p, ttyr_tty_Glyph *Glyphs_p, int cols, int rows, int row, NH_BOOL standardIO)
{
    if (!Window_p->Tabs_p || Window_p->Tabs_p->size == 0) {return TTYR_TTY_SUCCESS;}

    Window_p->View_p->cols = cols;
    Window_p->View_p->rows = rows;
    Window_p->View_p->standardIO = standardIO;

    ttyr_tty_Tile *RootTile_p = ((ttyr_tty_MicroTab*)Window_p->Tabs_p->pp[Window_p->current])->RootTile_p;
    ttyr_tty_updateTiling(RootTile_p, Window_p->View_p->rows, Window_p->View_p->cols);
    nh_List Tiles = ttyr_tty_getTiles(RootTile_p);

    TTYR_CHECK(ttyr_tty_updateView(Window_p->View_p, NULL, NH_FALSE))
    TTYR_CHECK(ttyr_tty_refreshGrid1Row(&Tiles, Window_p->View_p, row))

    memcpy(Glyphs_p, Window_p->View_p->Grid1_p[row].Glyphs_p, sizeof(ttyr_tty_Glyph)*cols);
    nh_core_freeList(&Tiles, NH_FALSE);
 
    return TTYR_TTY_SUCCESS;
}

// CURRENT PROGRAM =================================================================================

ttyr_tty_Program *ttyr_tty_getCurrentProgram(
    ttyr_tty_MicroWindow *Window_p)
{
    ttyr_tty_MicroTab *Tab_p = !Window_p->Tabs_p || Window_p->Tabs_p->size <= Window_p->current 
        ? NULL : Window_p->Tabs_p->pp[Window_p->current];

    return !Tab_p ? NULL : TTYR_TTY_MICRO_TILE(Tab_p->Tile_p)->Program_p;
}
