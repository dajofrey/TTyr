// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Menu.h"
#include "TTY.h"
#include "Macro.h"

#include "../Common/Macros.h"

#include "nh-core/System/Memory.h"
#include "nh-core/System/Thread.h"
#include "nh-encoding/Encodings/UTF32.h"
#include "nh-encoding/Encodings/UTF8.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// DRAW ============================================================================================

static ttyr_tty_Glyph ttyr_tty_getGlyphHelper( 
    NH_API_UTF32 codepoint) 
{ 
    ttyr_tty_Glyph Glyph; 
    memset(&Glyph, 0, sizeof(ttyr_tty_Glyph)); 
    Glyph.codepoint = codepoint; 
    Glyph.mark |= TTYR_TTY_MARK_LINE_HORIZONTAL | TTYR_TTY_MARK_ACCENT; 
    return Glyph; 
} 

TTYR_TTY_RESULT ttyr_tty_drawTabSwitchMenu(
    ttyr_tty_Row *Grid_p)
{
    ttyr_tty_TTY *TTY_p = nh_core_getWorkloadArg();
    ttyr_tty_View *View_p = TTY_p->Views.pp[0];

    nh_api_PixelPosition Position;
    Position.x = TTY_p->Window_p->Tile_p->colPosition+TTY_p->Window_p->Tile_p->colSize/2;
    Position.y = TTY_p->Window_p->Tile_p->rowPosition+TTY_p->Window_p->Tile_p->rowSize/2;

    // Get menu height.
    int height = TTYR_TTY_MACRO_TILE(TTY_p->Window_p->Tile_p)->MacroTabs.size+2;
    if (height <= 0) {return TTYR_TTY_SUCCESS;}

    // Get menu width.
    int width = 0;
    for (int i = 0; i <  TTYR_TTY_MACRO_TILE(TTY_p->Window_p->Tile_p)->MacroTabs.size; ++i) {
        ttyr_tty_Program *Program_p = ttyr_tty_getCurrentProgram(&((ttyr_tty_MacroTab*)((ttyr_tty_MacroTile*)TTY_p->Window_p->Tile_p->p)->MacroTabs.pp[i])->MicroWindow);
        if (!Program_p || !Program_p->Prototype_p) {continue;}
        NH_API_UTF32 title_p[255] = {0};
        Program_p->Prototype_p->Callbacks.getTitle_f(Program_p, title_p, 255);
        int length = 0;
        for (length = 0; length < 255 && title_p[length] != 0; ++length);
        if (width < length) {
            width = length;
        }
    }
    if (width <= 0) {return TTYR_TTY_SUCCESS;}

    width += 4;

    Position.x -= width/2;
    Position.y -= TTY_p->Windows.size/2;

    TTY_p->TabSwitchMenu.Position = Position;
    TTY_p->TabSwitchMenu.width = width;
    TTY_p->TabSwitchMenu.height = height;

    NH_API_UTF32 title_p[] = {'T', 'i', 'l', 'e', ' ', 'T', 'a', 'b', 's'};
    int offset = (width-sizeof(title_p)/sizeof(title_p[0]))/2;
    for (int i = 0; i < sizeof(title_p)/sizeof(title_p[0]); ++i) {
        Grid_p[Position.y].Glyphs_p[Position.x+i+offset] = ttyr_tty_getGlyphHelper(title_p[i]);
    }

    // Draw item names.
    NH_API_UTF32 defaultName_p[] = {'N', '/', 'A', 0};
    for (int row = Position.y+2, i = 0; row < Position.y+height; ++row, ++i) {
        ttyr_tty_Program *Program_p = ttyr_tty_getCurrentProgram(&((ttyr_tty_MacroTab*)((ttyr_tty_MacroTile*)TTY_p->Window_p->Tile_p->p)->MacroTabs.pp[i])->MicroWindow);
        if (!Program_p || !Program_p->Prototype_p) {
            for (int j = 0, k = 0; defaultName_p[j] != 0; ++j, k++) {
                Grid_p[row].Glyphs_p[Position.x+k+3] = ttyr_tty_getGlyphHelper(defaultName_p[j]);
            }
            continue;
        }
        NH_API_UTF32 title_p[255] = {0};
        Program_p->Prototype_p->Callbacks.getTitle_f(Program_p, title_p, 255);
        if (title_p[0] == 0) {
            for (int j = 0, k = 0; defaultName_p[j] != 0; ++j, k++) {
                Grid_p[row].Glyphs_p[Position.x+k+3] = ttyr_tty_getGlyphHelper(defaultName_p[j]);
            }
        }
        for (int j = 0, k = 0; title_p[j] != 0; ++j, k++) {
            Grid_p[row].Glyphs_p[Position.x+k+3] = ttyr_tty_getGlyphHelper(title_p[j]);
        }
        if (((ttyr_tty_MacroTile*)TTY_p->Window_p->Tile_p->p)->current == i) {
            Grid_p[row].Glyphs_p[Position.x+1] = ttyr_tty_getGlyphHelper(0x25cf);
        }
    }

    // Add basic outer lines.
    for (int row = Position.y, i = 0; row < Position.y+height; ++row, ++i) {
        Grid_p[row].Glyphs_p[Position.x-1].codepoint = 'x';
        Grid_p[row].Glyphs_p[Position.x-1].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
        Grid_p[row].Glyphs_p[Position.x+width].codepoint = 'x';
        Grid_p[row].Glyphs_p[Position.x+width].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
    }

    for (int col = Position.x-1, j = 0; col < Position.x+width+1; ++col, ++j) {
        Grid_p[Position.y-1].Glyphs_p[col].codepoint = 'q';
        Grid_p[Position.y-1].Glyphs_p[col].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
        Grid_p[Position.y+height].Glyphs_p[col].codepoint = 'q';
        Grid_p[Position.y+height].Glyphs_p[col].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
    }

    Grid_p[Position.y-1].Glyphs_p[Position.x-1].codepoint = 'l';
    Grid_p[Position.y-1].Glyphs_p[Position.x-1].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
    Grid_p[Position.y-1].Glyphs_p[Position.x+width].codepoint = 'k';
    Grid_p[Position.y-1].Glyphs_p[Position.x+width].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
    Grid_p[Position.y+height].Glyphs_p[Position.x-1].codepoint = 'm';
    Grid_p[Position.y+height].Glyphs_p[Position.x-1].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
    Grid_p[Position.y+height].Glyphs_p[Position.x+width].codepoint = 'j';
    Grid_p[Position.y+height].Glyphs_p[Position.x+width].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;

    for (int col = Position.x-1, j = 0; col < Position.x+width+1; ++col, ++j) {
        Grid_p[Position.y+1].Glyphs_p[col].codepoint = 'q';
        Grid_p[Position.y+1].Glyphs_p[col].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
    }

    Grid_p[Position.y+1].Glyphs_p[Position.x-1].codepoint = 't';
    Grid_p[Position.y+1].Glyphs_p[Position.x-1].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
    Grid_p[Position.y+1].Glyphs_p[Position.x+width].codepoint = 'u';
    Grid_p[Position.y+1].Glyphs_p[Position.x+width].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
 
    return TTYR_TTY_SUCCESS;
}

