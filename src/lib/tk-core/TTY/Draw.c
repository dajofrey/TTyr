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

#include "Draw.h"
#include "Topbar.h"
#include "View.h"
#include "Program.h"
#include "Menu.h"

#include "../Common/Macros.h"

#include "nh-core/System/Memory.h"
#include "nh-core/System/Thread.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

// FUNCTIONS: CURSOR ================================================================================

TTYR_CORE_RESULT tk_core_getCursorPosition(
    tk_core_Config *Config_p, tk_core_Tile *MacroTile_p, tk_core_Tile *MicroTile_p, bool standardIO, int *x_p, int *y_p)
{
    *x_p = -1;
    *y_p = -1;

    if (TTYR_CORE_MACRO_TAB(MacroTile_p)->Topbar.hasFocus) {
        TTYR_CHECK(tk_core_getTopbarCursor(&TTYR_CORE_MACRO_TAB(MacroTile_p)->Topbar, x_p, y_p, MacroTile_p->rowPosition == 0))
    }
    else if (TTYR_CORE_MICRO_TILE(MicroTile_p)->Program_p != NULL && TTYR_CORE_MICRO_TILE(MicroTile_p)->Program_p->Prototype_p->Callbacks.getCursorPosition_f != NULL) {
        TTYR_CHECK(TTYR_CORE_MICRO_TILE(MicroTile_p)->Program_p->Prototype_p->Callbacks.getCursorPosition_f(TTYR_CORE_MICRO_TILE(MicroTile_p)->Program_p, x_p, y_p))
        if (*x_p < 0 || *y_p < 0) {
             // Indicates that the program doesn't want the cursor to be shown.
             return TTYR_CORE_SUCCESS;
        }

        *y_p += 1 + (Config_p->Topbar.on || Config_p->Titlebar.on);
        *y_p += MicroTile_p->rowPosition > 0;
        *y_p += (!Config_p->Topbar.on && !Config_p->Titlebar.on) && MacroTile_p->rowPosition > 0;

        if (*x_p > -1 && *y_p > -1) {
            *x_p += MicroTile_p->colPosition;
            *y_p += MicroTile_p->rowPosition;
        }
    }

    if (*x_p > -1) {
        *x_p += MacroTile_p->colPosition + 1;
        *y_p += MacroTile_p->rowPosition;
    }

    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT tk_core_refreshCursor(
    tk_core_TTY *TTY_p) 
{
    tk_core_View *View_p = TTY_p->Views.pp[0];
    int x = -1, y = -1;
 
    if (TTY_p->hasFocus) {
        tk_core_Tile *MicroTile_p = TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(TTY_p->Window_p->Tile_p))->Tile_p;
        TTYR_CHECK(tk_core_getCursorPosition(&TTY_p->Config, TTY_p->Window_p->Tile_p, MicroTile_p, View_p->standardIO, &x, &y))
    }

    TTYR_CHECK(tk_core_forwardCursor(&TTY_p->Config, View_p, x, y))

    return TTYR_CORE_SUCCESS;
}

// FUNCTIONS: DRAW =================================================================================

static void tk_core_normalizeGlyph(
    tk_core_Glyph *Glyph_p)
{
    memset(Glyph_p, 0, sizeof(tk_core_Glyph));
    Glyph_p->codepoint = ' ';
    return;
}

static void tk_core_drawVerticalBorderGlyph(
    tk_core_Glyph *Glyph_p)
{
    memset(Glyph_p, 0, sizeof(tk_core_Glyph));

    Glyph_p->codepoint = ' ';
    Glyph_p->mark = TTYR_CORE_MARK_LINE_VERTICAL | TTYR_CORE_MARK_ACCENT;
    Glyph_p->Attributes.reverse = true;
    return;
}

static TTYR_CORE_RESULT tk_core_drawMicroTile(
    tk_core_Config *Config_p, tk_core_Tile *Tile_p, tk_core_View *View_p, int row)
{
    // Get relative row with 0 being the first row of the tile.
    row = row - Tile_p->rowPosition;
    int cols = Tile_p->colSize;

    // Draw vertical border and subtract from cols, if necessary.
    if (Tile_p->rightSeparator) {
        tk_core_drawVerticalBorderGlyph(&View_p->Row.Glyphs_p[cols-1]);
        cols--;
    }

    int offset = Tile_p->rowPosition != 0;
 
    // Draw topbar, if necessary.
    if (!View_p->standardIO) {
        bool topbar = row == 0;
        if (Tile_p->rowPosition == 0) {topbar = false;}
 
        if (topbar) {
            return tk_core_drawTopbarRow(
                NULL, View_p->Row.Glyphs_p, cols, row, View_p->standardIO
            );
        }
    }
 
    if (TTYR_CORE_MICRO_TILE(Tile_p)->Program_p) {
        TTYR_CHECK(TTYR_CORE_MICRO_TILE(Tile_p)->Program_p->Prototype_p->Callbacks.draw_f(
            TTYR_CORE_MICRO_TILE(Tile_p)->Program_p, View_p->Row.Glyphs_p, cols, 
            Tile_p->rowSize-offset,
            row-offset
        ))
    }

    return TTYR_CORE_SUCCESS;
}

