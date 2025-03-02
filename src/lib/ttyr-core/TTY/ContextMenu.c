// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "ContextMenu.h"
#include "TTY.h"

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

#define COMMAND_NAME 'C', 'o', 'm', 'm', 'a', 'n', 'd',
#define PROGRAM_NAME 'P', 'r', 'o', 'g', 'r', 'a', 'm',

#define ARROW_RIGHT 0x2192
#define ARROW_LEFT 0x2190
#define ARROW_DOWN 0x2191
#define ARROW_UP 0x2193

// ACTION ==========================================================================================

static bool ttyr_core_pathMatchesContextMenu(
    ttyr_core_ContextMenu *Menu_p, const NH_API_UTF32 **pp, int count)
{
    for (int i = count-1; i >= 0; --i) {
        if (!Menu_p || Menu_p->Name.length == 0) {
            return false;
        }
        for (int j = 0; j < Menu_p->Name.length; ++j) {
            if (pp[i][j] == 0 || pp[i][j] != Menu_p->Name.p[j]) {
                return false;
                break;
            }
        }
        Menu_p = Menu_p->Parent_p;
    }
    return true;
}

static int ttyr_core_isContextMenuTiling(
    ttyr_core_ContextMenu *Menu_p)
{
    static const NH_API_UTF32 append_p[] = {'A', 'p', 'p', 'e', 'n', 'd', 0};
    static const NH_API_UTF32 split_p[] = {'S', 'p', 'l', 'i', 't', 0};
    static const NH_API_UTF32 window_p[] = {'W', 'i', 'n', 'd', 'o', 'w', 0};
    static const NH_API_UTF32 tab_p[] = {'T', 'a', 'b', 0};

    static const NH_API_UTF32 *paths_ppp[][4] = {
        {append_p, tab_p},
        {append_p, window_p},
        {split_p, tab_p},
        {split_p, window_p},
    };

    for (int i = 0; i < 4; ++i) {
        if (ttyr_core_pathMatchesContextMenu(Menu_p, paths_ppp[i], 2)) {
            return i;
        }
    }

    return -1;
}

static int ttyr_core_isContextMenuWindowOrTabSelect(
    ttyr_core_ContextMenu *Menu_p)
{
    static const NH_API_UTF32 window_p[] = {'W', 'i', 'n', 'd', 'o', 'w', 0};
    static const NH_API_UTF32 tab_p[] = {'T', 'a', 'b', 0};
    static const NH_API_UTF32 p1[] = {'1', 1, ' ', 0};
    static const NH_API_UTF32 p2[] = {'2', 1, ' ', 0};
    static const NH_API_UTF32 p3[] = {'3', 1, ' ', 0};
    static const NH_API_UTF32 p4[] = {'4', 1, ' ', 0};
    static const NH_API_UTF32 p5[] = {'5', 1, ' ', 0};
    static const NH_API_UTF32 p6[] = {'6', 1, ' ', 0};
    static const NH_API_UTF32 p7[] = {'7', 1, ' ', 0};
    static const NH_API_UTF32 p8[] = {'8', 1, ' ', 0};
    static const NH_API_UTF32 p9[] = {'9', 1, ' ', 0};
 
    static const NH_API_UTF32 *paths_ppp[][18] = {
        {window_p, p1},
        {window_p, p2},
        {window_p, p3},
        {window_p, p4},
        {window_p, p5},
        {window_p, p6},
        {window_p, p7},
        {window_p, p8},
        {window_p, p9},
        {tab_p, p1},
        {tab_p, p2},
        {tab_p, p3},
        {tab_p, p4},
        {tab_p, p5},
        {tab_p, p6},
        {tab_p, p7},
        {tab_p, p8},
        {tab_p, p9},
    };

    for (int i = 0; i < 18; ++i) {
        if (ttyr_core_pathMatchesContextMenu(Menu_p, paths_ppp[i], 2)) {
            return i;
        }
    }

    return -1;
}

