// LICENSE NOTICE ==================================================================================

/**
 * Termoskanne - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Grid.h"
#include "Graphics.h"
#include "Vertices.h"

#include "../Vulkan/Render.h"
#include "../Common/Macros.h"

#include "nh-gfx/Base/Viewport.h"

#include "nh-core/System/Thread.h"
#include "nh-core/System/Memory.h"
#include "nh-core/System/Process.h"

#include "nh-encoding/Encodings/UTF8.h"
#include "nh-encoding/Encodings/UTF32.h"

#include <string.h>
#include <stdio.h>

// INIT ============================================================================================

TK_TERMINAL_RESULT tk_terminal_initGrid(
    tk_terminal_Grid *Grid_p)
{
    memset(Grid_p, 0, sizeof(tk_terminal_Grid));
    Grid_p->Rows = nh_core_initList(128);
    return TK_TERMINAL_SUCCESS;
}

TK_TERMINAL_RESULT tk_terminal_freeGrid(
    tk_terminal_Grid *Grid_p)
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
    tk_terminal_initGrid(Grid_p);

    return TK_TERMINAL_SUCCESS;
}

static TK_TERMINAL_RESULT tk_terminal_initTile(
    tk_terminal_Grid *Grid_p, tk_terminal_Tile *Tile_p, int row, int col)
{
    memset(Tile_p, 0, sizeof(tk_terminal_Tile));

    memset(Tile_p->Foreground.vertices_p, 0, sizeof(float) * 20);
    memset(Tile_p->Background.vertices_p, 0, sizeof(float) * 12);

    return TK_TERMINAL_SUCCESS;
}

// UPDATE ==========================================================================================

static void tk_terminal_getTileRowAndColumn(
    tk_terminal_Grid *Grid_p, tk_terminal_Tile *Tile_p, int *row_p, int *col_p)
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

tk_terminal_Tile *tk_terminal_getTile(
    tk_terminal_Grid *Grid_p, int row, int col)
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
        TK_TERMINAL_CHECK_MEM_2(NULL, Grid_p->Rows.pp[row])
        *((nh_core_List*)Grid_p->Rows.pp[row]) = nh_core_initList(128);
    }

    nh_core_List *Cols_p = Grid_p->Rows.pp[row];

     while (Cols_p->size <= col) {
        nh_core_appendToList(Cols_p, NULL);
    }

    if (!Cols_p->pp[col]) {
        Cols_p->pp[col] = nh_core_allocate(sizeof(tk_terminal_Tile));
        TK_TERMINAL_CHECK_MEM_2(NULL, Cols_p->pp[col])
        TK_TERMINAL_CHECK_2(NULL, tk_terminal_initTile(Grid_p, Cols_p->pp[col], row, col))
    }

    return Cols_p->pp[col];
}

tk_terminal_Tile *tk_terminal_getTileUnsafe(
    tk_terminal_Grid *Grid_p, int row, int col)
{
    return ((nh_core_List*)Grid_p->Rows.pp[row])->pp[col];
}

bool tk_terminal_compareBackgroundAttributes(
    tk_core_Glyph *Glyph1_p, tk_core_Glyph *Glyph2_p)
{
    // Compare attributes.
    if ((Glyph1_p->mark & TK_CORE_MARK_ACCENT) != (Glyph2_p->mark & TK_CORE_MARK_ACCENT)) {
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

bool tk_terminal_compareForegroundAttributes(
    tk_core_Glyph *Glyph1_p, tk_core_Glyph *Glyph2_p)
{
    if ((Glyph1_p->mark & TK_CORE_MARK_ACCENT) != (Glyph2_p->mark & TK_CORE_MARK_ACCENT)) {
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

static TK_TERMINAL_RESULT tk_terminal_updateCursorTile(
    tk_terminal_Grid *Grid_p, tk_terminal_GraphicsState *State_p, tk_terminal_TileUpdate *Update_p)
{
    // Check if inside draw area, if not don't show cursor.
    if (Update_p->row >= Grid_p->rows || Update_p->col >= Grid_p->cols || Update_p->row < 0 || Update_p->col < 0) {
        if (Grid_p->Cursor_p) {
            Grid_p->Cursor_p->Glyph.Attributes.blink = false;
            Grid_p->Cursor_p->dirty = true;
            Grid_p->Cursor_p = NULL;
        }
        return TK_TERMINAL_SUCCESS;
    }

    tk_terminal_Tile *Tile_p = ((nh_core_List*)Grid_p->Rows.pp[Update_p->row])->pp[Update_p->col];
    tk_terminal_Tile *OldCursor_p = Grid_p->Cursor_p;

    if (OldCursor_p) {
        // Update old cursor tile.
        OldCursor_p->Glyph.Attributes.blink = false;
        OldCursor_p->dirty = true;
    }

    // Set new cursor.
    Grid_p->Cursor_p = Tile_p;
    Grid_p->Cursor_p->Glyph.Attributes.blink = true;
    Grid_p->Cursor_p->Glyph.mark |= TK_CORE_MARK_ACCENT;
    Grid_p->Cursor_p->dirty = true;

    // Trigger blink at move.
    int row, col = 0;
    tk_terminal_getTileRowAndColumn(Grid_p, OldCursor_p, &row, &col);
 
    if (row != Update_p->row || col != Update_p->col) {
        State_p->Blink.on = true;
        State_p->Blink.LastBlink = nh_core_getSystemTime();
    }

    return TK_TERMINAL_SUCCESS;
}

static TK_TERMINAL_RESULT tk_terminal_updateTileVertices(
    tk_core_Glyph *Glyph_p, tk_terminal_GraphicsState *State_p, tk_terminal_Grid *Grid_p, int col,
    int row, tk_terminal_Tile *Tile_p, bool foreground, int fontSize)
{
    if (foreground) {
        TK_TERMINAL_CHECK(tk_terminal_getForegroundVertices(
            State_p, Grid_p, Glyph_p, col, row, Tile_p->Foreground.vertices_p, fontSize
        ))
    } else {
        TK_TERMINAL_CHECK(tk_terminal_getBackgroundVertices(
            State_p, Grid_p, Glyph_p, col, row, Tile_p->Background.vertices_p, fontSize
        ))
    }

    return TK_TERMINAL_SUCCESS;
}

TK_TERMINAL_RESULT tk_terminal_updateTile(
    tk_terminal_Grid *Grid_p, void *state_p, tk_terminal_TileUpdate *Update_p, bool *update_p, int fontSize)
{
    // Only update cursor tile in case of cursor flag.
    if (Update_p->cursor) {
        if (update_p) {*update_p = true;}
        return tk_terminal_updateCursorTile(Grid_p, state_p, Update_p);
    }

    tk_terminal_Tile *Tile_p = ((nh_core_List*)Grid_p->Rows.pp[Update_p->row])->pp[Update_p->col];

    // Compare codepoint.
    if (Tile_p->Glyph.codepoint != Update_p->Glyph.codepoint || Tile_p->Glyph.mark != Update_p->Glyph.mark)
    {
        TK_TERMINAL_CHECK(tk_terminal_updateTileVertices(
            &Update_p->Glyph, state_p, Grid_p, Update_p->col, Update_p->row, Tile_p, true, fontSize))
        TK_TERMINAL_CHECK(tk_terminal_updateTileVertices(
            &Update_p->Glyph, state_p, Grid_p, Update_p->col, Update_p->row, Tile_p, false, fontSize))
        Tile_p->dirty = true;
    }

    // Compare attributes.
    if (tk_terminal_compareForegroundAttributes(&Tile_p->Glyph, &Update_p->Glyph)
    ||  tk_terminal_compareBackgroundAttributes(&Tile_p->Glyph, &Update_p->Glyph))
    {
        Tile_p->dirty = true;
    }

    if (Tile_p->dirty) {
        if (Tile_p == Grid_p->Cursor_p) {
            bool blink = Grid_p->Cursor_p->Glyph.Attributes.blink;
            memcpy(&Tile_p->Glyph, &Update_p->Glyph, sizeof(tk_core_Glyph));
            Tile_p->Glyph.Attributes.blink = blink;
        } else {
            // This may overwrite the cursor tile, which could cause the blink attribute to disappear.
            // Which is why we need the if above.
            memcpy(&Tile_p->Glyph, &Update_p->Glyph, sizeof(tk_core_Glyph));
        }
        if (update_p) {*update_p = true;}
    }

    // Update or reset right gap tile if necessary.
    if (Update_p->col == Grid_p->cols-2) {
        if (Update_p->Glyph.mark & TK_CORE_MARK_LINE_HORIZONTAL) {
            Update_p->col++;
            if (!(Update_p->Glyph.mark & TK_CORE_MARK_LINE_GRAPHICS)) {
                Update_p->Glyph.codepoint = ' ';
            }
            TK_TERMINAL_CHECK(tk_terminal_updateTile(Grid_p, state_p, Update_p, update_p, fontSize))
        } else if (Update_p->Glyph.Attributes.reverse == true) {
            Update_p->Glyph.codepoint = 0;
            Update_p->col++;
            TK_TERMINAL_CHECK(tk_terminal_updateTile(Grid_p, state_p, Update_p, update_p, fontSize))
        } else {
            Update_p->col++;
            Update_p->Glyph.codepoint = 0;
            TK_TERMINAL_CHECK(tk_terminal_updateTile(Grid_p, state_p, Update_p, update_p, fontSize))
        } 
    }

    // Update or reset bottom gap tile if necessary.
    if (Update_p->row == Grid_p->rows-2) {
        if (Update_p->Glyph.mark & TK_CORE_MARK_LINE_VERTICAL) {
            Update_p->row++;
            TK_TERMINAL_CHECK(tk_terminal_updateTile(Grid_p, state_p, Update_p, update_p, fontSize))
        } else if (Update_p->col == 0 && (Update_p->Glyph.codepoint == 0 || Update_p->Glyph.codepoint == ' ')) {
            Update_p->row++;
            TK_TERMINAL_CHECK(tk_terminal_updateTile(Grid_p, state_p, Update_p, update_p, fontSize))
        } else {
            Update_p->row++;
            memset(&Update_p->Glyph, 0, sizeof(tk_core_Glyph));
            TK_TERMINAL_CHECK(tk_terminal_updateTile(Grid_p, state_p, Update_p, update_p, fontSize))
        }
    }

    return TK_TERMINAL_SUCCESS;
}

TK_TERMINAL_RESULT tk_terminal_updateBackdropGrid(
    tk_terminal_Config *Config_p, tk_terminal_Grid *BackdropGrid_p, void *state_p, nh_gfx_Text *Text_p)
{
    tk_terminal_GraphicsState *State_p = state_p;

    // Free data.
    tk_terminal_freeGrid(BackdropGrid_p);

    // Update data.
    BackdropGrid_p->TileSize.width = nh_gfx_getTextWidth(Text_p);
    BackdropGrid_p->TileSize.height = Config_p->fontSize+abs(State_p->FontInstance_p->descender);

    BackdropGrid_p->borderPixel = BackdropGrid_p->TileSize.width/3;

    int borderCols = (BackdropGrid_p->borderPixel+BackdropGrid_p->TileSize.width-1)/BackdropGrid_p->TileSize.width;
    int borderColsPixel = borderCols*BackdropGrid_p->TileSize.width;
    int borderColsPixelOffset = borderColsPixel-BackdropGrid_p->borderPixel;

    int borderRows = (BackdropGrid_p->borderPixel+BackdropGrid_p->TileSize.height-1)/BackdropGrid_p->TileSize.height;
    int borderRowsPixel = borderRows*BackdropGrid_p->TileSize.height;
    int borderRowsPixelOffset = borderRowsPixel-BackdropGrid_p->borderPixel;

    BackdropGrid_p->xOffset = borderColsPixelOffset;
    BackdropGrid_p->yOffset = borderRowsPixelOffset;

    BackdropGrid_p->Size.width = State_p->Viewport_p->Settings.Size.width+(borderColsPixel*2);
    BackdropGrid_p->Size.height = State_p->Viewport_p->Settings.Size.height+(borderRowsPixel*2);

    BackdropGrid_p->cols = BackdropGrid_p->Size.width / nh_gfx_getTextWidth(Text_p) + 2;
    BackdropGrid_p->rows = BackdropGrid_p->Size.height / BackdropGrid_p->TileSize.height + 2;

    for (int row = 0; row < BackdropGrid_p->rows; ++row) {
        for (int col = 0; col < BackdropGrid_p->cols; ++col) {
            tk_terminal_Tile *Tile_p = tk_terminal_getTile(BackdropGrid_p, row, col);
            TK_TERMINAL_CHECK_NULL(Tile_p)
            Tile_p->Glyph.mark = TK_CORE_MARK_ACCENT | TK_CORE_MARK_LINE_GRAPHICS;
            Tile_p->Glyph.Attributes.reverse = true;
            TK_TERMINAL_CHECK(tk_terminal_getBackgroundVertices(
                State_p, BackdropGrid_p, &Tile_p->Glyph, col, row, Tile_p->Background.vertices_p, Config_p->fontSize
            ))
            if (row == 2 && Tile_p->Glyph.codepoint == 'x') {
                TK_TERMINAL_CHECK(tk_terminal_getForegroundVertices(
                    State_p, BackdropGrid_p, &Tile_p->Glyph, col, row, Tile_p->Foreground.vertices_p, Config_p->fontSize 
                ))
            }
        }
    }

    return TK_TERMINAL_SUCCESS;
}

TK_TERMINAL_RESULT tk_terminal_updateGrid(
    tk_terminal_Config *Config_p, tk_terminal_Grid *Grid_p, void *state_p, nh_gfx_Text *Text_p)
{
    tk_terminal_GraphicsState *State_p = state_p;

    State_p->FontInstance_p = nh_gfx_claimFontInstance(
        State_p->Fonts.pp[State_p->font], Config_p->fontSize
    );

    // Free data.
    nh_core_freeList(&State_p->Glyphs, true);
    nh_core_freeList(&State_p->Codepoints, true);

    nh_core_freeHashMap(State_p->Map);
    State_p->Map = nh_core_createHashMap();

    tk_terminal_freeGrid(Grid_p);

    // Update data.
    Grid_p->TileSize.width = nh_gfx_getTextWidth(Text_p);
    Grid_p->TileSize.height = Config_p->fontSize+abs(State_p->FontInstance_p->descender);

    Grid_p->borderPixel = Grid_p->TileSize.width/3;

    Grid_p->Size.width = State_p->Viewport_p->Settings.Size.width-(Grid_p->borderPixel*2);
    Grid_p->Size.height = State_p->Viewport_p->Settings.Size.height-(Grid_p->borderPixel*2);

    Grid_p->cols = Grid_p->Size.width / nh_gfx_getTextWidth(Text_p);
    Grid_p->rows = Grid_p->Size.height / Grid_p->TileSize.height;

    // Add gap tiles.
    Grid_p->cols += 1;
    Grid_p->rows += 1;

    Grid_p->Updates_pp = (tk_terminal_TileUpdate**)nh_core_allocate(sizeof(tk_terminal_TileUpdate*) * Grid_p->rows);
    TK_TERMINAL_CHECK_MEM(Grid_p->Updates_pp)

    Grid_p->updates_pp = (bool**)nh_core_allocate(sizeof(bool*) * Grid_p->rows);
    TK_TERMINAL_CHECK_MEM(Grid_p->updates_pp)

    for (int row = 0; row < Grid_p->rows; ++row) {
        Grid_p->Updates_pp[row] = (tk_terminal_TileUpdate*)nh_core_allocate(sizeof(tk_terminal_TileUpdate) * Grid_p->cols);
        TK_TERMINAL_CHECK_MEM(Grid_p->Updates_pp[row])
        Grid_p->updates_pp[row] = (bool*)nh_core_allocate(sizeof(bool) * Grid_p->cols);
        TK_TERMINAL_CHECK_MEM(Grid_p->updates_pp[row])
        memset(Grid_p->updates_pp[row], 0, sizeof(bool)*Grid_p->cols);
    }

    for (int row = 0; row < Grid_p->rows; ++row) {
        for (int col = 0; col < Grid_p->cols; ++col) {
            tk_terminal_Tile *Tile_p =
                tk_terminal_getTile(Grid_p, row, col);
            TK_TERMINAL_CHECK_NULL(Tile_p)
            TK_TERMINAL_CHECK(tk_terminal_updateTileVertices(
                &Tile_p->Glyph, state_p, Grid_p, col, row, Tile_p, true, Config_p->fontSize))
            TK_TERMINAL_CHECK(tk_terminal_updateTileVertices(
                &Tile_p->Glyph, state_p, Grid_p, col, row, Tile_p, false, Config_p->fontSize))
        }
    }

    return TK_TERMINAL_SUCCESS;
}
