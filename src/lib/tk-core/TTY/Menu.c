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

static tk_core_Glyph tk_core_getGlyphHelper( 
    NH_API_UTF32 codepoint) 
{ 
    tk_core_Glyph Glyph; 
    memset(&Glyph, 0, sizeof(tk_core_Glyph)); 
    Glyph.codepoint = codepoint; 
    Glyph.mark |= TTYR_CORE_MARK_LINE_HORIZONTAL | TTYR_CORE_MARK_ACCENT; 
    return Glyph; 
} 

TTYR_CORE_RESULT tk_core_drawMicroWindowMenu(
    tk_core_Row *Grid_p)
{
    tk_core_TTY *TTY_p = nh_core_getWorkloadArg();
    tk_core_View *View_p = TTY_p->Views.pp[0];

    nh_api_PixelPosition Position;
    Position.x = TTY_p->Window_p->Tile_p->colPosition+TTY_p->Window_p->Tile_p->colSize/2;
    Position.y = TTY_p->Window_p->Tile_p->rowPosition+TTY_p->Window_p->Tile_p->rowSize/2;

    if (TTY_p->Config.Titlebar.on) {
        Position.x+=2;
    }

    // Get menu height.
    int height = TTYR_CORE_MACRO_TILE(TTY_p->Window_p->Tile_p)->MacroTabs.size;
    if (height <= 0) {return TTYR_CORE_SUCCESS;}

    // Get menu width.
    int width = 0;
    for (int i = 0; i <  TTYR_CORE_MACRO_TILE(TTY_p->Window_p->Tile_p)->MacroTabs.size; ++i) {
        tk_core_Program *Program_p = tk_core_getCurrentProgram(&((tk_core_MacroTab*)((tk_core_MacroTile*)TTY_p->Window_p->Tile_p->p)->MacroTabs.pp[i])->MicroWindow);
        if (!Program_p || !Program_p->Prototype_p) {continue;}
        NH_API_UTF32 title_p[255] = {0};
        Program_p->Prototype_p->Callbacks.getTitle_f(Program_p, title_p, 255);
        int length = 0;
        for (length = 0; length < 255 && title_p[length] != 0; ++length);
        if (width < length) {
            width = length;
        }
    }
    if (width <= 0) {return TTYR_CORE_SUCCESS;}

    width += 4;

    Position.x -= width/2;
    Position.y -= TTY_p->Windows.size/2;

    TTY_p->TabSwitchMenu.Position = Position;
    TTY_p->TabSwitchMenu.width = width;
    TTY_p->TabSwitchMenu.height = height;

//    NH_API_UTF32 title_p[] = {'M', 'i', 'c', 'r', 'o', ' ', 'W', 'i', 'n', 'd', 'o', 'w', 's'};
//    int offset = (width-sizeof(title_p)/sizeof(title_p[0]))/2;
//    for (int i = 0; i < sizeof(title_p)/sizeof(title_p[0]); ++i) {
//        Grid_p[Position.y].Glyphs_p[Position.x+i+offset] = tk_core_getGlyphHelper(title_p[i]);
//    }

    // Draw item names.
    NH_API_UTF32 defaultName_p[] = {'N', '/', 'A', 0};
    for (int row = Position.y, i = 0; row < Position.y+height; ++row, ++i) {
        tk_core_Program *Program_p = tk_core_getCurrentProgram(&((tk_core_MacroTab*)((tk_core_MacroTile*)TTY_p->Window_p->Tile_p->p)->MacroTabs.pp[i])->MicroWindow);
        if (!Program_p || !Program_p->Prototype_p) {
            for (int j = 0, k = 0; defaultName_p[j] != 0; ++j, k++) {
                Grid_p[row].Glyphs_p[Position.x+k+3] = tk_core_getGlyphHelper(defaultName_p[j]);
            }
            continue;
        }
        NH_API_UTF32 title_p[255] = {0};
        Program_p->Prototype_p->Callbacks.getTitle_f(Program_p, title_p, 255);
        if (title_p[0] == 0) {
            for (int j = 0, k = 0; defaultName_p[j] != 0; ++j, k++) {
                Grid_p[row].Glyphs_p[Position.x+k+3] = tk_core_getGlyphHelper(defaultName_p[j]);
            }
        }
        for (int j = 0, k = 0; title_p[j] != 0; ++j, k++) {
            Grid_p[row].Glyphs_p[Position.x+k+3] = tk_core_getGlyphHelper(title_p[j]);
        }
        if (((tk_core_MacroTile*)TTY_p->Window_p->Tile_p->p)->current == i) {
            Grid_p[row].Glyphs_p[Position.x+1] = tk_core_getGlyphHelper(0x25cf);
        }
    }

    // Add basic outer lines.
    for (int row = Position.y, i = 0; row < Position.y+height; ++row, ++i) {
        Grid_p[row].Glyphs_p[Position.x-1].codepoint = 'x';
        Grid_p[row].Glyphs_p[Position.x-1].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
        Grid_p[row].Glyphs_p[Position.x+width].codepoint = 'x';
        Grid_p[row].Glyphs_p[Position.x+width].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
    }

    for (int col = Position.x-1, j = 0; col < Position.x+width+1; ++col, ++j) {
        Grid_p[Position.y-1].Glyphs_p[col].codepoint = 'q';
        Grid_p[Position.y-1].Glyphs_p[col].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
        Grid_p[Position.y+height].Glyphs_p[col].codepoint = 'q';
        Grid_p[Position.y+height].Glyphs_p[col].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
    }

    Grid_p[Position.y-1].Glyphs_p[Position.x-1].codepoint = 'l';
    Grid_p[Position.y-1].Glyphs_p[Position.x-1].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
    Grid_p[Position.y-1].Glyphs_p[Position.x+width].codepoint = 'k';
    Grid_p[Position.y-1].Glyphs_p[Position.x+width].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
    Grid_p[Position.y+height].Glyphs_p[Position.x-1].codepoint = 'm';
    Grid_p[Position.y+height].Glyphs_p[Position.x-1].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
    Grid_p[Position.y+height].Glyphs_p[Position.x+width].codepoint = 'j';
    Grid_p[Position.y+height].Glyphs_p[Position.x+width].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;

//    for (int col = Position.x+1, j = 0; col < Position.x+width-1; ++col, ++j) {
//        Grid_p[Position.y+1].Glyphs_p[col].codepoint = 'q';
//        Grid_p[Position.y+1].Glyphs_p[col].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
//    }

//    Grid_p[Position.y+1].Glyphs_p[Position.x-1].codepoint = 't';
//    Grid_p[Position.y+1].Glyphs_p[Position.x-1].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
//    Grid_p[Position.y+1].Glyphs_p[Position.x+width].codepoint = 'u';
//    Grid_p[Position.y+1].Glyphs_p[Position.x+width].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
 
    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT tk_core_drawPrompt(
    tk_core_Row *Grid_p)
{
    tk_core_TTY *TTY_p = nh_core_getWorkloadArg();
    tk_core_View *View_p = TTY_p->Views.pp[0];

    nh_api_PixelPosition Position;
    Position.x = View_p->cols/2;
    Position.y = View_p->rows/2;

    // Get menu height.
    int height = 1;
    if (height <= 0) {return TTYR_CORE_SUCCESS;}

    // Get menu width.
    int width = 9;
//    for (int i = 0; i < TTY_p->Windows.size; ++i) {
//        tk_core_Program *Program_p = tk_core_getCurrentProgram(&TTYR_CORE_MACRO_TAB(((tk_core_MacroWindow*)TTY_p->Windows.pp[i])->Tile_p)->MicroWindow);
//        if (!Program_p || !Program_p->Prototype_p) {continue;}
//        NH_API_UTF32 title_p[255] = {0};
//        Program_p->Prototype_p->Callbacks.getTitle_f(Program_p, title_p, 255);
//        int length = 0;
//        for (length = 0; length < 255 && title_p[length] != 0; ++length);
//        if (width < length) {
//            width = length;
//        }
//    }
//    if (width <= 0) {return TTYR_CORE_SUCCESS;}
//
//    width += 4;

    Position.x -= width/2;

    TTY_p->WindowSwitchMenu.Position = Position;
    TTY_p->WindowSwitchMenu.width = width;
    TTY_p->WindowSwitchMenu.height = height;

    NH_API_UTF32 title_p[] = {'>', ' ', 'i', 'n', 'v', 'e', 'r', 't', '_'};
    int offset = (width-sizeof(title_p)/sizeof(title_p[0]))/2;
    for (int i = 0; i < sizeof(title_p)/sizeof(title_p[0]); ++i) {
        Grid_p[Position.y].Glyphs_p[Position.x+i+offset] = tk_core_getGlyphHelper(title_p[i]);
    }

//    // Draw item names.
//    NH_API_UTF32 defaultName_p[] = {'N', '/', 'A', 0};
//    for (int row = Position.y+2, i = 0; row < Position.y+height; ++row, ++i) {
//        tk_core_Program *Program_p = tk_core_getCurrentProgram(&TTYR_CORE_MACRO_TAB(((tk_core_MacroWindow*)TTY_p->Windows.pp[i])->Tile_p)->MicroWindow);
//        if (!Program_p || !Program_p->Prototype_p) {
//            for (int j = 0, k = 0; defaultName_p[j] != 0; ++j, k++) {
//                Grid_p[row].Glyphs_p[Position.x+k+3] = tk_core_getGlyphHelper(defaultName_p[j]);
//            }
//            continue;
//        }
//        NH_API_UTF32 title_p[255] = {0};
//        Program_p->Prototype_p->Callbacks.getTitle_f(Program_p, title_p, 255);
//        if (title_p[0] == 0) {
//            for (int j = 0, k = 0; defaultName_p[j] != 0; ++j, k++) {
//                Grid_p[row].Glyphs_p[Position.x+k+3] = tk_core_getGlyphHelper(defaultName_p[j]);
//            }
//        }
//        for (int j = 0, k = 0; title_p[j] != 0; ++j, k++) {
//            Grid_p[row].Glyphs_p[Position.x+k+3] = tk_core_getGlyphHelper(title_p[j]);
//        }
//        if (TTY_p->Window_p == TTY_p->Windows.pp[i]) {
//            Grid_p[row].Glyphs_p[Position.x+1] = tk_core_getGlyphHelper(0x25cf);
//        }
//    }

    // Add basic outer lines.
    for (int row = Position.y, i = 0; row < Position.y+height; ++row, ++i) {
        Grid_p[row].Glyphs_p[Position.x-1].codepoint = 'x';
        Grid_p[row].Glyphs_p[Position.x-1].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
        Grid_p[row].Glyphs_p[Position.x+width].codepoint = 'x';
        Grid_p[row].Glyphs_p[Position.x+width].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
    }

    for (int col = Position.x-1, j = 0; col < Position.x+width+1; ++col, ++j) {
        Grid_p[Position.y-1].Glyphs_p[col].codepoint = 'q';
        Grid_p[Position.y-1].Glyphs_p[col].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
        Grid_p[Position.y+height].Glyphs_p[col].codepoint = 'q';
        Grid_p[Position.y+height].Glyphs_p[col].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
    }

    Grid_p[Position.y-1].Glyphs_p[Position.x-1].codepoint = 'l';
    Grid_p[Position.y-1].Glyphs_p[Position.x-1].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
    Grid_p[Position.y-1].Glyphs_p[Position.x+width].codepoint = 'k';
    Grid_p[Position.y-1].Glyphs_p[Position.x+width].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
    Grid_p[Position.y+height].Glyphs_p[Position.x-1].codepoint = 'm';
    Grid_p[Position.y+height].Glyphs_p[Position.x-1].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
    Grid_p[Position.y+height].Glyphs_p[Position.x+width].codepoint = 'j';
    Grid_p[Position.y+height].Glyphs_p[Position.x+width].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;

//    for (int col = Position.x-1, j = 0; col < Position.x+width+1; ++col, ++j) {
//        Grid_p[Position.y+1].Glyphs_p[col].codepoint = 'q';
//        Grid_p[Position.y+1].Glyphs_p[col].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
//    }
//
//    Grid_p[Position.y+1].Glyphs_p[Position.x-1].codepoint = 't';
//    Grid_p[Position.y+1].Glyphs_p[Position.x-1].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
//    Grid_p[Position.y+1].Glyphs_p[Position.x+width].codepoint = 'u';
//    Grid_p[Position.y+1].Glyphs_p[Position.x+width].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
 
    return TTYR_CORE_SUCCESS;
}