static int ttyr_core_handleContextMenuTiling(
    int action, ttyr_core_Tile *Tile_p, int cCol, int cRow)
{
    nh_api_KeyboardEvent Event;
    ttyr_core_MacroWindow *Window_p = ((ttyr_core_TTY*)nh_core_getWorkloadArg())->Window_p;

    int direction = 0;
    
    float col = (((float)cCol)/((float)Tile_p->colSize))-0.5f;
    bool right = col > 0.0f;
    col = fabs(col);

    float row = (((float)cRow)/((float)Tile_p->rowSize))-0.5f;
    bool bottom = row > 0.0f;
    row = fabs(row);
   
    if (col > row) {
        direction = right ? 1 : 3;
    } else {
        direction = bottom ? 2 : 0;
    }

    switch (action)
    {
        // Handle tab append.
        case 0 :
            Event.trigger = NH_API_TRIGGER_PRESS;
            Event.codepoint = TTYR_CORE_TILING_KEY;
            Event.state |= NH_API_MODIFIER_CONTROL;
            ttyr_core_handleTilingInput(Window_p, Event); 

            if (direction == 0) {
                Event.codepoint = TTYR_CORE_INSERT_TILE_TOP_KEY;
            } else if (direction == 1) {
                Event.codepoint = TTYR_CORE_INSERT_TILE_RIGHT_KEY;
            } else if (direction == 2) {
                Event.codepoint = TTYR_CORE_INSERT_TILE_BOTTOM_KEY;
            } else if (direction == 3) {
                Event.codepoint = TTYR_CORE_INSERT_TILE_LEFT_KEY;
            }
 
            Event.state = 0;
            ttyr_core_handleTilingInput(Window_p, Event);
            break;

        // Handle tab split.
        case 2 :
            Event.trigger = NH_API_TRIGGER_PRESS;
            Event.codepoint = TTYR_CORE_TILING_KEY;
            Event.state |= NH_API_MODIFIER_CONTROL;
            ttyr_core_handleTilingInput(Window_p, Event); 

            Event.state = 0;
            Event.codepoint = TTYR_CORE_SPLIT_KEY;
            ttyr_core_handleTilingInput(Window_p, Event); 

            if (direction == 0) {
                Event.codepoint = TTYR_CORE_INSERT_TILE_TOP_KEY;
            } else if (direction == 1) {
                Event.codepoint = TTYR_CORE_INSERT_TILE_RIGHT_KEY;
            } else if (direction == 2) {
                Event.codepoint = TTYR_CORE_INSERT_TILE_BOTTOM_KEY;
            } else if (direction == 3) {
                Event.codepoint = TTYR_CORE_INSERT_TILE_LEFT_KEY;
            }
 
            ttyr_core_handleTilingInput(Window_p, Event);
            break;
 
        // Handle window append.
        case 1 :
            Event.trigger = NH_API_TRIGGER_PRESS;
            Event.codepoint = TTYR_CORE_TILING_KEY;
            Event.state |= NH_API_MODIFIER_CONTROL;
            ttyr_core_handleTilingInput(Window_p, Event); 
            ttyr_core_handleTilingInput(Window_p, Event); 

            if (direction == 0) {
                Event.codepoint = TTYR_CORE_INSERT_TILE_TOP_KEY;
            } else if (direction == 1) {
                Event.codepoint = TTYR_CORE_INSERT_TILE_RIGHT_KEY;
            } else if (direction == 2) {
                Event.codepoint = TTYR_CORE_INSERT_TILE_BOTTOM_KEY;
            } else if (direction == 3) {
                Event.codepoint = TTYR_CORE_INSERT_TILE_LEFT_KEY;
            }

            Event.state = 0;
            ttyr_core_handleTilingInput(Window_p, Event);
            break;

        // Handle window split.
        case 3 :
            Event.trigger = NH_API_TRIGGER_PRESS;
            Event.codepoint = TTYR_CORE_TILING_KEY;
            Event.state |= NH_API_MODIFIER_CONTROL;
            ttyr_core_handleTilingInput(Window_p, Event); 
            ttyr_core_handleTilingInput(Window_p, Event); 

            Event.codepoint = TTYR_CORE_SPLIT_KEY;
            Event.state = 0;
            ttyr_core_handleTilingInput(Window_p, Event); 

            if (direction == 0) {
                Event.codepoint = TTYR_CORE_INSERT_TILE_TOP_KEY;
            } else if (direction == 1) {
                Event.codepoint = TTYR_CORE_INSERT_TILE_RIGHT_KEY;
            } else if (direction == 2) {
                Event.codepoint = TTYR_CORE_INSERT_TILE_BOTTOM_KEY;
            } else if (direction == 3) {
                Event.codepoint = TTYR_CORE_INSERT_TILE_LEFT_KEY;
            }
 
            ttyr_core_handleTilingInput(Window_p, Event);
            break;
    }

    return -1;
}

// CREATE/FREE =====================================================================================

