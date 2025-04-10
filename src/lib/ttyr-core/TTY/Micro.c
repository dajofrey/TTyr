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

#include "nh-core/System/Memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// MICRO TILES =====================================================================================

ttyr_core_Tile *ttyr_core_createMicroTile(
    ttyr_core_Tile *Parent_p, ttyr_core_Program *Program_p, int index)
{
    ttyr_core_MicroTile *Tile_p = nh_core_allocate(sizeof(ttyr_core_MicroTile));
    TTYR_CHECK_MEM_2(NULL, Tile_p)

    Tile_p->Program_p = Program_p == NULL || Parent_p == NULL ? 
        Program_p : TTYR_CORE_MICRO_TILE(Parent_p)->Program_p;
 
    return ttyr_core_createTile(Tile_p, TTYR_CORE_TILE_TYPE_MICRO, Parent_p, index);
}

void ttyr_core_destroyMicroTile(
    ttyr_core_MicroTile *Tile_p)
{
    if (Tile_p->Program_p) {
        ttyr_core_destroyProgramInstance(Tile_p->Program_p);
    }
    nh_core_free(Tile_p);
}

TTYR_CORE_RESULT ttyr_core_getMicroTiles(
    ttyr_core_MicroWindow *Window_p, nh_core_List *List_p)
{
    *List_p = nh_core_initList(4);

    for (int i = 0; Window_p->Tabs_p && i < Window_p->Tabs_p->size; ++i) {
        nh_core_List List = ttyr_core_getTiles(((ttyr_core_MicroTab*)Window_p->Tabs_p->pp[i])->RootTile_p);
        nh_core_appendItemsToList(List_p, &List);
        nh_core_freeList(&List, false);
    }

    return TTYR_CORE_SUCCESS;
}

// MICRO TABS ======================================================================================

static ttyr_core_MicroTab *ttyr_core_createMicroTab(
    ttyr_core_Interface *Prototype_p, bool once)
{
    ttyr_core_MicroTab *Tab_p = nh_core_allocate(sizeof(ttyr_core_MicroTab));
    TTYR_CHECK_MEM_2(NULL, Tab_p)

    ttyr_core_Tile *Tile_p = ttyr_core_createMicroTile(NULL, ttyr_core_createProgramInstance(Prototype_p, once), 0);
    TTYR_CHECK_MEM_2(NULL, Tile_p)

    Tab_p->RootTile_p  = Tile_p;
    Tab_p->LastFocus_p = Tile_p;
    Tab_p->Tile_p      = Tile_p;
    Tab_p->Prototype_p = Prototype_p;

    return Tab_p;
}

nh_core_List *ttyr_core_createMicroTabs(
    ttyr_core_TTY *TTY_p)
{
    nh_core_List *Tabs_p = nh_core_allocate(sizeof(nh_core_List));
    TTYR_CHECK_MEM_2(NULL, Tabs_p)

    *Tabs_p = nh_core_initList(8); // Don't change size to TTY_p->Prototypes.size, it might not be initialized.

    for (int i = 0; i < TTY_p->Prototypes.size; ++i) {
        ttyr_core_Interface *Prototype_p = TTY_p->Prototypes.pp[i];
        ttyr_core_MicroTab *Tab_p = ttyr_core_createMicroTab(Prototype_p, false);
        TTYR_CHECK_NULL_2(NULL, Tab_p)
        nh_core_appendToList(Tabs_p, Tab_p);
    }

    return Tabs_p;
}

static void ttyr_core_freeMicroTab(
    ttyr_core_MicroTab *Tab_p)
{
    ttyr_core_closeTile(Tab_p->RootTile_p, Tab_p);
    nh_core_free(Tab_p);
}

TTYR_CORE_RESULT ttyr_core_appendMicroTab(
    ttyr_core_MicroWindow *Window_p, ttyr_core_Interface *Prototype_p, bool once)
{
    ttyr_core_MicroTab *Tab_p = ttyr_core_createMicroTab(Prototype_p, once);
    TTYR_CHECK_NULL(Tab_p)

    nh_core_appendToList(Window_p->Tabs_p, Tab_p);
    return TTYR_CORE_SUCCESS;
}

// MICRO WINDOW ====================================================================================

ttyr_core_MicroWindow ttyr_core_initMicroWindow(
    nh_core_List *MicroTabs_p)
{
    ttyr_core_MicroWindow Window;

    Window.Tabs_p = MicroTabs_p;
    Window.current = 0;
    Window.View_p = ttyr_core_createView(NULL, NULL, false);
    return Window;
}

TTYR_CORE_RESULT ttyr_core_destroyMicroWindow(
    ttyr_core_MicroWindow *Window_p)
{
    if (Window_p->Tabs_p) {
        for (int i = 0; i < Window_p->Tabs_p->size; ++i) {
            ttyr_core_freeMicroTab(Window_p->Tabs_p->pp[i]);
        }
        nh_core_freeList(Window_p->Tabs_p, false);
        nh_core_free(Window_p->Tabs_p);
        Window_p->Tabs_p = NULL;
    }

    TTYR_CHECK(ttyr_core_destroyView(NULL, Window_p->View_p))
    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT ttyr_core_drawMicroWindow(
    ttyr_core_MicroWindow *Window_p, ttyr_core_Glyph *Glyphs_p, int cols, int rows, int row, bool standardIO)
{
    if (!Window_p->Tabs_p || Window_p->Tabs_p->size == 0) {return TTYR_CORE_SUCCESS;}

    Window_p->View_p->cols = cols;
    Window_p->View_p->rows = rows;
    Window_p->View_p->standardIO = standardIO;

    ttyr_core_Tile *RootTile_p = ((ttyr_core_MicroTab*)Window_p->Tabs_p->pp[Window_p->current])->RootTile_p;
    ttyr_core_updateTiling(RootTile_p, Window_p->View_p->rows, Window_p->View_p->cols);
    nh_core_List Tiles = ttyr_core_getTiles(RootTile_p);

    TTYR_CHECK(ttyr_core_updateView(Window_p->View_p, NULL, false))
    TTYR_CHECK(ttyr_core_refreshGrid1Row(&Tiles, Window_p->View_p, row))

    memcpy(Glyphs_p, Window_p->View_p->Grid1_p[row].Glyphs_p, sizeof(ttyr_core_Glyph)*cols);
    nh_core_freeList(&Tiles, false);
 
    return TTYR_CORE_SUCCESS;
}

// CURRENT PROGRAM =================================================================================

ttyr_core_Program *ttyr_core_getCurrentProgram(
    ttyr_core_MicroWindow *Window_p)
{
    ttyr_core_MicroTab *Tab_p = !Window_p->Tabs_p || Window_p->Tabs_p->size <= Window_p->current 
        ? NULL : Window_p->Tabs_p->pp[Window_p->current];

    return !Tab_p ? NULL : TTYR_CORE_MICRO_TILE(Tab_p->Tile_p)->Program_p;
}
