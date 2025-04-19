// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Grid.h"
#include "Graphics.h"
#include "Vertices.h"

#include "../Vulkan/Render.h"
#include "../Common/Macros.h"
#include "../Common/Config.h"

#include "nh-gfx/Base/Viewport.h"

#include "nh-core/System/Thread.h"
#include "nh-core/System/Memory.h"
#include "nh-core/System/Process.h"

#include "nh-encoding/Encodings/UTF8.h"
#include "nh-encoding/Encodings/UTF32.h"

#include <string.h>
#include <stdio.h>

// INIT ============================================================================================

TTYR_TERMINAL_RESULT ttyr_terminal_initGrid(
    ttyr_terminal_Grid *Grid_p)
{
    memset(Grid_p, 0, sizeof(ttyr_terminal_Grid));

    Grid_p->Rows = nh_core_initList(128);
    Grid_p->Boxes = nh_core_initArray(sizeof(ttyr_terminal_Box), 16);

    return TTYR_TERMINAL_SUCCESS;
}

TTYR_TERMINAL_RESULT ttyr_terminal_freeGrid(
    ttyr_terminal_Grid *Grid_p)
{
    for (int row = 0; row < Grid_p->Rows.size; ++row) {
        nh_core_List *Cols_p = Grid_p->Rows.pp[row];
        for (int col = 0; col < Cols_p->size; ++col) {
            nh_core_free(Cols_p->pp[col]);
        }
        nh_core_freeList(Cols_p, false);
    }

    for (int row = 0; row < Grid_p->rows && Grid_p->Updates_pp; ++row) {
        nh_core_free(Grid_p->Updates_pp[row]);
        nh_core_free(Grid_p->updates_pp[row]);
    }

    if (Grid_p->Updates_pp) {
        nh_core_free(Grid_p->Updates_pp);
        nh_core_free(Grid_p->updates_pp);
    }

    nh_core_freeList(&Grid_p->Rows, true);
    nh_core_freeArray(&Grid_p->Boxes);

    ttyr_terminal_initGrid(Grid_p);

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_initTile(
    ttyr_terminal_Grid *Grid_p, ttyr_terminal_Tile *Tile_p, int row, int col)
{
    memset(Tile_p, 0, sizeof(ttyr_terminal_Tile));

    memset(Tile_p->Foreground.vertices_p, 0, sizeof(float) * 20);
    memset(Tile_p->Background.vertices_p, 0, sizeof(float) * 12);

    return TTYR_TERMINAL_SUCCESS;
}

// UPDATE ==========================================================================================

static void ttyr_terminal_getTileRowAndColumn(
    ttyr_terminal_Grid *Grid_p, ttyr_terminal_Tile *Tile_p, int *row_p, int *col_p)
{
    bool done = false;

    for (int row = 0; !done && row < Grid_p->Rows.size; ++row) {
        nh_core_List *Row_p = Grid_p->Rows.pp[row];
        for (int col = 0; !done && col < Row_p->size; ++col) {
            if (Row_p->pp[col] == Tile_p) {
                *row_p = row;
                *col_p = col;
                done = true;
            }
        }
    }
}

ttyr_terminal_Tile *ttyr_terminal_getTile(
    ttyr_terminal_Grid *Grid_p, int row, int col)
{
    if (Grid_p->cols <= col || Grid_p->rows <= row) {
        // If for some reason a tile is requested that is out of bounds, there is something wrong.
        return NULL;
    }

    while (Grid_p->Rows.size <= row) {
        nh_core_appendToList(&Grid_p->Rows, NULL);
    }

    if (!Grid_p->Rows.pp[row]) {
        Grid_p->Rows.pp[row] = nh_core_allocate(sizeof(nh_core_List));
        TTYR_TERMINAL_CHECK_MEM_2(NULL, Grid_p->Rows.pp[row])
        *((nh_core_List*)Grid_p->Rows.pp[row]) = nh_core_initList(128);
    }

    nh_core_List *Cols_p = Grid_p->Rows.pp[row];

     while (Cols_p->size <= col) {
        nh_core_appendToList(Cols_p, NULL);
    }

    if (!Cols_p->pp[col]) {
        Cols_p->pp[col] = nh_core_allocate(sizeof(ttyr_terminal_Tile));
        TTYR_TERMINAL_CHECK_MEM_2(NULL, Cols_p->pp[col])
        TTYR_TERMINAL_CHECK_2(NULL, ttyr_terminal_initTile(Grid_p, Cols_p->pp[col], row, col))
    }

    return Cols_p->pp[col];
}

bool ttyr_terminal_compareBackgroundAttributes(
    ttyr_core_Glyph *Glyph1_p, ttyr_core_Glyph *Glyph2_p)
{
    // Compare attributes.
    if ((Glyph1_p->mark & TTYR_CORE_MARK_ACCENT) != (Glyph2_p->mark & TTYR_CORE_MARK_ACCENT)) {
        return true;
    }

    if (Glyph1_p->Attributes.blink != Glyph2_p->Attributes.blink
    ||  Glyph1_p->Attributes.reverse != Glyph2_p->Attributes.reverse) {
        return true;
    }

    // Compare Color.
    if (Glyph1_p->Background.custom != Glyph2_p->Background.custom
    ||  Glyph1_p->Background.Color.r != Glyph2_p->Background.Color.r
    ||  Glyph1_p->Background.Color.g != Glyph2_p->Background.Color.g
    ||  Glyph1_p->Background.Color.b != Glyph2_p->Background.Color.b
    ||  Glyph1_p->Background.Color.a != Glyph2_p->Background.Color.a) {
        return true;
    }
    if (Glyph1_p->Foreground.custom != Glyph2_p->Foreground.custom
    ||  Glyph1_p->Foreground.Color.r != Glyph2_p->Foreground.Color.r
    ||  Glyph1_p->Foreground.Color.g != Glyph2_p->Foreground.Color.g
    ||  Glyph1_p->Foreground.Color.b != Glyph2_p->Foreground.Color.b
    ||  Glyph1_p->Foreground.Color.a != Glyph2_p->Foreground.Color.a) {
        return true;
    }

    return false;
}

bool ttyr_terminal_compareForegroundAttributes(
    ttyr_core_Glyph *Glyph1_p, ttyr_core_Glyph *Glyph2_p)
{
    if ((Glyph1_p->mark & TTYR_CORE_MARK_ACCENT) != (Glyph2_p->mark & TTYR_CORE_MARK_ACCENT)) {
        return true;
    }

    // Compare attributes.
    if (Glyph1_p->Attributes.bold != Glyph2_p->Attributes.bold
    ||  Glyph1_p->Attributes.faint != Glyph2_p->Attributes.faint
    ||  Glyph1_p->Attributes.italic != Glyph2_p->Attributes.italic
    ||  Glyph1_p->Attributes.underline != Glyph2_p->Attributes.underline
    ||  Glyph1_p->Attributes.blink != Glyph2_p->Attributes.blink
    ||  Glyph1_p->Attributes.reverse != Glyph2_p->Attributes.reverse
    ||  Glyph1_p->Attributes.invisible != Glyph2_p->Attributes.invisible
    ||  Glyph1_p->Attributes.struck != Glyph2_p->Attributes.struck
    ||  Glyph1_p->Attributes.wrap != Glyph2_p->Attributes.wrap
    ||  Glyph1_p->Attributes.wide != Glyph2_p->Attributes.wide) {
        return true;
    }

    // Compare background.
    if (Glyph1_p->Foreground.custom != Glyph2_p->Foreground.custom
    ||  Glyph1_p->Foreground.Color.r != Glyph2_p->Foreground.Color.r
    ||  Glyph1_p->Foreground.Color.g != Glyph2_p->Foreground.Color.g
    ||  Glyph1_p->Foreground.Color.b != Glyph2_p->Foreground.Color.b
    ||  Glyph1_p->Foreground.Color.a != Glyph2_p->Foreground.Color.a) {
        return true;
    }

    return false;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_updateCursorTile(
    ttyr_terminal_Grid *Grid_p, ttyr_terminal_GraphicsState *State_p, ttyr_terminal_TileUpdate *Update_p)
{
    // Check if inside draw area, if not don't show cursor.
    if (Update_p->row >= Grid_p->rows || Update_p->col >= Grid_p->cols || Update_p->row < 0 || Update_p->col < 0) {
        if (Grid_p->Cursor_p) {
            Grid_p->Cursor_p->Glyph.Attributes.blink = false;
            Grid_p->Cursor_p->dirty = true;
            Grid_p->Cursor_p = NULL;
        }
        return TTYR_TERMINAL_SUCCESS;
    }

    ttyr_terminal_Tile *Tile_p = ((nh_core_List*)Grid_p->Rows.pp[Update_p->row])->pp[Update_p->col];
    ttyr_terminal_Tile *OldCursor_p = Grid_p->Cursor_p;

    if (OldCursor_p) {
        // Update old cursor tile.
        OldCursor_p->Glyph.Attributes.blink = false;
        OldCursor_p->dirty = true;
    }

    // Set new cursor.
    Grid_p->Cursor_p = Tile_p;
    Grid_p->Cursor_p->Glyph.Attributes.blink = true;
    Grid_p->Cursor_p->Glyph.mark |= TTYR_CORE_MARK_ACCENT;
    Grid_p->Cursor_p->dirty = true;

    // Trigger blink at move.
    int row, col = 0;
    ttyr_terminal_getTileRowAndColumn(Grid_p, OldCursor_p, &row, &col);
 
    if (row != Update_p->row || col != Update_p->col) {
        State_p->Blink.on = true;
        State_p->Blink.LastBlink = nh_core_getSystemTime();
    }

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_updateTileVertices(
    ttyr_core_Glyph *Glyph_p, ttyr_terminal_GraphicsState *State_p, ttyr_terminal_Grid *Grid_p, int col,
    int row, ttyr_terminal_Tile *Tile_p, bool foreground)
{
    if (foreground) {
        TTYR_TERMINAL_CHECK(ttyr_terminal_getForegroundVertices(
            State_p, Grid_p, Glyph_p, col, row, Tile_p->Foreground.vertices_p
        ))
    } else {
        TTYR_TERMINAL_CHECK(ttyr_terminal_getBackgroundVertices(
            State_p, Grid_p, Glyph_p, col, row, Tile_p->Background.vertices_p, 0, 0
        ))
    }

    return TTYR_TERMINAL_SUCCESS;
}

TTYR_TERMINAL_RESULT ttyr_terminal_updateTile(
    ttyr_terminal_Grid *Grid_p, void *state_p, ttyr_terminal_TileUpdate *Update_p, bool *update_p)
{
    // Only update cursor tile in case of cursor flag.
    if (Update_p->cursor) {
        if (update_p) {*update_p = true;}
        return ttyr_terminal_updateCursorTile(Grid_p, state_p, Update_p);
    }

    ttyr_terminal_Tile *Tile_p = ((nh_core_List*)Grid_p->Rows.pp[Update_p->row])->pp[Update_p->col];

    // Compare codepoint.
    if (Tile_p->Glyph.codepoint != Update_p->Glyph.codepoint || Tile_p->Glyph.mark != Update_p->Glyph.mark)
    {
        TTYR_TERMINAL_CHECK(ttyr_terminal_updateTileVertices(
            &Update_p->Glyph, state_p, Grid_p, Update_p->col, Update_p->row, Tile_p, true))
        TTYR_TERMINAL_CHECK(ttyr_terminal_updateTileVertices(
            &Update_p->Glyph, state_p, Grid_p, Update_p->col, Update_p->row, Tile_p, false))
        Tile_p->dirty = true;
    }

    // Compare attributes.
    if (ttyr_terminal_compareForegroundAttributes(&Tile_p->Glyph, &Update_p->Glyph)
    ||  ttyr_terminal_compareBackgroundAttributes(&Tile_p->Glyph, &Update_p->Glyph))
    {
        Tile_p->dirty = true;
    }

    if (Tile_p->dirty) {
        if (Tile_p == Grid_p->Cursor_p) {
            bool blink = Grid_p->Cursor_p->Glyph.Attributes.blink;
            memcpy(&Tile_p->Glyph, &Update_p->Glyph, sizeof(ttyr_core_Glyph));
            Tile_p->Glyph.Attributes.blink = blink;
        } else {
            // This may overwrite the cursor tile, which could cause the blink attribute to disappear.
            // Which is why we need the if above.
            memcpy(&Tile_p->Glyph, &Update_p->Glyph, sizeof(ttyr_core_Glyph));
        }
        if (update_p) {*update_p = true;}
    }

    // Update or reset right gap tile if necessary.
    if (Update_p->col == Grid_p->cols-2) {
        if (Update_p->Glyph.mark & TTYR_CORE_MARK_LINE_HORIZONTAL) {
            Update_p->col++;
            if (!(Update_p->Glyph.mark & TTYR_CORE_MARK_LINE_GRAPHICS)) {
                Update_p->Glyph.codepoint = ' ';
            }
            TTYR_TERMINAL_CHECK(ttyr_terminal_updateTile(Grid_p, state_p, Update_p, update_p))
        } else if (Update_p->Glyph.Attributes.reverse == true) {
            Update_p->Glyph.codepoint = 0;
            Update_p->col++;
            TTYR_TERMINAL_CHECK(ttyr_terminal_updateTile(Grid_p, state_p, Update_p, update_p))
        } else {
            Update_p->col++;
            Update_p->Glyph.codepoint = 0;
            TTYR_TERMINAL_CHECK(ttyr_terminal_updateTile(Grid_p, state_p, Update_p, update_p))
        } 
    }
    // Update or reset bottom gap tile if necessary.
    if (Update_p->row == Grid_p->rows-2) {
        if (Update_p->Glyph.mark & TTYR_CORE_MARK_LINE_VERTICAL) {
            Update_p->row++;
            TTYR_TERMINAL_CHECK(ttyr_terminal_updateTile(Grid_p, state_p, Update_p, update_p))
        } else if (Update_p->col == 0 && (Update_p->Glyph.codepoint == 0 || Update_p->Glyph.codepoint == ' ')) {
            Update_p->row++;
            TTYR_TERMINAL_CHECK(ttyr_terminal_updateTile(Grid_p, state_p, Update_p, update_p))
        } else {
            Update_p->row++;
            memset(&Update_p->Glyph, 0, sizeof(ttyr_core_Glyph));
            TTYR_TERMINAL_CHECK(ttyr_terminal_updateTile(Grid_p, state_p, Update_p, update_p))
        }
    }

    return TTYR_TERMINAL_SUCCESS;
}

TTYR_TERMINAL_RESULT ttyr_terminal_updateBorderGrid(
    ttyr_terminal_Grid *Grid_p, void *state_p, nh_gfx_Text *Text_p)
{
    ttyr_terminal_GraphicsState *State_p = state_p;
    ttyr_terminal_Config Config = ttyr_terminal_getConfig();

    // Free data.
    ttyr_terminal_freeGrid(Grid_p);

    // Update data.
    Grid_p->TileSize.width = nh_gfx_getTextWidth(Text_p);
    Grid_p->TileSize.height = Config.fontSize+abs(State_p->FontInstance_p->descender);

    int borderCols = (Config.border+Grid_p->TileSize.width-1)/Grid_p->TileSize.width;
    int borderColsPixels = borderCols*Grid_p->TileSize.width;
    int borderColsPixelsOffset = borderColsPixels-Config.border;

    int borderRows = (Config.border+Grid_p->TileSize.height-1)/Grid_p->TileSize.height;
    int borderRowsPixels = borderRows*Grid_p->TileSize.height;
    int borderRowsPixelsOffset = borderRowsPixels-Config.border;

    Grid_p->xOffset = borderColsPixelsOffset;
    Grid_p->yOffset = borderRowsPixelsOffset;

    Grid_p->Size.width = State_p->Viewport_p->Settings.Size.width+(borderColsPixels*2);
    Grid_p->Size.height = State_p->Viewport_p->Settings.Size.height+(borderRowsPixels*2);

    Grid_p->cols = Grid_p->Size.width / nh_gfx_getTextWidth(Text_p);
    Grid_p->rows = Grid_p->Size.height / Grid_p->TileSize.height + 1;

    for (int row = 0; row < Grid_p->rows; ++row) {
        for (int col = 0; col < Grid_p->cols; ++col) {
            ttyr_terminal_Tile *Tile_p = ttyr_terminal_getTile(Grid_p, row, col);
            TTYR_TERMINAL_CHECK_NULL(Tile_p)
            Tile_p->Glyph.mark |= TTYR_CORE_MARK_ACCENT;
            Tile_p->Glyph.Attributes.reverse = true;
            TTYR_TERMINAL_CHECK(ttyr_terminal_getBackgroundVertices(
                State_p, Grid_p, &Tile_p->Glyph, col, row, Tile_p->Background.vertices_p, borderColsPixelsOffset, borderRowsPixelsOffset
            ))
        }
    }

    return TTYR_TERMINAL_SUCCESS;
}

TTYR_TERMINAL_RESULT ttyr_terminal_updateGrid(
    ttyr_terminal_Grid *Grid_p, void *state_p, nh_gfx_Text *Text_p)
{
    ttyr_terminal_GraphicsState *State_p = state_p;
    ttyr_terminal_Config Config = ttyr_terminal_getConfig();

    State_p->FontInstance_p = nh_gfx_claimFontInstance(
        State_p->Fonts.pp[State_p->font], Config.fontSize
    );

    // Free data.
    nh_core_freeList(&State_p->Glyphs, true);
    nh_core_freeList(&State_p->Codepoints, true);

    nh_core_freeHashMap(State_p->Map);
    State_p->Map = nh_core_createHashMap();

    ttyr_terminal_freeGrid(Grid_p);

    // Update data.
    Grid_p->Size.width = State_p->Viewport_p->Settings.Size.width-(Config.border*2);
    Grid_p->Size.height = State_p->Viewport_p->Settings.Size.height-(Config.border*2);

    Grid_p->TileSize.width = nh_gfx_getTextWidth(Text_p);
    Grid_p->TileSize.height = Config.fontSize+abs(State_p->FontInstance_p->descender);

    Grid_p->cols = Grid_p->Size.width / nh_gfx_getTextWidth(Text_p);
    Grid_p->rows = Grid_p->Size.height / Grid_p->TileSize.height;

    // Add gap tiles.
    Grid_p->cols += 1;
    Grid_p->rows += 1;

    Grid_p->Updates_pp = nh_core_allocate(sizeof(void*) * Grid_p->rows);
    TTYR_TERMINAL_CHECK_MEM(Grid_p->Updates_pp)

    Grid_p->updates_pp = nh_core_allocate(sizeof(void*) * Grid_p->rows);
    TTYR_TERMINAL_CHECK_MEM(Grid_p->updates_pp)

    for (int row = 0; row < Grid_p->rows; ++row) {
        Grid_p->Updates_pp[row] = nh_core_allocate(sizeof(ttyr_terminal_TileUpdate) * Grid_p->cols);
        TTYR_TERMINAL_CHECK_MEM(Grid_p->Updates_pp[row])
        Grid_p->updates_pp[row] = nh_core_allocate(sizeof(bool) * Grid_p->cols);
        TTYR_TERMINAL_CHECK_MEM(Grid_p->updates_pp[row])
        memset(Grid_p->updates_pp[row], 0, sizeof(bool)*Grid_p->cols);
    }

    for (int row = 0; row < Grid_p->rows; ++row) {
        for (int col = 0; col < Grid_p->cols; ++col) {
            ttyr_terminal_Tile *Tile_p =
                ttyr_terminal_getTile(Grid_p, row, col);
            TTYR_TERMINAL_CHECK_NULL(Tile_p)
            TTYR_TERMINAL_CHECK(ttyr_terminal_updateTileVertices(
                &Tile_p->Glyph, state_p, Grid_p, col, row, Tile_p, true))
            TTYR_TERMINAL_CHECK(ttyr_terminal_updateTileVertices(
                &Tile_p->Glyph, state_p, Grid_p, col, row, Tile_p, false))
        }
    }

    return TTYR_TERMINAL_SUCCESS;
}

TTYR_TERMINAL_RESULT ttyr_terminal_updateBoxes(
    ttyr_terminal_Grid *Grid_p, void *state_p, nh_core_Array *Boxes_p)
{   
    if (Grid_p->Boxes.length > 0) {
        nh_core_freeArray(&Grid_p->Boxes);
    }

    for (int i = 0; i < Boxes_p->length; ++i) 
    {
        ttyr_terminal_Box *Box_p = nh_core_incrementArray(&Grid_p->Boxes);
        TTYR_TERMINAL_CHECK_MEM(Box_p)
        *Box_p = ((ttyr_terminal_Box*)Boxes_p->p)[i];

        if (Box_p->UpperLeft.x == Box_p->LowerRight.x && Box_p->UpperLeft.y == Box_p->UpperLeft.y) {
            TTYR_TERMINAL_CHECK(ttyr_terminal_getOutlineVertices(state_p, Grid_p, Box_p, true))
            TTYR_TERMINAL_CHECK(ttyr_terminal_getOutlineVertices(state_p, Grid_p, Box_p, false))
        } else {
            TTYR_TERMINAL_CHECK(ttyr_terminal_getBoxVertices(state_p, Grid_p, Box_p, true))
            TTYR_TERMINAL_CHECK(ttyr_terminal_getBoxVertices(state_p, Grid_p, Box_p, false))
        }
    }

    return TTYR_TERMINAL_SUCCESS;
}