static ttyr_core_ContextMenu *ttyr_core_parseContextMenu(
    NH_API_UTF32 **menu_pp, ttyr_core_ContextMenu *Parent_p)
{
    ttyr_core_ContextMenu *Menu_p = nh_core_allocate(sizeof(ttyr_core_ContextMenu));
    TTYR_CHECK_MEM_2(NULL, Menu_p)

    Menu_p->Parent_p = Parent_p;
    Menu_p->active = false;
    Menu_p->hit = false;
    Menu_p->Name = nh_encoding_initUTF32(16); 
    Menu_p->Position.x = 0;
    Menu_p->Position.y = 0;
    Menu_p->Items = nh_core_initList(8);

    bool curly = false;

    while (**menu_pp != 0) {
        if (**menu_pp == '{' || (**menu_pp == ',' && curly)) {
            (*menu_pp)++;
            ttyr_core_ContextMenu *Child_p = ttyr_core_parseContextMenu(menu_pp, Menu_p);
            TTYR_CHECK_NULL_2(NULL, Child_p)
            nh_core_appendToList(&Menu_p->Items, Child_p);
            curly = true;
            continue;
        }
        else if (**menu_pp == '}') {
            if (curly) {
                curly = false;
                (*menu_pp)++;
                continue;
            }
            return Menu_p;
        }
        else if (**menu_pp == ',') {
            return Menu_p;
        }
        else {
            nh_encoding_appendUTF32Codepoint(&Menu_p->Name, **menu_pp);
            (*menu_pp)++;
        }
    }

    return Menu_p;
}

void ttyr_core_freeContextMenu(
    ttyr_core_ContextMenu *Menu_p)
{
    for (int i = 0; i < Menu_p->Items.size; ++i) {
        ttyr_core_freeContextMenu(Menu_p->Items.pp[i]);
    }
    nh_core_freeList(&Menu_p->Items, false);
    nh_encoding_freeUTF32(&Menu_p->Name);
    nh_core_free(Menu_p);
    return;
}

// POSITION ========================================================================================

static void ttyr_core_computeContextMenuPosition(
    ttyr_core_ContextMenu *Menu_p, int x, int y, int maxX, int maxY)
{
    // Get menu width.
    int width = 0;
    for (int i = 0; i < Menu_p->Items.size; ++i) {
        nh_encoding_UTF32String *Name_p = &((ttyr_core_ContextMenu*)Menu_p->Items.pp[i])->Name;
        if (Name_p->length > width) {width = Name_p->length;}
    }

    Menu_p->Position.x = x;
    Menu_p->Position.y = y;

    // Make sure that context menu respects boundaries.
    if (Menu_p->Position.y + Menu_p->Items.size + 1 > maxY) {
        Menu_p->Position.y -= (Menu_p->Position.y + Menu_p->Items.size + 1) - maxY;
    }
    if (Menu_p->Position.y == 0) {
        Menu_p->Position.y = 1;
    }
    if (Menu_p->Position.x == 0) {
        Menu_p->Position.x = 1;
    }
    if (Menu_p->Position.x + width + 1 > maxX) {
        Menu_p->Position.x -= (Menu_p->Position.x + width + 1) - maxX;
        return; // TODO
    }

    for (int i = 0; i < Menu_p->Items.size; ++i) {
        ttyr_core_computeContextMenuPosition(Menu_p->Items.pp[i], x + width + 2, Menu_p->Position.y + i, maxX, maxY);
    }

    return;
}

static bool ttyr_core_compareContextMenuName(
    ttyr_core_ContextMenu *Menu_p, char *name_p)
{
    nh_encoding_UTF8String Name = nh_encoding_encodeUTF8(Menu_p->Name.p, Menu_p->Name.length);
    bool result = !strcmp(Name.p, name_p);
    nh_encoding_freeUTF8(&Name);
    return result;
}

