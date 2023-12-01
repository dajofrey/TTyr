// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// DEFINE ==========================================================================================

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

// INCLUDES ========================================================================================

#include "View.h"
#include "Draw.h"
#include "TTY.h"
#include "StandardIO.h"
#include "Program.h"

#include "../Common/Macros.h"

#include "../../ttyr-terminal/Terminal/Grid.h"

#include "../../../../external/Netzhaut/src/lib/nhcore/System/Memory.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/System/Thread.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/Util/RingBuffer.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/Common/Macros.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// SIZE ============================================================================================

TTYR_TTY_RESULT ttyr_tty_getViewSize(
    ttyr_tty_View *View_p) 
{
TTYR_TTY_BEGIN()

    if (View_p->standardIO) {
        TTYR_TTY_CHECK(ttyr_tty_getStandardOutputWindowSize(&View_p->cols, &View_p->rows))
    }

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

TTYR_TTY_RESULT ttyr_tty_translateMousePosition(
    ttyr_tty_View *View_p, nh_wsi_MouseEvent Mouse, int *col_p, int *row_p)
{
TTYR_TTY_BEGIN()

    if (View_p->standardIO) {
        TTYR_TTY_CHECK(TTYR_TTY_ERROR_BAD_STATE)
    }

    int index = 0;
    for (int i = 0; i <= Mouse.Position.x; i += View_p->TileSize.width) {
        index++;
    }
    *col_p = index-1;

    index = 0;
    for (int i = 0; i <= Mouse.Position.y; i += View_p->TileSize.height) {
        index++;
    }
    *row_p = index-1;

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

TTYR_TTY_RESULT ttyr_tty_updateView(
    ttyr_tty_View *View_p, NH_BOOL *updated_p, NH_BOOL macro)
{
TTYR_TTY_BEGIN()

    if (View_p->cols == View_p->previousCols && View_p->rows == View_p->previousRows &&
        View_p->Size.width == View_p->PreviousSize.width && View_p->Size.height == View_p->PreviousSize.height) {
        TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
    }

    ttyr_tty_Config Config = ttyr_tty_getConfig();

    if (View_p->Row.Glyphs_p) {
        nh_core_free(View_p->Row.Glyphs_p);
    }
    View_p->Row.Glyphs_p = nh_core_allocate(sizeof(ttyr_tty_Glyph)*View_p->cols);

    for (int i = 0; i < (Config.Titlebar.on && macro && View_p->previousRows > 0 ? View_p->previousRows+1 : View_p->previousRows); ++i) {
        nh_core_free(View_p->Grid1_p[i].Glyphs_p);
        nh_core_free(View_p->Grid1_p[i].update_p);
        nh_core_free(View_p->Grid2_p[i].Glyphs_p);
        nh_core_free(View_p->Grid2_p[i].update_p);
    }
    if (View_p->Grid1_p) {
        nh_core_free(View_p->Grid1_p);
        nh_core_free(View_p->Grid2_p);
    }

    View_p->Grid1_p = nh_core_allocate(sizeof(ttyr_tty_Row)*View_p->rows);
    View_p->Grid2_p = nh_core_allocate(sizeof(ttyr_tty_Row)*View_p->rows);
 
    for (int i = 0; i < View_p->rows; ++i) {
        View_p->Grid1_p[i].Glyphs_p = nh_core_allocate(sizeof(ttyr_tty_Glyph)*View_p->cols);
        memset(View_p->Grid1_p[i].Glyphs_p, 0, sizeof(ttyr_tty_Glyph)*View_p->cols);
        View_p->Grid1_p[i].update_p = nh_core_allocate(sizeof(NH_BOOL)*View_p->cols);
        memset(View_p->Grid1_p[i].update_p, NH_TRUE, sizeof(NH_BOOL)*View_p->cols);

        View_p->Grid2_p[i].Glyphs_p = nh_core_allocate(sizeof(ttyr_tty_Glyph)*View_p->cols);
        memset(View_p->Grid2_p[i].Glyphs_p, 0, sizeof(ttyr_tty_Glyph)*View_p->cols);
        View_p->Grid2_p[i].update_p = nh_core_allocate(sizeof(NH_BOOL)*View_p->cols);
        memset(View_p->Grid2_p[i].update_p, NH_TRUE, sizeof(NH_BOOL)*View_p->cols);
    }

    View_p->previousCols = View_p->cols;
    View_p->previousRows = View_p->rows;
    View_p->PreviousSize = View_p->Size;

    if (Config.Titlebar.on && macro) {
        View_p->previousRows--;
        View_p->rows--;
    }

    if (updated_p) {*updated_p = NH_TRUE;}

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

// CREATE/DESTROY ==================================================================================

static void ttyr_tty_initTilesBuffer(
    nh_RingBuffer *Buffer_p, int itemCount)
{
TTYR_TTY_BEGIN()

    for (int i = 0; i < itemCount; ++i) {
        nh_Array *Array_p = nh_core_advanceRingBuffer(Buffer_p);
        *Array_p = nh_core_initArray(sizeof(ttyr_terminal_TileUpdate), 255);
    }

TTYR_TTY_SILENT_END()
}

static void ttyr_tty_initBoxesBuffer(
    nh_RingBuffer *Buffer_p, int itemCount)
{
TTYR_TTY_BEGIN()

    for (int i = 0; i < itemCount; ++i) {
        nh_Array *Array_p = nh_core_advanceRingBuffer(Buffer_p);
        *Array_p = nh_core_initArray(sizeof(ttyr_terminal_Box), 16);
    }

TTYR_TTY_SILENT_END()
}

ttyr_tty_View *ttyr_tty_createView(
    ttyr_tty_TTY *TTY_p, void *p, NH_BOOL standardIO)
{
TTYR_TTY_BEGIN()

    ttyr_tty_View View;
    memset(&View, 0, sizeof(ttyr_tty_View));

    View.standardIO = standardIO;
    View.p = p;

    NH_CORE_CHECK_2(NULL, nh_core_initRingBuffer(
        &View.Forward.Tiles, 64, sizeof(nh_Array), ttyr_tty_initTilesBuffer
    ))
    NH_CORE_CHECK_2(NULL, nh_core_initRingBuffer(
        &View.Forward.Events, 64, sizeof(nh_wsi_Event), NULL 
    ))
    NH_CORE_CHECK_2(NULL, nh_core_initRingBuffer(
        &View.Forward.Boxes, 16, sizeof(nh_Array), ttyr_tty_initBoxesBuffer 
    ))

    ttyr_tty_View *View_p = nh_core_allocate(sizeof(ttyr_tty_View));
    TTYR_TTY_CHECK_MEM_2(NULL, View_p)

    *View_p = View;

    if (TTY_p) {
        nh_core_appendToList(&TTY_p->Views, View_p);
    }

TTYR_TTY_END(View_p)
}

TTYR_TTY_RESULT ttyr_tty_destroyView(
    ttyr_tty_TTY *TTY_p, ttyr_tty_View *View_p)
{
TTYR_TTY_BEGIN()

    for (int i = 0; i < 64; ++i) {
        nh_Array *Array_p = nh_core_advanceRingBuffer(&View_p->Forward.Tiles);
        nh_core_freeArray(Array_p);
    }
    for (int i = 0; i < 16; ++i) {
        nh_Array *Array_p = nh_core_advanceRingBuffer(&View_p->Forward.Boxes);
        nh_core_freeArray(Array_p);
    }

    nh_core_freeRingBuffer(&View_p->Forward.Tiles);
    nh_core_freeRingBuffer(&View_p->Forward.Events);
    nh_core_freeRingBuffer(&View_p->Forward.Boxes);
 
    nh_core_free(View_p->Row.Glyphs_p);
    for (int i = 0; i < View_p->rows; ++i) {
        nh_core_free(View_p->Grid1_p[i].Glyphs_p);
        nh_core_free(View_p->Grid1_p[i].update_p);
        nh_core_free(View_p->Grid2_p[i].Glyphs_p);
        nh_core_free(View_p->Grid2_p[i].update_p);
    }

    nh_core_free(View_p->Grid1_p);
    nh_core_free(View_p->Grid2_p);

    if (TTY_p) {
        nh_core_removeFromList2(&TTY_p->Views, NH_TRUE, View_p);
    } else {
        nh_core_free(View_p); 
    }

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

// FORWARD FUNCTIONS ===============================================================================
// These functions forward specific TTY internal data to the view for rendering.

TTYR_TTY_RESULT ttyr_tty_forwardCursor(
    ttyr_tty_View *View_p, int x, int y)
{
TTYR_TTY_BEGIN()

    if (View_p->standardIO) {
        TTYR_TTY_END(ttyr_tty_writeCursorToStandardOutput(x, y))
    }

    ttyr_tty_Config Config = ttyr_tty_getConfig();

    // Prepare cursor glyph.
    ttyr_tty_Glyph Glyph;
    memset(&Glyph, 0, sizeof(ttyr_tty_Glyph));

    // Write to nhterminal.
    nh_Array *Array_p = nh_core_advanceRingBuffer(&View_p->Forward.Tiles);
    nh_core_freeArray(Array_p);

    ttyr_terminal_TileUpdate Update;
    Update.row = Config.Titlebar.on ? y : y - 1;
    Update.col = x - 1;
    Update.Glyph = Glyph;
    Update.cursor = NH_TRUE;

    nh_core_appendToArray(Array_p, &Update, 1);

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

static TTYR_TTY_RESULT ttyr_tty_setContextMenus(
    ttyr_tty_ContextMenu *Menu_p, nh_Array *Boxes_p)
{
TTYR_TTY_BEGIN()

    if (!Menu_p->active || Menu_p->Items.size == 0) {TTYR_TTY_END(TTYR_TTY_SUCCESS)}

    ttyr_terminal_Box *Box_p = nh_core_incrementArray(Boxes_p);

    int width = 0;
    for (int i = 0; i < Menu_p->Items.size; ++i) {
        nh_encoding_UTF32String *String_p = &((ttyr_tty_ContextMenu*)Menu_p->Items.pp[i])->Name;
        if (String_p->length > width) {width = String_p->length;}
    }

    memset(Box_p, 0, sizeof(ttyr_terminal_Box));
    Box_p->UpperLeft.x = Menu_p->Position.x-1;
    Box_p->UpperLeft.y = Menu_p->Position.y;
    Box_p->LowerRight.x = Menu_p->Position.x+width+1;
    Box_p->LowerRight.y = Menu_p->Position.y+Menu_p->Items.size;

    for (int i = 0; i < Menu_p->Items.size; ++i) {
        ttyr_tty_setContextMenus(Menu_p->Items.pp[i], Boxes_p);
    }

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

TTYR_TTY_RESULT ttyr_tty_forwardGrid1(
    ttyr_tty_View *View_p)
{
TTYR_TTY_BEGIN()

    if (View_p->standardIO) {
        TTYR_TTY_END(ttyr_tty_writeToStandardOutput(View_p->Grid1_p, View_p->cols, View_p->rows))
    }

    ttyr_tty_Config Config = ttyr_tty_getConfig();

    // Write to nhterminal.
    nh_Array *Boxes_p = nh_core_advanceRingBuffer(&View_p->Forward.Boxes);
    nh_Array *Array_p = nh_core_advanceRingBuffer(&View_p->Forward.Tiles);
    nh_core_freeArray(Array_p);
    nh_core_freeArray(Boxes_p);

    if (Config.Titlebar.on) {
        for (int col = 0; col < View_p->cols; ++col) {
            if (View_p->Grid1_p[View_p->rows].update_p[col] == 0) {continue;}
            ttyr_terminal_TileUpdate Update;
            Update.row = 0;
            Update.col = col;
            Update.Glyph = View_p->Grid1_p[View_p->rows].Glyphs_p[col];
            Update.cursor = NH_FALSE;
            nh_core_appendToArray(Array_p, &Update, 1);
            View_p->Grid1_p[View_p->rows].update_p[col] = NH_FALSE;
        }
    }

    for (int row = 0; row < View_p->rows; ++row) {
        for (int col = 0; col < View_p->cols; ++col) {
            if (!(View_p->Grid1_p[row].update_p[col])) {continue;}

            ttyr_terminal_TileUpdate Update;
            Update.row = Config.Titlebar.on ? row+1 : row;
            Update.col = col;
            Update.Glyph = View_p->Grid1_p[row].Glyphs_p[col];
            Update.cursor = NH_FALSE;
            nh_core_appendToArray(Array_p, &Update, 1);

            View_p->Grid1_p[row].update_p[col] = NH_FALSE;
        }
    }

    // Forward boxes: Context Menu.
    ttyr_tty_TTY *TTY_p = nh_core_getWorkloadArg();
    if (TTY_p->Window_p->MouseMenu_p) {
        ttyr_tty_setContextMenus(TTY_p->Window_p->MouseMenu_p, Boxes_p);
    }

    // Forward boxes: Inactive cursor outlines.
    nh_List MacroTiles = ttyr_tty_getTiles(TTY_p->Window_p->RootTile_p);
    for (int i = 0; i < MacroTiles.size; ++i) {

        // Skip macro tile without payload.
        if (((ttyr_tty_Tile*)MacroTiles.pp[i])->p == NULL 
        || TTYR_TTY_MACRO_TILE(MacroTiles.pp[i])->MacroTabs.size <= 0 
        || TTYR_TTY_MACRO_TAB(MacroTiles.pp[i])->MicroWindow.Tabs_p == NULL) {continue;}

        nh_List MicroTiles = ttyr_tty_getTiles(TTYR_TTY_MICRO_TAB(TTYR_TTY_MACRO_TAB(MacroTiles.pp[i]))->RootTile_p);
        for (int j = 0; j < MicroTiles.size; ++j) {

            // Skip micro tile without payload.
            if (((ttyr_tty_Tile*)MicroTiles.pp[j])->p == NULL) {continue;}

            ttyr_tty_Program *Program_p = TTYR_TTY_MICRO_TILE(MicroTiles.pp[j])->Program_p;
            ttyr_tty_Program *CurrentProgram_p = ttyr_tty_getCurrentProgram(&TTYR_TTY_MACRO_TAB(TTY_p->Window_p->Tile_p)->MicroWindow);
            if (CurrentProgram_p == Program_p && TTY_p->hasFocus) {continue;}

            int x = 0, y = 0;
            TTYR_TTY_CHECK(ttyr_tty_getCursorPosition(MacroTiles.pp[i], MicroTiles.pp[j], View_p->standardIO, &x, &y))

            ttyr_terminal_Box *Box_p = nh_core_incrementArray(Boxes_p);
            memset(Box_p, 0, sizeof(ttyr_terminal_Box));
            Box_p->UpperLeft.x = x-2;
            Box_p->UpperLeft.y = Config.Titlebar.on ? y : y-1;
            Box_p->LowerRight.x = x-2;
            Box_p->LowerRight.y = Config.Titlebar.on ? y : y-1;
        }
        nh_core_freeList(&MicroTiles, NH_FALSE);
    }
    nh_core_freeList(&MacroTiles, NH_FALSE);

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

TTYR_TTY_RESULT ttyr_tty_forwardGrid2(
    ttyr_tty_View *View_p)
{
TTYR_TTY_BEGIN()

    // Write to nhterminal.
    nh_Array *Array_p = nh_core_advanceRingBuffer(&View_p->Forward.Tiles);
    nh_core_freeArray(Array_p);

    for (int row = 0; row < View_p->rows; ++row) {
        for (int col = 0; col < View_p->cols; ++col) {
            ttyr_terminal_TileUpdate Update;
            Update.row = row;
            Update.col = col;
            Update.Glyph = View_p->Grid2_p[row].Glyphs_p[col];
            Update.Glyph.mark |= TTYR_TTY_MARK_ELEVATED | TTYR_TTY_MARK_ACCENT;
            Update.cursor = NH_FALSE;
            nh_core_appendToArray(Array_p, &Update, 1);
        }
    }

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

TTYR_TTY_RESULT ttyr_tty_forwardEvent(
    ttyr_tty_View *View_p, nh_wsi_Event Event)
{
TTYR_TTY_BEGIN()

    if (View_p->standardIO) {
        TTYR_TTY_END(TTYR_TTY_SUCCESS)
    }

    // Write to nhterminal.
    nh_wsi_Event *Event_p = nh_core_advanceRingBuffer(&View_p->Forward.Events);
    TTYR_TTY_CHECK_MEM(Event_p)
    *Event_p = Event;

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