TTYR_TTY_RESULT ttyr_tty_drawWindowSwitchMenu(
    ttyr_tty_Row *Grid_p)
{
    ttyr_tty_TTY *TTY_p = nh_core_getWorkloadArg();
    ttyr_tty_View *View_p = TTY_p->Views.pp[0];

    nh_api_PixelPosition Position;
    Position.x = View_p->cols/2;
    Position.y = View_p->rows/2;

    // Get menu height.
    int height = TTY_p->Windows.size+2;
    if (height <= 0) {return TTYR_TTY_SUCCESS;}

    // Get menu width.
    int width = 0;
    for (int i = 0; i < TTY_p->Windows.size; ++i) {
        ttyr_tty_Program *Program_p = ttyr_tty_getCurrentProgram(&TTYR_TTY_MACRO_TAB(((ttyr_tty_MacroWindow*)TTY_p->Windows.pp[i])->Tile_p)->MicroWindow);
        if (!Program_p || !Program_p->Prototype_p) {continue;}
        NH_API_UTF32 title_p[255] = {0};
        Program_p->Prototype_p->Callbacks.getTitle_f(Program_p, title_p, 255);
        int length = 0;
        for (length = 0; length < 255 && title_p[length] != 0; ++length);
        if (width < length) {
            width = length;
        }
    }
    if (width <= 0) {return TTYR_TTY_SUCCESS;}

    width += 4;

    Position.x -= width/2;
    Position.y -= TTY_p->Windows.size/2;

    TTY_p->WindowSwitchMenu.Position = Position;
    TTY_p->WindowSwitchMenu.width = width;
    TTY_p->WindowSwitchMenu.height = height;

    NH_API_UTF32 title_p[] = {'T', 'i', 'l', 'e', ' ', 'W', 'i', 'n', 'd', 'o', 'w', 's'};
    int offset = (width-sizeof(title_p)/sizeof(title_p[0]))/2;
    for (int i = 0; i < sizeof(title_p)/sizeof(title_p[0]); ++i) {
        Grid_p[Position.y].Glyphs_p[Position.x+i+offset] = ttyr_tty_getGlyphHelper(title_p[i]);
    }

    // Draw item names.
    NH_API_UTF32 defaultName_p[] = {'N', '/', 'A', 0};
    for (int row = Position.y+2, i = 0; row < Position.y+height; ++row, ++i) {
        ttyr_tty_Program *Program_p = ttyr_tty_getCurrentProgram(&TTYR_TTY_MACRO_TAB(((ttyr_tty_MacroWindow*)TTY_p->Windows.pp[i])->Tile_p)->MicroWindow);
        if (!Program_p || !Program_p->Prototype_p) {
            for (int j = 0, k = 0; defaultName_p[j] != 0; ++j, k++) {
                Grid_p[row].Glyphs_p[Position.x+k+3] = ttyr_tty_getGlyphHelper(defaultName_p[j]);
            }
            continue;
        }
        NH_API_UTF32 title_p[255] = {0};
        Program_p->Prototype_p->Callbacks.getTitle_f(Program_p, title_p, 255);
        if (title_p[0] == 0) {
            for (int j = 0, k = 0; defaultName_p[j] != 0; ++j, k++) {
                Grid_p[row].Glyphs_p[Position.x+k+3] = ttyr_tty_getGlyphHelper(defaultName_p[j]);
            }
        }
        for (int j = 0, k = 0; title_p[j] != 0; ++j, k++) {
            Grid_p[row].Glyphs_p[Position.x+k+3] = ttyr_tty_getGlyphHelper(title_p[j]);
        }
        if (TTY_p->Window_p == TTY_p->Windows.pp[i]) {
            Grid_p[row].Glyphs_p[Position.x+1] = ttyr_tty_getGlyphHelper(0x25cf);
        }
    }

    // Add basic outer lines.
    for (int row = Position.y, i = 0; row < Position.y+height; ++row, ++i) {
        Grid_p[row].Glyphs_p[Position.x-1].codepoint = 'x';
        Grid_p[row].Glyphs_p[Position.x-1].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
        Grid_p[row].Glyphs_p[Position.x+width].codepoint = 'x';
        Grid_p[row].Glyphs_p[Position.x+width].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
    }

    for (int col = Position.x-1, j = 0; col < Position.x+width+1; ++col, ++j) {
        Grid_p[Position.y-1].Glyphs_p[col].codepoint = 'q';
        Grid_p[Position.y-1].Glyphs_p[col].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
        Grid_p[Position.y+height].Glyphs_p[col].codepoint = 'q';
        Grid_p[Position.y+height].Glyphs_p[col].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
    }

    Grid_p[Position.y-1].Glyphs_p[Position.x-1].codepoint = 'l';
    Grid_p[Position.y-1].Glyphs_p[Position.x-1].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
    Grid_p[Position.y-1].Glyphs_p[Position.x+width].codepoint = 'k';
    Grid_p[Position.y-1].Glyphs_p[Position.x+width].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
    Grid_p[Position.y+height].Glyphs_p[Position.x-1].codepoint = 'm';
    Grid_p[Position.y+height].Glyphs_p[Position.x-1].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
    Grid_p[Position.y+height].Glyphs_p[Position.x+width].codepoint = 'j';
    Grid_p[Position.y+height].Glyphs_p[Position.x+width].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;

    for (int col = Position.x-1, j = 0; col < Position.x+width+1; ++col, ++j) {
        Grid_p[Position.y+1].Glyphs_p[col].codepoint = 'q';
        Grid_p[Position.y+1].Glyphs_p[col].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
    }

    Grid_p[Position.y+1].Glyphs_p[Position.x-1].codepoint = 't';
    Grid_p[Position.y+1].Glyphs_p[Position.x-1].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
    Grid_p[Position.y+1].Glyphs_p[Position.x+width].codepoint = 'u';
    Grid_p[Position.y+1].Glyphs_p[Position.x+width].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
 
    return TTYR_TTY_SUCCESS;
}