ttyr_core_ContextMenu *ttyr_core_isContextMenuHit(
    ttyr_core_ContextMenu *Menu_p, ttyr_core_ContextMenu *Parent_p, bool recursive, int x, int y)
{
    int x2 = x;
    int y2 = Menu_p->Position.y;

    int width = 0;
    for (int i = 0; Parent_p && i < Parent_p->Items.size; ++i) {
        nh_encoding_UTF32String *Name_p = &((ttyr_core_ContextMenu*)Parent_p->Items.pp[i])->Name;
        if (Name_p->length > width) {width = Name_p->length;}
        if (Parent_p->Items.pp[i] == Menu_p) {y2 = Parent_p->Position.y + i;}
    }

    if (y2 == y && Parent_p && Parent_p->Position.x <= x2 && (Parent_p->Position.x + width) >= x2) {
        ttyr_core_TTY *TTY_p = nh_core_getWorkloadArg();
        int maxCols = ((ttyr_core_View*)TTY_p->Views.pp[0])->cols;
        int width2 = 0;
        for (int i = 0; i < Menu_p->Items.size; ++i) {
            nh_encoding_UTF32String *Name_p = &((ttyr_core_ContextMenu*)Menu_p->Items.pp[i])->Name;
            if (Name_p->length > width2) {width2 = Name_p->length;}
        }
        if ((Menu_p->Position.x + width2 + 2 < maxCols && Parent_p->Position.x + width + 2 < maxCols) || Menu_p->Items.size == 0) {
            return Menu_p;
        }
    }

    for (int i = 0; (Menu_p->active || Menu_p->hit) && i < Menu_p->Items.size && recursive; ++i) {
        ttyr_core_ContextMenu *Result_p = ttyr_core_isContextMenuHit(Menu_p->Items.pp[i], Menu_p, recursive, x, y);
        if (Result_p) {return Result_p;}
    }

    return NULL;
}

void ttyr_core_updateContextMenuHit(
    ttyr_core_ContextMenu *Menu_p, ttyr_core_ContextMenu *Parent_p, int x, int y, bool activate)
{
    bool newHit = !Menu_p->hit;
    Menu_p->hit = ttyr_core_isContextMenuHit(Menu_p, Parent_p, false, x, y) != NULL && Menu_p->Name.length > 0;

    if (Menu_p->hit) {
        Menu_p->active = true;
    }
    if (Menu_p->active && Parent_p) {
        for (int i = 0; i < Parent_p->Items.size; ++i) {
            ((ttyr_core_ContextMenu*)Parent_p->Items.pp[i])->active = Parent_p->Items.pp[i] == Menu_p;
            ((ttyr_core_ContextMenu*)Parent_p->Items.pp[i])->hit = Parent_p->Items.pp[i] == Menu_p;
        }
    }

    if (newHit && Menu_p->hit) {
    }

    for (int i = 0; (Menu_p->active || Menu_p->hit) && i < Menu_p->Items.size; ++i) {
        ttyr_core_updateContextMenuHit(Menu_p->Items.pp[i], Menu_p, x, y, activate);
        if (((ttyr_core_ContextMenu*)Menu_p->Items.pp[i])->hit) {Menu_p->hit = true;}
    }

    return;
}

// MOUSE MENU ======================================================================================
// Mouse menu functions.

