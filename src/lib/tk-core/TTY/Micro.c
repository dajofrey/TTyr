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

tk_core_Tile *tk_core_createMicroTile(
    tk_core_Tile *Parent_p, tk_core_Program *Program_p, int index)
{
    tk_core_MicroTile *Tile_p = (tk_core_MicroTile*)nh_core_allocate(sizeof(tk_core_MicroTile));
    TK_CHECK_MEM_2(NULL, Tile_p)

    Tile_p->Program_p = Program_p == NULL || Parent_p == NULL ? 
        Program_p : TK_CORE_MICRO_TILE(Parent_p)->Program_p;
 
    return tk_core_createTile(Tile_p, TK_CORE_TILE_TYPE_MICRO, Parent_p, index);
}

void tk_core_destroyMicroTile(
    tk_core_MicroTile *Tile_p)
{
    if (Tile_p->Program_p) {
        tk_core_destroyProgramInstance(Tile_p->Program_p);
    }
    nh_core_free(Tile_p);
}

TK_CORE_RESULT tk_core_getMicroTiles(
    tk_core_MicroWindow *Window_p, nh_core_List *List_p)
{
    *List_p = nh_core_initList(4);

    for (int i = 0; Window_p->Tabs_p && i < Window_p->Tabs_p->size; ++i) {
        nh_core_List List = tk_core_getTiles(((tk_core_MicroTab*)Window_p->Tabs_p->pp[i])->RootTile_p);
        nh_core_appendItemsToList(List_p, &List);
        nh_core_freeList(&List, false);
    }

    return TK_CORE_SUCCESS;
}

// MICRO TABS ======================================================================================

static tk_core_MicroTab *tk_core_createMicroTab(
    tk_core_Interface *Prototype_p, bool once)
{
    tk_core_MicroTab *Tab_p = (tk_core_MicroTab*)nh_core_allocate(sizeof(tk_core_MicroTab));
    TK_CHECK_MEM_2(NULL, Tab_p)

    tk_core_Tile *Tile_p = tk_core_createMicroTile(NULL, tk_core_createProgramInstance(Prototype_p, once), 0);
    TK_CHECK_MEM_2(NULL, Tile_p)

    Tab_p->RootTile_p  = Tile_p;
    Tab_p->LastFocus_p = Tile_p;
    Tab_p->Tile_p      = Tile_p;
    Tab_p->Prototype_p = Prototype_p;

    return Tab_p;
}

nh_core_List *tk_core_createMicroTabs(
    tk_core_TTY *TTY_p)
{
    nh_core_List *Tabs_p = (nh_core_List*)nh_core_allocate(sizeof(nh_core_List));
    TK_CHECK_MEM_2(NULL, Tabs_p)

    *Tabs_p = nh_core_initList(8); // Don't change size to TTY_p->Prototypes.size, it might not be initialized.

    for (int i = 0; i < TTY_p->Prototypes.size; ++i) {
        tk_core_Interface *Prototype_p = TTY_p->Prototypes.pp[i];
        tk_core_MicroTab *Tab_p = tk_core_createMicroTab(Prototype_p, false);
        TK_CHECK_NULL_2(NULL, Tab_p)
        nh_core_appendToList(Tabs_p, Tab_p);
    }

    return Tabs_p;
}

static void tk_core_freeMicroTab(
    tk_core_MicroTab *Tab_p)
{
    tk_core_closeTile(Tab_p->RootTile_p, Tab_p);
    nh_core_free(Tab_p);
}

TK_CORE_RESULT tk_core_appendMicroTab(
    tk_core_MicroWindow *Window_p, tk_core_Interface *Prototype_p, bool once)
{
    tk_core_MicroTab *Tab_p = tk_core_createMicroTab(Prototype_p, once);
    TK_CHECK_NULL(Tab_p)

    nh_core_appendToList(Window_p->Tabs_p, Tab_p);
    return TK_CORE_SUCCESS;
}

// MICRO WINDOW ====================================================================================

tk_core_MicroWindow tk_core_initMicroWindow(
    nh_core_List *MicroTabs_p)
{
    tk_core_MicroWindow Window;

    Window.Tabs_p = MicroTabs_p;
    Window.current = 0;
    Window.View_p = tk_core_createView(NULL, NULL, false);
    return Window;
}

TK_CORE_RESULT tk_core_destroyMicroWindow(
    tk_core_MicroWindow *Window_p)
{
    if (Window_p->Tabs_p) {
        for (int i = 0; i < Window_p->Tabs_p->size; ++i) {
            tk_core_freeMicroTab(Window_p->Tabs_p->pp[i]);
        }
        nh_core_freeList(Window_p->Tabs_p, false);
        nh_core_free(Window_p->Tabs_p);
        Window_p->Tabs_p = NULL;
    }

    TK_CHECK(tk_core_destroyView(NULL, Window_p->View_p))
    return TK_CORE_SUCCESS;
}

TK_CORE_RESULT tk_core_drawMicroWindow(
    tk_core_Config *Config_p, tk_core_MicroWindow *Window_p, tk_core_Glyph *Glyphs_p, int cols,
    int rows, int row, bool standardIO)
{
    if (!Window_p->Tabs_p || Window_p->Tabs_p->size == 0) {return TK_CORE_SUCCESS;}

    Window_p->View_p->cols = cols;
    Window_p->View_p->rows = rows;
    Window_p->View_p->standardIO = standardIO;

    tk_core_Tile *RootTile_p = ((tk_core_MicroTab*)Window_p->Tabs_p->pp[Window_p->current])->RootTile_p;
    tk_core_updateTiling(RootTile_p, Window_p->View_p->rows, Window_p->View_p->cols);
    nh_core_List Tiles = tk_core_getTiles(RootTile_p);

    TK_CHECK(tk_core_updateView(NULL, Window_p->View_p, NULL, false))
    TK_CHECK(tk_core_refreshGrid1Row(Config_p, &Tiles, Window_p->View_p, row))

    memcpy(Glyphs_p, Window_p->View_p->Grid1_p[row].Glyphs_p, sizeof(tk_core_Glyph)*cols);
    nh_core_freeList(&Tiles, false);
 
    return TK_CORE_SUCCESS;
}

// CURRENT PROGRAM =================================================================================

tk_core_Program *tk_core_getCurrentProgram(
    tk_core_MicroWindow *Window_p)
{
    tk_core_MicroTab *Tab_p = !Window_p->Tabs_p || Window_p->Tabs_p->size <= Window_p->current 
        ? NULL : Window_p->Tabs_p->pp[Window_p->current];

    return !Tab_p ? NULL : TK_CORE_MICRO_TILE(Tab_p->Tile_p)->Program_p;
}