static TTYR_CORE_RESULT tk_core_drawMacroTile(
    tk_core_Config *Config_p, tk_core_Tile *Tile_p, tk_core_View *View_p, int row)
{

    // Get relative row with 0 being the first row of the tile.
    row = row - Tile_p->rowPosition;

    int cols = Tile_p->colSize;
    bool topbar = Tile_p->rowPosition != 0 || Config_p->Topbar.on || Config_p->Titlebar.on;

    // Draw vertical border and subtract from cols, if necessary.
    if (Tile_p->rightSeparator) {
        tk_core_drawVerticalBorderGlyph(&View_p->Row.Glyphs_p[cols-1]);
        cols--;
    }

    if (topbar && row == 0 && !View_p->standardIO) {
        return tk_core_drawTopbarRow(
            Tile_p, View_p->Row.Glyphs_p, cols, row, View_p->standardIO
        );
    }
 
    TTYR_CHECK(tk_core_drawMicroWindow(
        Config_p,
        &TTYR_CORE_MACRO_TAB(Tile_p)->MicroWindow, View_p->Row.Glyphs_p, cols,
        topbar ? Tile_p->rowSize-1 : Tile_p->rowSize, 
        topbar ? row-1 : row, 
        View_p->standardIO
    ))

    return TTYR_CORE_SUCCESS;
}

static TTYR_CORE_RESULT tk_core_draw(
    tk_core_Config *Config_p, tk_core_Tile *Tile_p, tk_core_View *View_p, int row)
{
    // Normalize glyphs.
    for (int i = 0; i < Tile_p->colSize; ++i) {
        tk_core_normalizeGlyph(View_p->Row.Glyphs_p+i);
    }
 
    return Tile_p->type == TTYR_CORE_TILE_TYPE_MACRO ? 
        tk_core_drawMacroTile(Config_p, Tile_p, View_p, row) : tk_core_drawMicroTile(Config_p, Tile_p, View_p, row);
}

// FUNCTIONS: REFRESH ===============================================================================