ttyr_core_ContextMenu *ttyr_core_createMouseMenu(
    int x, int y)
{
    ttyr_core_TTY *TTY_p = nh_core_getWorkloadArg();
    ttyr_core_Config Config = ttyr_core_getConfig();
    ttyr_core_Program *Program_p = ttyr_core_getCurrentProgram(&TTYR_CORE_MACRO_TAB(TTY_p->Window_p->Tile_p)->MicroWindow);

    nh_encoding_UTF32String Menu = nh_encoding_initUTF32(128);
    nh_encoding_appendUTF32Codepoint(&Menu, '{');

    // Generate command menu.
    if (Program_p && Program_p->Prototype_p && Program_p->Prototype_p->commands > 0) {
        for (int i = 0; i < Program_p->Prototype_p->commands; ++i) {
            NH_API_UTF32 *name_p = Program_p->Prototype_p->commands_pp[i];
            nh_encoding_appendUTF32(&Menu, name_p, nh_encoding_getUTF32Length(name_p)); 
            if (i < Program_p->Prototype_p->commands-1) {nh_encoding_appendUTF32Codepoint(&Menu, ',');}
        }
    }
 
    nh_encoding_appendUTF32Codepoint(&Menu, ',');
    bool border = true;

    if (Config.Menu.program && TTY_p->Prototypes.size > 1) {
        ttyr_core_Program *Program_p = ttyr_core_getCurrentProgram(&TTYR_CORE_MACRO_TAB(TTY_p->Window_p->Tile_p)->MicroWindow);
        NH_API_UTF32 apps_p[] = {PROGRAM_NAME '{'};
        nh_encoding_appendUTF32(&Menu, apps_p, sizeof(apps_p)/sizeof(apps_p[0]));
        int width = 0;
        for (int i = 0; i < TTY_p->Prototypes.size; ++i) {
            if (nh_encoding_getUTF32Length(((ttyr_core_Interface*)TTY_p->Prototypes.pp[i])->name_p) > width) {
                width = nh_encoding_getUTF32Length(((ttyr_core_Interface*)TTY_p->Prototypes.pp[i])->name_p);
            }
        }
        for (int i = 0; i < TTY_p->Prototypes.size; ++i) {
            ttyr_core_Interface *Prototype_p = TTY_p->Prototypes.pp[i];
            nh_encoding_appendUTF32(&Menu, Prototype_p->name_p, nh_encoding_getUTF32Length(Prototype_p->name_p)); 
            for (int j = width - nh_encoding_getUTF32Length(Prototype_p->name_p); j > 0; --j) {
                nh_encoding_appendUTF32Codepoint(&Menu, ' '); 
            }
            nh_encoding_appendUTF32Codepoint(&Menu, 1); 
            nh_encoding_appendUTF32Codepoint(&Menu, Program_p->Prototype_p == Prototype_p ? 0x25cf : ' '); 
            if (i < TTY_p->Prototypes.size-1) {nh_encoding_appendUTF32Codepoint(&Menu, ',');}
        }
        nh_encoding_appendUTF32Codepoint(&Menu, '}');
        nh_encoding_appendUTF32Codepoint(&Menu, ',');
        border = true;
    }

    if (Config.Menu.split) {
        if (border) {
            nh_encoding_appendUTF32Codepoint(&Menu, ',');
        } 
        NH_API_UTF32 tiling_p[] = {
            'S', 'p', 'l', 'i', 't', '{',
                 'W', 'i', 'n', 'd', 'o', 'w', ',',
                 'T', 'a', 'b',
            '}', ',',
        };
        nh_encoding_appendUTF32(&Menu, tiling_p, sizeof(tiling_p)/sizeof(tiling_p[0]));
        border = true;
    }

    if (Config.Menu.append) {
        if (border && !Config.Menu.split) {
            nh_encoding_appendUTF32Codepoint(&Menu, ',');
        } 
        NH_API_UTF32 tiling_p[] = {
            'A', 'p', 'p', 'e', 'n', 'd', '{',
                 'W', 'i', 'n', 'd', 'o', 'w', ',',
                 'T', 'a', 'b',
            '}', ',',
        };
        nh_encoding_appendUTF32(&Menu, tiling_p, sizeof(tiling_p)/sizeof(tiling_p[0]));
        border = true;
    }

//    if (Config.Menu.window) {
//        NH_API_UTF32 tmp1_p[] = {',', 'W', 'i', 'n', 'd', 'o', 'w', '{'};
//        nh_encoding_appendUTF32(&Menu, tmp1_p, 7);
//        for (int i = 0; i < Config.windows; ++i) {
//            NH_API_UTF32 tmp2_p[] = {i + '1', 1, ' '};
//            if (nh_core_getListIndex(&TTY_p->Windows, TTY_p->Window_p) == i) {
//                tmp2_p[2] = 0x2022;
//            }
//            nh_encoding_appendUTF32(&Menu, tmp2_p, 3);
//            if (i < (Config.windows-1)) {
//                nh_encoding_appendUTF32Codepoint(&Menu, ',');
//            } else {
//                nh_encoding_appendUTF32Codepoint(&Menu, '}');
//                nh_encoding_appendUTF32Codepoint(&Menu, ',');
//            }
//        }
//    }
//
//    if (Config.Menu.tab) {
//        NH_API_UTF32 tmp3_p[] = {',', 'T', 'a', 'b', '{'};
//        nh_encoding_appendUTF32(&Menu, Config.Menu.window ? tmp3_p+1 : tmp3_p , Config.Menu.window ? 3 : 4);
//        for (int i = 0; i < Config.tabs; ++i) {
//            NH_API_UTF32 tmp2_p[] = {i + '1', 1, ' '};
//            if (((ttyr_core_MacroTile*)TTY_p->Window_p->Tile_p->p)->current == i) {
//                tmp2_p[2] = 0x2022;
//            }
//            nh_encoding_appendUTF32(&Menu, tmp2_p, 3);
//            if (i < (Config.tabs-1)) {
//                nh_encoding_appendUTF32Codepoint(&Menu, ',');
//            } else {
//                nh_encoding_appendUTF32Codepoint(&Menu, '}');
//                nh_encoding_appendUTF32Codepoint(&Menu, ',');
//            }
//        }
//    }

    if (Config.Menu.close) {
        if (border) {
            nh_encoding_appendUTF32Codepoint(&Menu, ',');
        } 
        NH_API_UTF32 close_p[] = {'C', 'l', 'o', 's', 'e', ','};
        nh_encoding_appendUTF32(&Menu, close_p, sizeof(close_p)/sizeof(close_p[0]));
        border = true;
    }

    if (Config.Menu.debug) {
        if (border) {
            nh_encoding_appendUTF32Codepoint(&Menu, ',');
        } 
 
        NH_API_UTF32 debug_p[] = {'D', 'e', 'b', 'u', 'g', '{', 0};
        nh_encoding_appendUTF32(&Menu, debug_p, sizeof(debug_p)/sizeof(debug_p[0]));
     
        NH_API_UTF32 x_p[16];
        NH_API_UTF32 y_p[16];
    
        int xLength = nh_encoding_integerToUTF32(x, x_p, 64);
        int yLength = nh_encoding_integerToUTF32(y, y_p, 64);
     
        nh_encoding_appendUTF32Codepoint(&Menu, 'x');
        nh_encoding_appendUTF32Codepoint(&Menu, ':');
        for (int i = 0; i < xLength; ++i) {
            nh_encoding_appendUTF32Codepoint(&Menu, x_p[i]);
        }
        nh_encoding_appendUTF32Codepoint(&Menu, ',');
    
        nh_encoding_appendUTF32Codepoint(&Menu, 'y');
        nh_encoding_appendUTF32Codepoint(&Menu, ':');
        for (int i = 0; i < yLength; ++i) {
            nh_encoding_appendUTF32Codepoint(&Menu, y_p[i]);
        }
        nh_encoding_appendUTF32Codepoint(&Menu, ',');
    
        ttyr_core_Glyph Glyph = ((ttyr_core_View*)TTY_p->Views.pp[0])->Grid1_p[y].Glyphs_p[x];
    
        nh_encoding_appendUTF8ToUTF32(&Menu, Glyph.Attributes.bold ? "Attr.bold:1," : "Attr.bold:0,", 12);
        nh_encoding_appendUTF8ToUTF32(&Menu, Glyph.Attributes.faint ? "Attr.faint:1," : "Attr.faint:0,", 13);
        nh_encoding_appendUTF8ToUTF32(&Menu, Glyph.Attributes.italic ? "Attr.italic:1," : "Attr.italic:0,", 14);
        nh_encoding_appendUTF8ToUTF32(&Menu, Glyph.Attributes.underline ? "Attr.underline:1," : "Attr.underline:0,", 17);
        nh_encoding_appendUTF8ToUTF32(&Menu, Glyph.Attributes.blink ? "Attr.blink:1," : "Attr.blink:0,", 13);
        nh_encoding_appendUTF8ToUTF32(&Menu, Glyph.Attributes.reverse ? "Attr.reverse:1," : "Attr.reverse:0,", 15);
        nh_encoding_appendUTF8ToUTF32(&Menu, Glyph.Attributes.invisible ? "Attr.invisible:1," : "Attr.invisible:0,", 17);
        nh_encoding_appendUTF8ToUTF32(&Menu, Glyph.Attributes.struck ? "Attr.struck:1," : "Attr.struck:0,", 14);
        nh_encoding_appendUTF8ToUTF32(&Menu, Glyph.Attributes.wrap ? "Attr.wrap:1," : "Attr.wrap:0,", 12);
        nh_encoding_appendUTF8ToUTF32(&Menu, Glyph.Attributes.wide ? "Attr.wide:1}" : "Attr.wide:0}", 12);
     
        nh_encoding_appendUTF32Codepoint(&Menu, '}');
    }

    NH_API_UTF32 *p = Menu.p;
    if (Menu.length) {
        p[Menu.length-1] = 0; // Remove the last ','
    }

    ttyr_core_ContextMenu *Menu_p = ttyr_core_parseContextMenu(&p, NULL);
    TTYR_CHECK_NULL_2(NULL, Menu_p)

    ttyr_core_computeContextMenuPosition(Menu_p, x, y, ((ttyr_core_View*)TTY_p->Views.pp[0])->cols, ((ttyr_core_View*)TTY_p->Views.pp[0])->rows);

    nh_encoding_freeUTF32(&Menu);

    return Menu_p;
}

