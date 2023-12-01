// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 *
 * See https://vt100.net/docs/vt100-ug/chapter3.html for escape sequences.
 */

// DEFINE ==========================================================================================

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

// INCLUDES ========================================================================================

#include "Draw.h"
#include "TopBar.h"
#include "SideBar.h"
#include "View.h"
#include "Program.h"

#include "../Common/Macros.h"

#include "../../../../external/Netzhaut/src/lib/nhcore/System/Memory.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/System/Thread.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

// CURSOR ==========================================================================================

TTYR_TTY_RESULT ttyr_tty_getCursorPosition(
    ttyr_tty_Tile *MacroTile_p, ttyr_tty_Tile *MicroTile_p, NH_BOOL standardIO, int *x_p, int *y_p)
{
TTYR_TTY_BEGIN()

    *x_p = -1;
    *y_p = -1;

    if (TTYR_TTY_MACRO_TAB(MacroTile_p)->TopBar.hasFocus) {
        TTYR_TTY_CHECK(ttyr_tty_getTopBarCursor(&TTYR_TTY_MACRO_TAB(MacroTile_p)->TopBar, x_p, y_p, MacroTile_p->rowPosition == 0))
    }
    else if (TTYR_TTY_MICRO_TILE(MicroTile_p)->Program_p != NULL && TTYR_TTY_MICRO_TILE(MicroTile_p)->Program_p->Prototype_p->Callbacks.getCursorPosition_f != NULL) {
        TTYR_TTY_CHECK(TTYR_TTY_MICRO_TILE(MicroTile_p)->Program_p->Prototype_p->Callbacks.getCursorPosition_f(TTYR_TTY_MICRO_TILE(MicroTile_p)->Program_p, x_p, y_p))
        if (*x_p < 0 || *y_p < 0) {
             // Indicates that the program doesn't want the cursor to be shown.
             TTYR_TTY_END(TTYR_TTY_SUCCESS)
        }
        *y_p += 2; 
        if (*x_p > -1 && *y_p > -1) {
            *x_p += MicroTile_p->colPosition;
            *y_p += MicroTile_p->rowPosition;
        }
    }

    if (*x_p > -1) {
        *x_p += MacroTile_p->colPosition + 1;
        *y_p += MacroTile_p->rowPosition;
    }

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

TTYR_TTY_RESULT ttyr_tty_refreshCursor(
    ttyr_tty_TTY *TTY_p) 
{
TTYR_TTY_BEGIN()

    ttyr_tty_View *View_p = TTY_p->Views.pp[0];
    int x = -1, y = -1;
 
    if (TTY_p->hasFocus) {
        ttyr_tty_Tile *MicroTile_p = TTYR_TTY_MICRO_TAB(TTYR_TTY_MACRO_TAB(TTY_p->Window_p->Tile_p))->Tile_p;
        TTYR_TTY_CHECK(ttyr_tty_getCursorPosition(TTY_p->Window_p->Tile_p, MicroTile_p, View_p->standardIO, &x, &y))
    }

    TTYR_TTY_CHECK(ttyr_tty_forwardCursor(View_p, x, y))

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

// DRAW ============================================================================================

static void ttyr_tty_normalizeGlyph(
    ttyr_tty_Glyph *Glyph_p)
{
TTYR_TTY_BEGIN()

    memset(Glyph_p, 0, sizeof(ttyr_tty_Glyph));
    Glyph_p->codepoint = ' ';

TTYR_TTY_SILENT_END()
}

static void ttyr_tty_drawVerticalBorderGlyph(
    ttyr_tty_Glyph *Glyph_p)
{
TTYR_TTY_BEGIN()

    memset(Glyph_p, 0, sizeof(ttyr_tty_Glyph));

    Glyph_p->codepoint = ' ';
    Glyph_p->mark = TTYR_TTY_MARK_LINE_VERTICAL | TTYR_TTY_MARK_ACCENT;
    Glyph_p->Attributes.reverse = NH_TRUE;

TTYR_TTY_SILENT_END()
}

static TTYR_TTY_RESULT ttyr_tty_draw(
    ttyr_tty_Tile *Tile_p, ttyr_tty_View *View_p, int row)
{
TTYR_TTY_BEGIN()

    // Normalize glyphs.
    for (int i = 0; i < Tile_p->colSize; ++i) {
        ttyr_tty_normalizeGlyph(View_p->Row.Glyphs_p+i);
    }

    // Get relative row with 0 being the first row of the tile.
    row = row - Tile_p->rowPosition;

    int cols = Tile_p->colSize;
    NH_BOOL topbar = NH_FALSE;

    // Check if topbar should be drawn. 
    if (Tile_p->type == TTYR_TTY_TILE_TYPE_MACRO) {
        topbar = row == 0;
    } else {
        topbar = Tile_p->rowPosition != 0 && row == 0;
    }

    // Draw vertical border and subtract from cols, if necessary.
    if (Tile_p->rightSeparator) {
        ttyr_tty_drawVerticalBorderGlyph(&View_p->Row.Glyphs_p[cols-1]);
        cols--;
    }

    if (topbar) {
        if (Tile_p->type == TTYR_TTY_TILE_TYPE_MACRO) { 
            TTYR_TTY_END(ttyr_tty_drawTopBarRow(
                Tile_p, View_p->Row.Glyphs_p, cols, row, View_p->standardIO
            ))
        } else {
            TTYR_TTY_END(ttyr_tty_drawTopBarRow(
                NULL, View_p->Row.Glyphs_p, cols, row, View_p->standardIO
            ))
        }
    }
 
    if (Tile_p->type == TTYR_TTY_TILE_TYPE_MACRO) { 
        TTYR_TTY_CHECK(ttyr_tty_drawMicroWindow(
            &TTYR_TTY_MACRO_TAB(Tile_p)->MicroWindow, View_p->Row.Glyphs_p, cols, Tile_p->rowSize, row, View_p->standardIO
        ))
    } else if (TTYR_TTY_MICRO_TILE(Tile_p)->Program_p) {
        TTYR_TTY_CHECK(TTYR_TTY_MICRO_TILE(Tile_p)->Program_p->Prototype_p->Callbacks.draw_f(
            TTYR_TTY_MICRO_TILE(Tile_p)->Program_p, View_p->Row.Glyphs_p, cols, Tile_p->rowSize-1, row-1
        ))
    }

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

// REFRESH =========================================================================================

static TTYR_TTY_RESULT ttyr_tty_postProcessRow(
    ttyr_tty_View *View_p, int row)
{
TTYR_TTY_BEGIN()

    ttyr_tty_Row *Row_p = View_p->Grid1_p+row;

    if (View_p->standardIO) {TTYR_TTY_END(TTYR_TTY_SUCCESS)}

    // Post process line.
    for (int i = 0; i < View_p->cols; ++i) {
        ttyr_tty_Glyph *Glyph_p = &Row_p->Glyphs_p[i];
        if (Glyph_p->mark & TTYR_TTY_MARK_LINE_VERTICAL) {
            Glyph_p->codepoint = 'x';
            Glyph_p->Attributes.reverse = NH_FALSE;
            Glyph_p->mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
            if (((Glyph_p+1)->Attributes.reverse && (Glyph_p+1)->mark & TTYR_TTY_MARK_LINE_HORIZONTAL)
            &&  ((Glyph_p-1)->Attributes.reverse && (Glyph_p-1)->mark & TTYR_TTY_MARK_LINE_HORIZONTAL)) {
                Glyph_p->Attributes.reverse = NH_TRUE;
                continue;
            } 
            if ((Glyph_p+1)->mark & TTYR_TTY_MARK_LINE_HORIZONTAL) {
                if ((Glyph_p-1)->mark & TTYR_TTY_MARK_LINE_HORIZONTAL) {
                    if (row > 0 && ((Row_p-1)->Glyphs_p[i].mark & TTYR_TTY_MARK_LINE_VERTICAL)) {
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
            else if (i < View_p->cols-1 && ((Glyph_p-1)->mark & TTYR_TTY_MARK_LINE_HORIZONTAL)) {
                Glyph_p->codepoint = (Glyph_p-1)->Attributes.reverse ? 'e' : 'u';
            }
        }
        if (Glyph_p->mark & TTYR_TTY_MARK_LINE_HORIZONTAL && !Glyph_p->Attributes.reverse) {
            if (row > 0 && ((Row_p-1)->Glyphs_p[i].mark & TTYR_TTY_MARK_LINE_VERTICAL)) {
                Glyph_p->codepoint = 'v';
            }
        }
    }

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

TTYR_TTY_RESULT ttyr_tty_refreshGrid1Row(
    nh_List *Tiles_p, ttyr_tty_View *View_p, int row)
{
TTYR_TTY_BEGIN()

    memset(View_p->Row.Glyphs_p, 0, sizeof(ttyr_tty_Glyph)*View_p->cols);
    int offset = 0;

    for (int col = offset; col < View_p->cols;) {
        for (int tile = 0; tile < Tiles_p->size; ++tile) {

            ttyr_tty_Tile *Tile_p = Tiles_p->pp[tile];
            if (Tile_p->Children.count > 0) {continue;}

            Tile_p->refresh = NH_FALSE;

            if (Tile_p->rowPosition <= row
            &&  Tile_p->rowPosition  + Tile_p->rowSize > row
            &&  Tile_p->colPosition == col-offset)
            {
                TTYR_TTY_CHECK(ttyr_tty_draw(Tile_p, View_p, row))

                for (int i = 0; i < Tile_p->colSize; ++i) {
                    ttyr_tty_Glyph *Glyph_p = View_p->Grid1_p[row].Glyphs_p+col+i;
                    if (memcmp(Glyph_p, View_p->Row.Glyphs_p+i, sizeof(ttyr_tty_Glyph))) {
                        View_p->Grid1_p[row].update_p[col+i] = NH_TRUE;
                    }
                }

                memcpy(View_p->Grid1_p[row].Glyphs_p+col, View_p->Row.Glyphs_p, 
                    sizeof(ttyr_tty_Glyph)*Tile_p->colSize);

                col += Tile_p->colSize;
                break;
            }
        }
    }

    TTYR_TTY_CHECK(ttyr_tty_postProcessRow(View_p, row))

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

TTYR_TTY_RESULT ttyr_tty_refreshGrid1(
    ttyr_tty_TTY *TTY_p) 
{
TTYR_TTY_BEGIN()

    ttyr_tty_View *View_p = TTY_p->Views.pp[0];
    ttyr_tty_Config Config = ttyr_tty_getConfig();
    int offset = 0;

    ttyr_tty_updateTiling(TTY_p->Window_p->RootTile_p, View_p->rows, View_p->cols-offset);
    nh_List Tiles = ttyr_tty_getTiles(TTY_p->Window_p->RootTile_p);

    for (int row = 0; row < View_p->rows; ++row) {
        TTYR_TTY_CHECK(ttyr_tty_refreshGrid1Row(&Tiles, View_p, row))
    }

    nh_core_freeList(&Tiles, NH_FALSE);

    TTYR_TTY_CHECK(ttyr_tty_forwardGrid1(View_p))

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

TTYR_TTY_RESULT ttyr_tty_refreshGrid2(
    ttyr_tty_TTY *TTY_p)
{
TTYR_TTY_BEGIN()

    ttyr_tty_View *View_p = TTY_p->Views.pp[0];

    for (int row = 0; row < View_p->rows; ++row) {
        memset(View_p->Grid2_p[row].Glyphs_p, 0, sizeof(ttyr_tty_Glyph)*View_p->cols);
    }
 
    TTYR_TTY_CHECK(ttyr_tty_drawContextMenuRecursively(TTY_p->Window_p->MouseMenu_p, View_p->Grid2_p))
    TTYR_TTY_CHECK(ttyr_tty_forwardGrid2(View_p))

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