static TTYR_CORE_RESULT tk_core_postProcessRow(
    tk_core_View *View_p, int row)
{
    tk_core_Row *Row_p = View_p->Grid1_p+row;

    if (View_p->standardIO) {return TTYR_CORE_SUCCESS;}

    // Post process line.
    for (int i = 0; i < View_p->cols; ++i) {
        tk_core_Glyph *Glyph_p = &Row_p->Glyphs_p[i];
        if (Glyph_p->mark & TTYR_CORE_MARK_LINE_VERTICAL) {
            Glyph_p->codepoint = 'x';
            Glyph_p->Attributes.reverse = false;
            Glyph_p->mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
            if (((Glyph_p+1)->Attributes.reverse && (Glyph_p+1)->mark & TTYR_CORE_MARK_LINE_HORIZONTAL)
            &&  ((Glyph_p-1)->Attributes.reverse && (Glyph_p-1)->mark & TTYR_CORE_MARK_LINE_HORIZONTAL)) {
                Glyph_p->Attributes.reverse = true;
                continue;
            } 
            if ((Glyph_p+1)->mark & TTYR_CORE_MARK_LINE_HORIZONTAL) {
                if ((Glyph_p-1)->mark & TTYR_CORE_MARK_LINE_HORIZONTAL) {
                    if (row > 0 && ((Row_p-1)->Glyphs_p[i].mark & TTYR_CORE_MARK_LINE_VERTICAL)) {
                        if ((Glyph_p+1)->Attributes.reverse) {
                            Glyph_p->codepoint = 'b';
                        } else if ((Glyph_p-1)->Attributes.reverse) {
                            Glyph_p->codepoint = 'c';
                        } else {
                            Glyph_p->codepoint = 'n';
                        }
                    } else {
                        Glyph_p->codepoint = 'w';
                    }
                }
                else {
                    Glyph_p->codepoint = (Glyph_p+1)->Attributes.reverse ? 'd' : 't';
                }
            }
            else if (i < View_p->cols-1 && ((Glyph_p-1)->mark & TTYR_CORE_MARK_LINE_HORIZONTAL)) {
                Glyph_p->codepoint = (Glyph_p-1)->Attributes.reverse ? 'e' : 'u';
            }
        }
        if (Glyph_p->mark & TTYR_CORE_MARK_LINE_HORIZONTAL && !Glyph_p->Attributes.reverse) {
            if (row > 0 && ((Row_p-1)->Glyphs_p[i].mark & TTYR_CORE_MARK_LINE_VERTICAL)) {
                Glyph_p->codepoint = 'v';
            }
        }
        if (Glyph_p->mark & TTYR_CORE_MARK_LINE_HORIZONTAL && !Glyph_p->Attributes.reverse) {
            if (View_p->rows>row+1 && ((Row_p+1)->Glyphs_p[i].mark & TTYR_CORE_MARK_LINE_VERTICAL)) {
                Glyph_p->codepoint = 'w';
            }
        }
        if (Glyph_p->mark & TTYR_CORE_MARK_LINE_VERTICAL) {
            if (i < View_p->cols-1 && (Glyph_p+1)->Attributes.reverse) {
                Glyph_p->mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
                Glyph_p->codepoint = 'd';
            }
        }
        if (Glyph_p->mark & TTYR_CORE_MARK_LINE_VERTICAL) {
            if (i > 0 && (Glyph_p-1)->Attributes.reverse) {
                Glyph_p->mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
                Glyph_p->codepoint = 'e';
            }
        }
        if (Glyph_p->mark & TTYR_CORE_MARK_LINE_VERTICAL) {
            if (i > 0 && (Glyph_p-1)->Attributes.reverse && i < View_p->cols-1 && (Glyph_p+1)->Attributes.reverse) {
                Glyph_p->Attributes.reverse = true;
                Glyph_p->mark = 0;
                Glyph_p->codepoint = 0;
            }
        }

    }

    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT tk_core_refreshGrid1Row(
    tk_core_Config *Config_p, nh_core_List *Tiles_p, tk_core_View *View_p, int row)
{
    memset(View_p->Row.Glyphs_p, 0, sizeof(tk_core_Glyph)*View_p->cols);
    int offset = 0;

    for (int col = offset; col < View_p->cols;) {
        for (int tile = 0; tile < Tiles_p->size; ++tile) {

            tk_core_Tile *Tile_p = Tiles_p->pp[tile];
            if (Tile_p->Children.count > 0) {continue;}

            Tile_p->refresh = false;

            if (Tile_p->rowPosition <= row
            &&  Tile_p->rowPosition  + Tile_p->rowSize > row
            &&  Tile_p->colPosition == col-offset)
            {
                TTYR_CHECK(tk_core_draw(Config_p, Tile_p, View_p, row))

                for (int i = 0; i < Tile_p->colSize; ++i) {
                    tk_core_Glyph *Glyph_p = View_p->Grid1_p[row].Glyphs_p+col+i;
                    if (memcmp(Glyph_p, View_p->Row.Glyphs_p+i, sizeof(tk_core_Glyph))) {
                        View_p->Grid1_p[row].update_p[col+i] = true;
                    }
                }

                memcpy(View_p->Grid1_p[row].Glyphs_p+col, View_p->Row.Glyphs_p, 
                    sizeof(tk_core_Glyph)*Tile_p->colSize);

                col += Tile_p->colSize;
                break;
            }
        }
    }

    TTYR_CHECK(tk_core_postProcessRow(View_p, row))

    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT tk_core_refreshGrid1(
    tk_core_TTY *TTY_p) 
{
    tk_core_View *View_p = TTY_p->Views.pp[0];
    int offset = 0;

    tk_core_updateTiling(TTY_p->Window_p->RootTile_p, View_p->rows, View_p->cols-offset);
    nh_core_List Tiles = tk_core_getTiles(TTY_p->Window_p->RootTile_p);

    for (int row = 0; row < View_p->rows; ++row) {
        TTYR_CHECK(tk_core_refreshGrid1Row(&TTY_p->Config, &Tiles, View_p, row))
    }

    nh_core_freeList(&Tiles, false);

    TTYR_CHECK(tk_core_forwardGrid1(&TTY_p->Config, View_p))

    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT tk_core_refreshGrid2(
    tk_core_TTY *TTY_p)
{
    tk_core_View *View_p = TTY_p->Views.pp[0];

    for (int row = 0; row < View_p->rows; ++row) {
        memset(View_p->Grid2_p[row].Glyphs_p, 0, sizeof(tk_core_Glyph)*View_p->cols);
    }
 
    if (TTY_p->alt && TTY_p->ctrl) {
        TTYR_CHECK(tk_core_drawMicroWindowMenu(View_p->Grid2_p))
    }
//    if (1) {
//        TTYR_CHECK(tk_core_drawPrompt(View_p->Grid2_p))
//    }

    TTYR_CHECK(tk_core_drawContextMenuRecursively(TTY_p->Window_p->MouseMenu_p, View_p->Grid2_p))
    TTYR_CHECK(tk_core_forwardGrid2(View_p))

    return TTYR_CORE_SUCCESS;
}