TTYR_CORE_RESULT ttyr_core_handleMouseMenuPress(
    ttyr_core_ContextMenu *Root_p, ttyr_core_ContextMenu *Menu_p)
{
    if (!Menu_p->Parent_p || Menu_p->Name.length == 0) {return TTYR_CORE_SUCCESS;}

    ttyr_core_TTY *TTY_p = nh_core_getWorkloadArg();

    // Handle tiling or tabing if necessary.
    int tiling = ttyr_core_isContextMenuTiling(Menu_p);
    if (tiling >= 0) {
        if (tiling == 1 || tiling == 3) { 
            ttyr_core_handleContextMenuTiling(tiling, TTY_p->Window_p->Tile_p, Root_p->cCol, Root_p->cRow);
        } else {
            ttyr_core_handleContextMenuTiling(tiling, TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(TTY_p->Window_p->Tile_p))->Tile_p, Root_p->cCol, Root_p->cRow);
        }
        nh_api_KeyboardEvent Event;
        Event.trigger = NH_API_TRIGGER_PRESS;
        Event.codepoint = 13;
        return ttyr_core_handleTilingInput(TTY_p->Window_p, Event);
    }

    int select = ttyr_core_isContextMenuWindowOrTabSelect(Menu_p);
    if (select >= 0) {
        if (select < 9) {
            TTYR_CHECK_NULL(ttyr_core_insertAndFocusWindow(TTY_p, select))
        } else {
            ((ttyr_core_MacroTile*)TTY_p->Window_p->Tile_p->p)->current = select - 9; 
        }
        return TTYR_CORE_SUCCESS;
    }

    // Handle program switch if necessary.
    NH_API_UTF32 program_p[] = {PROGRAM_NAME 0};
    NH_API_UTF32 close_p[] = {'C', 'l', 'o', 's', 'e', 0};
    if (nh_encoding_compareUTF32(Menu_p->Parent_p->Name.p, program_p)) {
        for (int i = 0; i < Menu_p->Parent_p->Items.size; ++i) {
            ttyr_core_ContextMenu *Prog_p = Menu_p->Parent_p->Items.pp[i];
            if (Prog_p->active) {
                TTYR_CORE_MACRO_TAB(((ttyr_core_TTY*)nh_core_getWorkloadArg())->Window_p->Tile_p)->MicroWindow.current = i;
            } 
        }
    } else if (nh_encoding_compareUTF32(Menu_p->Name.p, close_p)) {
        TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(TTY_p->Window_p->Tile_p))->Tile_p->close = true;
    } else {
        // Handle command execution if necessary.
        for (int i = 0; i < Menu_p->Parent_p->Items.size; ++i) {
            ttyr_core_ContextMenu *Cmd_p = Menu_p->Parent_p->Items.pp[i];
            if (Cmd_p->active) {
                ttyr_core_Program *Program_p = ttyr_core_getCurrentProgram(&TTYR_CORE_MACRO_TAB(TTY_p->Window_p->Tile_p)->MicroWindow);
                if (Program_p->Prototype_p->Callbacks.handleCommand_f) {
                    Program_p->command = i;
                    Program_p->Prototype_p->Callbacks.handleCommand_f(Program_p);
                }
            } 
        }
    }

    return TTYR_CORE_SUCCESS;
}

