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

#include "nh-core/System/Memory.h"
#include "nh-core/System/Thread.h"
#include "nh-core/Util/RingBuffer.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// SIZE ============================================================================================

TTYR_CORE_RESULT ttyr_core_getViewSize(
    ttyr_core_View *View_p) 
{
    if (View_p->standardIO) {
        TTYR_CHECK(ttyr_core_getStandardOutputWindowSize(&View_p->cols, &View_p->rows))
    }
    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT ttyr_core_translateMousePosition(
    ttyr_core_View *View_p, nh_api_MouseEvent Mouse, int *col_p, int *row_p)
{
    if (View_p->standardIO) {
        TTYR_CHECK(TTYR_CORE_ERROR_BAD_STATE)
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

    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT ttyr_core_updateView(
    ttyr_core_View *View_p, bool *updated_p, bool macro)
{
    if (View_p->cols == View_p->previousCols && View_p->rows == View_p->previousRows &&
        View_p->Size.width == View_p->PreviousSize.width && View_p->Size.height == View_p->PreviousSize.height) {
        if (updated_p) {*updated_p = false;}
        return TTYR_CORE_SUCCESS;
    }

    ttyr_core_Config Config = ttyr_core_getConfig();

    if (View_p->Row.Glyphs_p) {
        nh_core_free(View_p->Row.Glyphs_p);
    }
    View_p->Row.Glyphs_p = nh_core_allocate(sizeof(ttyr_core_Glyph)*View_p->cols);

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

    View_p->Grid1_p = nh_core_allocate(sizeof(ttyr_core_Row)*View_p->rows);
    View_p->Grid2_p = nh_core_allocate(sizeof(ttyr_core_Row)*View_p->rows);
 
    for (int i = 0; i < View_p->rows; ++i) {
        View_p->Grid1_p[i].Glyphs_p = nh_core_allocate(sizeof(ttyr_core_Glyph)*View_p->cols);
        memset(View_p->Grid1_p[i].Glyphs_p, 0, sizeof(ttyr_core_Glyph)*View_p->cols);
        View_p->Grid1_p[i].update_p = nh_core_allocate(sizeof(bool)*View_p->cols);
        memset(View_p->Grid1_p[i].update_p, true, sizeof(bool)*View_p->cols);

        View_p->Grid2_p[i].Glyphs_p = nh_core_allocate(sizeof(ttyr_core_Glyph)*View_p->cols);
        memset(View_p->Grid2_p[i].Glyphs_p, 0, sizeof(ttyr_core_Glyph)*View_p->cols);
        View_p->Grid2_p[i].update_p = nh_core_allocate(sizeof(bool)*View_p->cols);
        memset(View_p->Grid2_p[i].update_p, true, sizeof(bool)*View_p->cols);
    }

    View_p->previousCols = View_p->cols;
    View_p->previousRows = View_p->rows;
    View_p->PreviousSize = View_p->Size;

    if (Config.Titlebar.on && macro) {
        View_p->previousRows--;
        View_p->rows--;
    }

    if (updated_p) {*updated_p = true;}

    return TTYR_CORE_SUCCESS;
}

// CREATE/DESTROY ==================================================================================

static void ttyr_core_initTilesBuffer(
    nh_core_RingBuffer *Buffer_p, int itemCount)
{
    for (int i = 0; i < itemCount; ++i) {
        nh_core_Array *Array_p = nh_core_advanceRingBuffer(Buffer_p);
        *Array_p = nh_core_initArray(sizeof(ttyr_terminal_TileUpdate), 255);
    }
}

static void ttyr_core_initBoxesBuffer(
    nh_core_RingBuffer *Buffer_p, int itemCount)
{
    for (int i = 0; i < itemCount; ++i) {
        nh_core_Array *Array_p = nh_core_advanceRingBuffer(Buffer_p);
        *Array_p = nh_core_initArray(sizeof(ttyr_terminal_Box), 16);
    }
}

ttyr_core_View *ttyr_core_createView(
    ttyr_core_TTY *TTY_p, void *p, bool standardIO)
{
    ttyr_core_View View;
    memset(&View, 0, sizeof(ttyr_core_View));

    View.standardIO = standardIO;
    View.p = p;

    NH_CORE_CHECK_2(NULL, nh_core_initRingBuffer(
        &View.Forward.Tiles, 64, sizeof(nh_core_Array), ttyr_core_initTilesBuffer
    ))
    NH_CORE_CHECK_2(NULL, nh_core_initRingBuffer(
        &View.Forward.Events, 64, sizeof(nh_api_WSIEvent), NULL 
    ))
    NH_CORE_CHECK_2(NULL, nh_core_initRingBuffer(
        &View.Forward.Boxes, 16, sizeof(nh_core_Array), ttyr_core_initBoxesBuffer 
    ))

    ttyr_core_View *View_p = nh_core_allocate(sizeof(ttyr_core_View));
    TTYR_CHECK_MEM_2(NULL, View_p)

    *View_p = View;

    if (TTY_p) {
        nh_core_appendToList(&TTY_p->Views, View_p);
    }

    return View_p;
}

TTYR_CORE_RESULT ttyr_core_destroyView(
    ttyr_core_TTY *TTY_p, ttyr_core_View *View_p)
{
    for (int i = 0; i < 64; ++i) {
        nh_core_Array *Array_p = nh_core_advanceRingBuffer(&View_p->Forward.Tiles);
        nh_core_freeArray(Array_p);
    }
    for (int i = 0; i < 16; ++i) {
        nh_core_Array *Array_p = nh_core_advanceRingBuffer(&View_p->Forward.Boxes);
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
        nh_core_removeFromList2(&TTY_p->Views, true, View_p);
    } else {
        nh_core_free(View_p); 
    }

    return TTYR_CORE_SUCCESS;
}

// FORWARD FUNCTIONS ===============================================================================
// These functions forward specific TTY internal data to the view for rendering.

TTYR_CORE_RESULT ttyr_core_forwardCursor(
    ttyr_core_View *View_p, int x, int y)
{
    if (View_p->standardIO) {
        return ttyr_core_writeCursorToStandardOutput(x, y);
    }

    ttyr_core_Config Config = ttyr_core_getConfig();

    // Prepare cursor glyph.
    ttyr_core_Glyph Glyph;
    memset(&Glyph, 0, sizeof(ttyr_core_Glyph));

    // Write to nhterminal.
    nh_core_Array *Array_p = nh_core_advanceRingBuffer(&View_p->Forward.Tiles);
    nh_core_freeArray(Array_p);

    ttyr_terminal_TileUpdate Update;
    Update.row = Config.Titlebar.on ? y : y - 1;
    Update.col = x - 1;
    Update.Glyph = Glyph;
    Update.cursor = true;

    nh_core_appendToArray(Array_p, &Update, 1);

    return TTYR_CORE_SUCCESS;
}

static TTYR_CORE_RESULT ttyr_core_setContextMenuBoxes(
    ttyr_core_ContextMenu *Menu_p, nh_core_Array *Boxes_p)
{
    if (!Menu_p->active || Menu_p->Items.size == 0) {return TTYR_CORE_SUCCESS;}

    ttyr_terminal_Box *Box_p = nh_core_incrementArray(Boxes_p);

    int width = 0;
    for (int i = 0; i < Menu_p->Items.size; ++i) {
        nh_encoding_UTF32String *String_p = &((ttyr_core_ContextMenu*)Menu_p->Items.pp[i])->Name;
        if (String_p->length > width) {width = String_p->length;}
    }

    memset(Box_p, 0, sizeof(ttyr_terminal_Box));
    Box_p->UpperLeft.x = Menu_p->Position.x-1;
    Box_p->UpperLeft.y = Menu_p->Position.y;
    Box_p->LowerRight.x = Menu_p->Position.x+width+1;
    Box_p->LowerRight.y = Menu_p->Position.y+Menu_p->Items.size;

    for (int i = 0; i < Menu_p->Items.size; ++i) {
        ttyr_core_setContextMenuBoxes(Menu_p->Items.pp[i], Boxes_p);
    }

    return TTYR_CORE_SUCCESS;
}

static TTYR_CORE_RESULT ttyr_core_setBox(
    ttyr_core_Menu Menu, nh_core_Array *Boxes_p)
{
    ttyr_terminal_Box *Box_p = nh_core_incrementArray(Boxes_p);

    memset(Box_p, 0, sizeof(ttyr_terminal_Box));
    Box_p->UpperLeft.x = Menu.Position.x-1;
    Box_p->UpperLeft.y = Menu.Position.y;
    Box_p->LowerRight.x = Menu.Position.x+Menu.width+1;
    Box_p->LowerRight.y = Menu.Position.y+Menu.height;

    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT ttyr_core_forwardGrid1(
    ttyr_core_View *View_p)
{
    if (View_p->standardIO) {
        return ttyr_core_writeToStandardOutput(View_p->Grid1_p, View_p->cols, View_p->rows);
    }

    ttyr_core_Config Config = ttyr_core_getConfig();

    // Write to nhterminal.
    nh_core_Array *Boxes_p = nh_core_advanceRingBuffer(&View_p->Forward.Boxes);
    nh_core_Array *Array_p = nh_core_advanceRingBuffer(&View_p->Forward.Tiles);
    nh_core_freeArray(Array_p);
    nh_core_freeArray(Boxes_p);

    if (Config.Titlebar.on) {
        for (int col = 0; col < View_p->cols; ++col) {
            if (View_p->Grid1_p[View_p->rows].update_p[col] == 0) {continue;}
            ttyr_terminal_TileUpdate Update;
            Update.row = 0;
            Update.col = col;
            Update.Glyph = View_p->Grid1_p[View_p->rows].Glyphs_p[col];
            Update.cursor = false;
            nh_core_appendToArray(Array_p, &Update, 1);
            View_p->Grid1_p[View_p->rows].update_p[col] = false;
        }
    }

    for (int row = 0; row < View_p->rows; ++row) {
        for (int col = 0; col < View_p->cols; ++col) {
            if (!(View_p->Grid1_p[row].update_p[col])) {continue;}

            ttyr_terminal_TileUpdate Update;
            Update.row = Config.Titlebar.on ? row+1 : row;
            Update.col = col;
            Update.Glyph = View_p->Grid1_p[row].Glyphs_p[col];
            Update.cursor = false;
            nh_core_appendToArray(Array_p, &Update, 1);

            View_p->Grid1_p[row].update_p[col] = false;
        }
    }

    // Forward boxes: Tab menu.
    ttyr_core_TTY *TTY_p = nh_core_getWorkloadArg();
    if (TTY_p->alt && TTY_p->ctrl) {
        ttyr_core_setBox(TTY_p->TabSwitchMenu, Boxes_p);
    }

    // Forward boxes: Context Menu.
    if (TTY_p->Window_p->MouseMenu_p) {
        ttyr_core_setContextMenuBoxes(TTY_p->Window_p->MouseMenu_p, Boxes_p);
    }

    // Forward boxes: Inactive cursor outlines.
    nh_core_List MacroTiles = ttyr_core_getTiles(TTY_p->Window_p->RootTile_p);
    for (int i = 0; i < MacroTiles.size; ++i) {

        // Skip macro tile without payload.
        if (((ttyr_core_Tile*)MacroTiles.pp[i])->p == NULL 
        || TTYR_CORE_MACRO_TILE(MacroTiles.pp[i])->MacroTabs.size <= 0 
        || TTYR_CORE_MACRO_TAB(MacroTiles.pp[i])->MicroWindow.Tabs_p == NULL) {continue;}

        nh_core_List MicroTiles = ttyr_core_getTiles(TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(MacroTiles.pp[i]))->RootTile_p);
        for (int j = 0; j < MicroTiles.size; ++j) {

            // Skip micro tile without payload.
            if (((ttyr_core_Tile*)MicroTiles.pp[j])->p == NULL) {continue;}

            ttyr_core_Program *Program_p = TTYR_CORE_MICRO_TILE(MicroTiles.pp[j])->Program_p;
            ttyr_core_Program *CurrentProgram_p = ttyr_core_getCurrentProgram(&TTYR_CORE_MACRO_TAB(TTY_p->Window_p->Tile_p)->MicroWindow);
            if (CurrentProgram_p == Program_p && TTY_p->hasFocus) {continue;}

            int x = 0, y = 0;
            TTYR_CHECK(ttyr_core_getCursorPosition(MacroTiles.pp[i], MicroTiles.pp[j], View_p->standardIO, &x, &y))

            ttyr_terminal_Box *Box_p = nh_core_incrementArray(Boxes_p);
            memset(Box_p, 0, sizeof(ttyr_terminal_Box));
            Box_p->UpperLeft.x = x-2;
            Box_p->UpperLeft.y = Config.Titlebar.on ? y : y-1;
            Box_p->LowerRight.x = x-2;
            Box_p->LowerRight.y = Config.Titlebar.on ? y : y-1;
            Box_p->accent = true;
        }
        nh_core_freeList(&MicroTiles, false);
    }
    nh_core_freeList(&MacroTiles, false);

    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT ttyr_core_forwardGrid2(
    ttyr_core_View *View_p)
{
    // Write to nhterminal.
    nh_core_Array *Array_p = nh_core_advanceRingBuffer(&View_p->Forward.Tiles);
    nh_core_freeArray(Array_p);

    for (int row = 0; row < View_p->rows; ++row) {
        for (int col = 0; col < View_p->cols; ++col) {
            ttyr_terminal_TileUpdate Update;
            Update.row = row;
            Update.col = col;
            Update.Glyph = View_p->Grid2_p[row].Glyphs_p[col];
            Update.Glyph.mark |= TTYR_CORE_MARK_ELEVATED | TTYR_CORE_MARK_ACCENT;
            Update.cursor = false;
            nh_core_appendToArray(Array_p, &Update, 1);
        }
    }

    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT ttyr_core_forwardEvent(
    ttyr_core_View *View_p, nh_api_WSIEvent Event)
{
    if (View_p->standardIO) {
        return TTYR_CORE_SUCCESS;
    }

    // Write to nhterminal.
    nh_api_WSIEvent *Event_p = nh_core_advanceRingBuffer(&View_p->Forward.Events);
    TTYR_CHECK_MEM(Event_p)
    *Event_p = Event;

    return TTYR_CORE_SUCCESS;
}