// DRAW ============================================================================================

TTYR_CORE_RESULT ttyr_core_drawContextMenuRecursively(
    ttyr_core_ContextMenu *Menu_p, ttyr_core_Row *Grid_p)
{
    if (Menu_p == NULL || Menu_p->Items.size == 0 || Menu_p->hit == false && Menu_p->active == false) {
        return TTYR_CORE_SUCCESS;
    }

    int width = 0;
    int height = Menu_p->Items.size;

    // Get menu width.
    for (int i = 0; i < Menu_p->Items.size; ++i) {
        nh_encoding_UTF32String *String_p = &((ttyr_core_ContextMenu*)Menu_p->Items.pp[i])->Name;
        if (String_p->length > width) {width = String_p->length;}
    }

    // Draw item names.
    for (int row = Menu_p->Position.y, i = 0; row < Menu_p->Position.y+height; ++row, ++i) {
        ttyr_core_ContextMenu *Child_p = Menu_p->Items.pp[i];
        if (Child_p->hit) {
            for (int col = Menu_p->Position.x, j = 0; j < width && Child_p->Name.p[j] != 1; ++col, ++j) {
                Grid_p[row].Glyphs_p[col].Attributes.reverse = true;
            }
        }
        for (int col = Menu_p->Position.x, j = 0; col < Menu_p->Position.x+width; ++col, ++j) {
            Grid_p[row].Glyphs_p[col].codepoint = Child_p->Name.length > j ? Child_p->Name.p[j] : ' ';
            // Draw horizontal separator if required.
            if (Child_p->Name.length == 0) {
                Grid_p[row].Glyphs_p[col].codepoint = 'q';
                Grid_p[row].Glyphs_p[col].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
            }
            // Draw vertical separator if required.
            else if (Child_p->Name.p[j] == 1) {
                Grid_p[row].Glyphs_p[col].codepoint = 'x';
                Grid_p[row].Glyphs_p[col].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
            }
        }
    }

    // Add basic outer lines.
    for (int row = Menu_p->Position.y, i = 0; row < Menu_p->Position.y+height; ++row, ++i) {
        Grid_p[row].Glyphs_p[Menu_p->Position.x-1].codepoint = 'x';
        Grid_p[row].Glyphs_p[Menu_p->Position.x-1].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
        Grid_p[row].Glyphs_p[Menu_p->Position.x+width].codepoint = 'x';
        Grid_p[row].Glyphs_p[Menu_p->Position.x+width].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
    }

    for (int col = Menu_p->Position.x-1, j = 0; col < Menu_p->Position.x+width+1; ++col, ++j) {
        Grid_p[Menu_p->Position.y-1].Glyphs_p[col].codepoint = 'q';
        Grid_p[Menu_p->Position.y-1].Glyphs_p[col].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
        Grid_p[Menu_p->Position.y+height].Glyphs_p[col].codepoint = 'q';
        Grid_p[Menu_p->Position.y+height].Glyphs_p[col].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
    }

    Grid_p[Menu_p->Position.y-1].Glyphs_p[Menu_p->Position.x-1].codepoint = 'l';
    Grid_p[Menu_p->Position.y-1].Glyphs_p[Menu_p->Position.x-1].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
    Grid_p[Menu_p->Position.y-1].Glyphs_p[Menu_p->Position.x+width].codepoint = 'k';
    Grid_p[Menu_p->Position.y-1].Glyphs_p[Menu_p->Position.x+width].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
    Grid_p[Menu_p->Position.y+height].Glyphs_p[Menu_p->Position.x-1].codepoint = 'm';
    Grid_p[Menu_p->Position.y+height].Glyphs_p[Menu_p->Position.x-1].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;
    Grid_p[Menu_p->Position.y+height].Glyphs_p[Menu_p->Position.x+width].codepoint = 'j';
    Grid_p[Menu_p->Position.y+height].Glyphs_p[Menu_p->Position.x+width].mark |= TTYR_CORE_MARK_LINE_GRAPHICS;

    // Add horizontal outer-line separator |- piece.
    for (int row = Menu_p->Position.y; row < Menu_p->Position.y+height; ++row) {
        if (Grid_p[row].Glyphs_p[Menu_p->Position.x].codepoint == 'q' 
        &&  Grid_p[row].Glyphs_p[Menu_p->Position.x].mark & TTYR_CORE_MARK_LINE_GRAPHICS) {
            Grid_p[row].Glyphs_p[Menu_p->Position.x-1].codepoint = 't';
        }
    } 

    // Add horizontal outer-line separator -| piece.
    for (int row = Menu_p->Position.y; row < Menu_p->Position.y+height; ++row) {
        if (Grid_p[row].Glyphs_p[Menu_p->Position.x+width-1].codepoint == 'q' 
        &&  Grid_p[row].Glyphs_p[Menu_p->Position.x+width-1].mark & TTYR_CORE_MARK_LINE_GRAPHICS) {
            Grid_p[row].Glyphs_p[Menu_p->Position.x+width].codepoint = 'u';
        }
    }
 
    // Add vertical outer-line separator T piece.
    for (int col = Menu_p->Position.x; col < Menu_p->Position.x+width; ++col) {
        if (Grid_p[Menu_p->Position.y].Glyphs_p[col].codepoint == 'x' 
        &&  Grid_p[Menu_p->Position.y].Glyphs_p[col].mark & TTYR_CORE_MARK_LINE_GRAPHICS) {
            Grid_p[Menu_p->Position.y-1].Glyphs_p[col].codepoint = 'w';
        }
    }
 
    // Add vertical outer-line separator bottom piece.
    for (int col = Menu_p->Position.x; col < Menu_p->Position.x+width; ++col) {
        if (Grid_p[Menu_p->Position.y+height-1].Glyphs_p[col].codepoint == 'x' 
        &&  Grid_p[Menu_p->Position.y+height-1].Glyphs_p[col].mark & TTYR_CORE_MARK_LINE_GRAPHICS) {
            Grid_p[Menu_p->Position.y+height].Glyphs_p[col].codepoint = 'v';
        }
    }
 
    // Recursion.
    for (int i = 0; i < Menu_p->Items.size; ++i) {
        ttyr_core_ContextMenu *Child_p = Menu_p->Items.pp[i];
        TTYR_CHECK(ttyr_core_drawContextMenuRecursively(Child_p, Grid_p))
    }

    return TTYR_CORE_SUCCESS;
}
