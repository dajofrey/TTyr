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

#include "../../../../external/Netzhaut/src/lib/nhcore/System/Memory.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/System/Thread.h"
#include "../../../../external/Netzhaut/src/lib/nhencoding/Encodings/UTF32.h"
#include "../../../../external/Netzhaut/src/lib/nhencoding/Encodings/UTF8.h"

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

static NH_BOOL ttyr_tty_pathMatchesContextMenu(
    ttyr_tty_ContextMenu *Menu_p, const NH_ENCODING_UTF32 **pp, int count)
{
TTYR_TTY_BEGIN()

    for (int i = count-1; i >= 0; --i) {
        if (!Menu_p || Menu_p->Name.length == 0) {
            TTYR_TTY_END(NH_FALSE)
        }
        for (int j = 0; j < Menu_p->Name.length; ++j) {
            if (pp[i][j] == 0 || pp[i][j] != Menu_p->Name.p[j]) {
                TTYR_TTY_END(NH_FALSE)
                break;
            }
        }
        Menu_p = Menu_p->Parent_p;
    }

TTYR_TTY_END(NH_TRUE)
}

static int ttyr_tty_isContextMenuTiling(
    ttyr_tty_ContextMenu *Menu_p)
{
TTYR_TTY_BEGIN()

    static const NH_ENCODING_UTF32 append_p[] = {'A', 'p', 'p', 'e', 'n', 'd', 0};
    static const NH_ENCODING_UTF32 split_p[] = {'S', 'p', 'l', 'i', 't', 0};
    static const NH_ENCODING_UTF32 window_p[] = {'W', 'i', 'n', 'd', 'o', 'w', 0};
    static const NH_ENCODING_UTF32 tab_p[] = {'T', 'a', 'b', 0};

    static const NH_ENCODING_UTF32 *paths_ppp[][4] = {
        {append_p, tab_p},
        {append_p, window_p},
        {split_p, tab_p},
        {split_p, window_p},
    };

    for (int i = 0; i < 4; ++i) {
        if (ttyr_tty_pathMatchesContextMenu(Menu_p, paths_ppp[i], 2)) {
            TTYR_TTY_END(i)
        }
    }

TTYR_TTY_END(-1)
}

static int ttyr_tty_isContextMenuWindowOrTabSelect(
    ttyr_tty_ContextMenu *Menu_p)
{
TTYR_TTY_BEGIN()

    static const NH_ENCODING_UTF32 window_p[] = {'W', 'i', 'n', 'd', 'o', 'w', 0};
    static const NH_ENCODING_UTF32 tab_p[] = {'T', 'a', 'b', 0};
    static const NH_ENCODING_UTF32 p1[] = {'1', 1, ' ', 0};
    static const NH_ENCODING_UTF32 p2[] = {'2', 1, ' ', 0};
    static const NH_ENCODING_UTF32 p3[] = {'3', 1, ' ', 0};
    static const NH_ENCODING_UTF32 p4[] = {'4', 1, ' ', 0};
    static const NH_ENCODING_UTF32 p5[] = {'5', 1, ' ', 0};
    static const NH_ENCODING_UTF32 p6[] = {'6', 1, ' ', 0};
    static const NH_ENCODING_UTF32 p7[] = {'7', 1, ' ', 0};
    static const NH_ENCODING_UTF32 p8[] = {'8', 1, ' ', 0};
    static const NH_ENCODING_UTF32 p9[] = {'9', 1, ' ', 0};
 
    static const NH_ENCODING_UTF32 *paths_ppp[][18] = {
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
        if (ttyr_tty_pathMatchesContextMenu(Menu_p, paths_ppp[i], 2)) {
            TTYR_TTY_END(i)
        }
    }

TTYR_TTY_END(-1)
}

static int ttyr_tty_handleContextMenuTiling(
    int action, ttyr_tty_Tile *Tile_p, int cCol, int cRow)
{
TTYR_TTY_BEGIN()

    nh_wsi_KeyboardEvent Event;
    ttyr_tty_MacroWindow *Window_p = ((ttyr_tty_TTY*)nh_core_getWorkloadArg())->Window_p;

    int direction = 0;
    
    float col = (((float)cCol)/((float)Tile_p->colSize))-0.5f;
    NH_BOOL right = col > 0.0f;
    col = fabs(col);

    float row = (((float)cRow)/((float)Tile_p->rowSize))-0.5f;
    NH_BOOL bottom = row > 0.0f;
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
            Event.trigger = NH_WSI_TRIGGER_PRESS;
            Event.codepoint = TTYR_TTY_TILING_KEY;
            Event.state |= NH_WSI_MODIFIER_CONTROL;
            ttyr_tty_handleTilingInput(Window_p, Event); 

            if (direction == 0) {
                Event.codepoint = TTYR_TTY_INSERT_TILE_TOP_KEY;
            } else if (direction == 1) {
                Event.codepoint = TTYR_TTY_INSERT_TILE_RIGHT_KEY;
            } else if (direction == 2) {
                Event.codepoint = TTYR_TTY_INSERT_TILE_BOTTOM_KEY;
            } else if (direction == 3) {
                Event.codepoint = TTYR_TTY_INSERT_TILE_LEFT_KEY;
            }
 
            Event.state = 0;
            ttyr_tty_handleTilingInput(Window_p, Event);
            break;

        // Handle tab split.
        case 2 :
            Event.trigger = NH_WSI_TRIGGER_PRESS;
            Event.codepoint = TTYR_TTY_TILING_KEY;
            Event.state |= NH_WSI_MODIFIER_CONTROL;
            ttyr_tty_handleTilingInput(Window_p, Event); 

            Event.state = 0;
            Event.codepoint = TTYR_TTY_SPLIT_KEY;
            ttyr_tty_handleTilingInput(Window_p, Event); 

            if (direction == 0) {
                Event.codepoint = TTYR_TTY_INSERT_TILE_TOP_KEY;
            } else if (direction == 1) {
                Event.codepoint = TTYR_TTY_INSERT_TILE_RIGHT_KEY;
            } else if (direction == 2) {
                Event.codepoint = TTYR_TTY_INSERT_TILE_BOTTOM_KEY;
            } else if (direction == 3) {
                Event.codepoint = TTYR_TTY_INSERT_TILE_LEFT_KEY;
            }
 
            ttyr_tty_handleTilingInput(Window_p, Event);
            break;
 
        // Handle window append.
        case 1 :
            Event.trigger = NH_WSI_TRIGGER_PRESS;
            Event.codepoint = TTYR_TTY_TILING_KEY;
            Event.state |= NH_WSI_MODIFIER_CONTROL;
            ttyr_tty_handleTilingInput(Window_p, Event); 
            ttyr_tty_handleTilingInput(Window_p, Event); 

            if (direction == 0) {
                Event.codepoint = TTYR_TTY_INSERT_TILE_TOP_KEY;
            } else if (direction == 1) {
                Event.codepoint = TTYR_TTY_INSERT_TILE_RIGHT_KEY;
            } else if (direction == 2) {
                Event.codepoint = TTYR_TTY_INSERT_TILE_BOTTOM_KEY;
            } else if (direction == 3) {
                Event.codepoint = TTYR_TTY_INSERT_TILE_LEFT_KEY;
            }

            Event.state = 0;
            ttyr_tty_handleTilingInput(Window_p, Event);
            break;

        // Handle window split.
        case 3 :
            Event.trigger = NH_WSI_TRIGGER_PRESS;
            Event.codepoint = TTYR_TTY_TILING_KEY;
            Event.state |= NH_WSI_MODIFIER_CONTROL;
            ttyr_tty_handleTilingInput(Window_p, Event); 
            ttyr_tty_handleTilingInput(Window_p, Event); 

            Event.codepoint = TTYR_TTY_SPLIT_KEY;
            Event.state = 0;
            ttyr_tty_handleTilingInput(Window_p, Event); 

            if (direction == 0) {
                Event.codepoint = TTYR_TTY_INSERT_TILE_TOP_KEY;
            } else if (direction == 1) {
                Event.codepoint = TTYR_TTY_INSERT_TILE_RIGHT_KEY;
            } else if (direction == 2) {
                Event.codepoint = TTYR_TTY_INSERT_TILE_BOTTOM_KEY;
            } else if (direction == 3) {
                Event.codepoint = TTYR_TTY_INSERT_TILE_LEFT_KEY;
            }
 
            ttyr_tty_handleTilingInput(Window_p, Event);
            break;
    }

TTYR_TTY_END(-1)
}

// CREATE/FREE =====================================================================================

static ttyr_tty_ContextMenu *ttyr_tty_parseContextMenu(
    NH_ENCODING_UTF32 **menu_pp, ttyr_tty_ContextMenu *Parent_p)
{
TTYR_TTY_BEGIN()

    ttyr_tty_ContextMenu *Menu_p = nh_core_allocate(sizeof(ttyr_tty_ContextMenu));
    TTYR_TTY_CHECK_MEM_2(NULL, Menu_p)

    Menu_p->Parent_p = Parent_p;
    Menu_p->active = NH_FALSE;
    Menu_p->hit = NH_FALSE;
    Menu_p->Name = nh_encoding_initUTF32(16); 
    Menu_p->Position.x = 0;
    Menu_p->Position.y = 0;
    Menu_p->Items = nh_core_initList(8);

    NH_BOOL curly = NH_FALSE;

    while (**menu_pp != 0) {
        if (**menu_pp == '{' || (**menu_pp == ',' && curly)) {
            (*menu_pp)++;
            ttyr_tty_ContextMenu *Child_p = ttyr_tty_parseContextMenu(menu_pp, Menu_p);
            TTYR_TTY_CHECK_NULL_2(NULL, Child_p)
            nh_core_appendToList(&Menu_p->Items, Child_p);
            curly = NH_TRUE;
            continue;
        }
        else if (**menu_pp == '}') {
            if (curly) {
                curly = NH_FALSE;
                (*menu_pp)++;
                continue;
            }
            TTYR_TTY_END(Menu_p)
        }
        else if (**menu_pp == ',') {
            TTYR_TTY_END(Menu_p)
        }
        else {
            nh_encoding_appendUTF32Codepoint(&Menu_p->Name, **menu_pp);
            (*menu_pp)++;
        }
    }

TTYR_TTY_END(Menu_p)
}

void ttyr_tty_freeContextMenu(
    ttyr_tty_ContextMenu *Menu_p)
{
TTYR_TTY_BEGIN()

    for (int i = 0; i < Menu_p->Items.size; ++i) {
        ttyr_tty_freeContextMenu(Menu_p->Items.pp[i]);
    }
    nh_core_freeList(&Menu_p->Items, NH_FALSE);
    nh_encoding_freeUTF32(&Menu_p->Name);
    nh_core_free(Menu_p);

TTYR_TTY_SILENT_END()
}

// POSITION ========================================================================================

static void ttyr_tty_computeContextMenuPosition(
    ttyr_tty_ContextMenu *Menu_p, int x, int y, int maxX, int maxY)
{
TTYR_TTY_BEGIN()

    // Get menu width.

    int width = 0;
    for (int i = 0; i < Menu_p->Items.size; ++i) {
        nh_encoding_UTF32String *Name_p = &((ttyr_tty_ContextMenu*)Menu_p->Items.pp[i])->Name;
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
        TTYR_TTY_SILENT_END() // TODO
    }

    for (int i = 0; i < Menu_p->Items.size; ++i) {
        ttyr_tty_computeContextMenuPosition(Menu_p->Items.pp[i], x + width + 2, Menu_p->Position.y + i, maxX, maxY);
    }

TTYR_TTY_SILENT_END()
}

static NH_BOOL ttyr_tty_compareContextMenuName(
    ttyr_tty_ContextMenu *Menu_p, NH_BYTE *name_p)
{
TTYR_TTY_BEGIN()

    nh_encoding_UTF8String Name = nh_encoding_encodeUTF8(Menu_p->Name.p, Menu_p->Name.length);
    NH_BOOL result = !strcmp(Name.p, name_p);
    nh_encoding_freeUTF8(&Name);

TTYR_TTY_END(result)
}

ttyr_tty_ContextMenu *ttyr_tty_isContextMenuHit(
    ttyr_tty_ContextMenu *Menu_p, ttyr_tty_ContextMenu *Parent_p, NH_BOOL recursive, int x, int y)
{
TTYR_TTY_BEGIN()

    int x2 = x;
    int y2 = Menu_p->Position.y;

    int width = 0;
    for (int i = 0; Parent_p && i < Parent_p->Items.size; ++i) {
        nh_encoding_UTF32String *Name_p = &((ttyr_tty_ContextMenu*)Parent_p->Items.pp[i])->Name;
        if (Name_p->length > width) {width = Name_p->length;}
        if (Parent_p->Items.pp[i] == Menu_p) {y2 = Parent_p->Position.y + i;}
    }

    if (y2 == y && Parent_p && Parent_p->Position.x <= x2 && (Parent_p->Position.x + width) >= x2) {
        ttyr_tty_TTY *TTY_p = nh_core_getWorkloadArg();
        int maxCols = ((ttyr_tty_View*)TTY_p->Views.pp[0])->cols;
        int width2 = 0;
        for (int i = 0; i < Menu_p->Items.size; ++i) {
            nh_encoding_UTF32String *Name_p = &((ttyr_tty_ContextMenu*)Menu_p->Items.pp[i])->Name;
            if (Name_p->length > width2) {width2 = Name_p->length;}
        }
        if ((Menu_p->Position.x + width2 + 2 < maxCols && Parent_p->Position.x + width + 2 < maxCols) || Menu_p->Items.size == 0) {
            TTYR_TTY_END(Menu_p)
        }
    }

    for (int i = 0; (Menu_p->active || Menu_p->hit) && i < Menu_p->Items.size && recursive; ++i) {
        ttyr_tty_ContextMenu *Result_p = ttyr_tty_isContextMenuHit(Menu_p->Items.pp[i], Menu_p, recursive, x, y);
        if (Result_p) {TTYR_TTY_END(Result_p)}
    }

TTYR_TTY_END(NULL)
}

void ttyr_tty_updateContextMenuHit(
    ttyr_tty_ContextMenu *Menu_p, ttyr_tty_ContextMenu *Parent_p, int x, int y, NH_BOOL activate)
{
TTYR_TTY_BEGIN()

    NH_BOOL newHit = !Menu_p->hit;
    Menu_p->hit = ttyr_tty_isContextMenuHit(Menu_p, Parent_p, NH_FALSE, x, y) != NULL && Menu_p->Name.length > 0;

    if (Menu_p->hit) {
        Menu_p->active = NH_TRUE;
    }
    if (Menu_p->active && Parent_p) {
        for (int i = 0; i < Parent_p->Items.size; ++i) {
            ((ttyr_tty_ContextMenu*)Parent_p->Items.pp[i])->active = Parent_p->Items.pp[i] == Menu_p;
            ((ttyr_tty_ContextMenu*)Parent_p->Items.pp[i])->hit = Parent_p->Items.pp[i] == Menu_p;
        }
    }

    if (newHit && Menu_p->hit) {
    }

    for (int i = 0; (Menu_p->active || Menu_p->hit) && i < Menu_p->Items.size; ++i) {
        ttyr_tty_updateContextMenuHit(Menu_p->Items.pp[i], Menu_p, x, y, activate);
        if (((ttyr_tty_ContextMenu*)Menu_p->Items.pp[i])->hit) {Menu_p->hit = NH_TRUE;}
    }

TTYR_TTY_SILENT_END()
}

// MOUSE MENU ======================================================================================
// Mouse menu functions.

ttyr_tty_ContextMenu *ttyr_tty_createMouseMenu1(
    int x, int y)
{
TTYR_TTY_BEGIN()

    nh_encoding_UTF32String Menu = nh_encoding_initUTF32(128);
    nh_encoding_appendUTF32Codepoint(&Menu, '{');

    ttyr_tty_TTY *TTY_p = nh_core_getWorkloadArg();

    ttyr_tty_Program *Program_p = ttyr_tty_getCurrentProgram(&TTYR_TTY_MACRO_TAB(TTY_p->Window_p->Tile_p)->MicroWindow);

    // Generate command menu.
    if (Program_p && Program_p->Prototype_p && Program_p->Prototype_p->commands > 0) {
        for (int i = 0; i < Program_p->Prototype_p->commands; ++i) {
            nh_encoding_UTF32String *Name_p = Program_p->Prototype_p->CommandNames_p+i;
            nh_encoding_appendUTF32(&Menu, Name_p->p, Name_p->length); 
            if (i < Program_p->Prototype_p->commands-1) {nh_encoding_appendUTF32Codepoint(&Menu, ',');}
        }
    }
 
    nh_encoding_appendUTF32Codepoint(&Menu, '}');

    NH_ENCODING_UTF32 *p = Menu.p;
    ttyr_tty_ContextMenu *Menu_p = ttyr_tty_parseContextMenu(&p, NULL);
    TTYR_TTY_CHECK_NULL_2(NULL, Menu_p)

    ttyr_tty_computeContextMenuPosition(Menu_p, x, y, ((ttyr_tty_View*)TTY_p->Views.pp[0])->cols, ((ttyr_tty_View*)TTY_p->Views.pp[0])->rows);

    nh_encoding_freeUTF32(&Menu);

TTYR_TTY_END(Menu_p)
}

ttyr_tty_ContextMenu *ttyr_tty_createMouseMenu2(
    int x, int y)
{
TTYR_TTY_BEGIN()

    nh_encoding_UTF32String Menu = nh_encoding_initUTF32(128);
    nh_encoding_appendUTF32Codepoint(&Menu, '{');

    ttyr_tty_TTY *TTY_p = nh_core_getWorkloadArg();
    ttyr_tty_Config Config = ttyr_tty_getConfig();

    bool border = false;

    if (Config.Menu.program && TTY_p->Prototypes.size > 1) {
        ttyr_tty_Program *Program_p = ttyr_tty_getCurrentProgram(&TTYR_TTY_MACRO_TAB(TTY_p->Window_p->Tile_p)->MicroWindow);
        NH_ENCODING_UTF32 apps_p[] = {PROGRAM_NAME '{'};
        nh_encoding_appendUTF32(&Menu, apps_p, sizeof(apps_p)/sizeof(apps_p[0]));
        int width = 0;
        for (int i = 0; i < TTY_p->Prototypes.size; ++i) {
            if (((ttyr_tty_ProgramPrototype*)TTY_p->Prototypes.pp[i])->Name.length > width) {
                width = ((ttyr_tty_ProgramPrototype*)TTY_p->Prototypes.pp[i])->Name.length;
            }
        }
        for (int i = 0; i < TTY_p->Prototypes.size; ++i) {
            ttyr_tty_ProgramPrototype *Prototype_p = TTY_p->Prototypes.pp[i];
            nh_encoding_appendUTF32(&Menu, Prototype_p->Name.p, Prototype_p->Name.length); 
            for (int j = width - Prototype_p->Name.length; j > 0; --j) {
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
        NH_ENCODING_UTF32 tiling_p[] = {
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
        NH_ENCODING_UTF32 tiling_p[] = {
            'A', 'p', 'p', 'e', 'n', 'd', '{',
                 'W', 'i', 'n', 'd', 'o', 'w', ',',
                 'T', 'a', 'b',
            '}', ',',
        };
        nh_encoding_appendUTF32(&Menu, tiling_p, sizeof(tiling_p)/sizeof(tiling_p[0]));
        border = true;
    }

//    if (Config.Menu.window) {
//        NH_ENCODING_UTF32 tmp1_p[] = {',', 'W', 'i', 'n', 'd', 'o', 'w', '{'};
//        nh_encoding_appendUTF32(&Menu, tmp1_p, 7);
//        for (int i = 0; i < Config.windows; ++i) {
//            NH_ENCODING_UTF32 tmp2_p[] = {i + '1', 1, ' '};
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
//        NH_ENCODING_UTF32 tmp3_p[] = {',', 'T', 'a', 'b', '{'};
//        nh_encoding_appendUTF32(&Menu, Config.Menu.window ? tmp3_p+1 : tmp3_p , Config.Menu.window ? 3 : 4);
//        for (int i = 0; i < Config.tabs; ++i) {
//            NH_ENCODING_UTF32 tmp2_p[] = {i + '1', 1, ' '};
//            if (((ttyr_tty_MacroTile*)TTY_p->Window_p->Tile_p->p)->current == i) {
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
        NH_ENCODING_UTF32 close_p[] = {'C', 'l', 'o', 's', 'e', ','};
        nh_encoding_appendUTF32(&Menu, close_p, sizeof(close_p)/sizeof(close_p[0]));
        border = true;
    }

    if (Config.Menu.debug) {
        if (border) {
            nh_encoding_appendUTF32Codepoint(&Menu, ',');
        } 
 
        NH_ENCODING_UTF32 debug_p[] = {'D', 'e', 'b', 'u', 'g', '{', 0};
        nh_encoding_appendUTF32(&Menu, debug_p, sizeof(debug_p)/sizeof(debug_p[0]));
     
        NH_ENCODING_UTF32 x_p[16];
        NH_ENCODING_UTF32 y_p[16];
    
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
    
        ttyr_tty_Glyph Glyph = ((ttyr_tty_View*)TTY_p->Views.pp[0])->Grid1_p[y].Glyphs_p[x];
    
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

    NH_ENCODING_UTF32 *p = Menu.p;
    if (Menu.length) {
        p[Menu.length-1] = 0; // Remove the last ','
    }

    ttyr_tty_ContextMenu *Menu_p = ttyr_tty_parseContextMenu(&p, NULL);
    TTYR_TTY_CHECK_NULL_2(NULL, Menu_p)

    ttyr_tty_computeContextMenuPosition(Menu_p, x, y, ((ttyr_tty_View*)TTY_p->Views.pp[0])->cols, ((ttyr_tty_View*)TTY_p->Views.pp[0])->rows);

    nh_encoding_freeUTF32(&Menu);

TTYR_TTY_END(Menu_p)
}

TTYR_TTY_RESULT ttyr_tty_handleMouseMenuPress(
    ttyr_tty_ContextMenu *Root_p, ttyr_tty_ContextMenu *Menu_p)
{
TTYR_TTY_BEGIN()

    if (!Menu_p->Parent_p || Menu_p->Name.length == 0) {TTYR_TTY_END(TTYR_TTY_SUCCESS)}

    ttyr_tty_TTY *TTY_p = nh_core_getWorkloadArg();

    // Handle tiling or tabing if necessary.
    int tiling = ttyr_tty_isContextMenuTiling(Menu_p);
    if (tiling >= 0) {
        if (tiling == 1 || tiling == 3) { 
            ttyr_tty_handleContextMenuTiling(tiling, TTY_p->Window_p->Tile_p, Root_p->cCol, Root_p->cRow);
        } else {
            ttyr_tty_handleContextMenuTiling(tiling, TTYR_TTY_MICRO_TAB(TTYR_TTY_MACRO_TAB(TTY_p->Window_p->Tile_p))->Tile_p, Root_p->cCol, Root_p->cRow);
        }
        nh_wsi_KeyboardEvent Event;
        Event.trigger = NH_WSI_TRIGGER_PRESS;
        Event.codepoint = 13;
        TTYR_TTY_END(ttyr_tty_handleTilingInput(TTY_p->Window_p, Event))
    }

    int select = ttyr_tty_isContextMenuWindowOrTabSelect(Menu_p);
    if (select >= 0) {
        if (select < 9) {
            TTYR_TTY_CHECK_NULL(ttyr_tty_insertAndFocusWindow(TTY_p, select))
        } else {
            ((ttyr_tty_MacroTile*)TTY_p->Window_p->Tile_p->p)->current = select - 9; 
        }
        TTYR_TTY_END(TTYR_TTY_SUCCESS)
    }

    // Handle program switch if necessary.
    NH_ENCODING_UTF32 program_p[] = {PROGRAM_NAME 0};
    NH_ENCODING_UTF32 close_p[] = {'C', 'l', 'o', 's', 'e', 0};
    if (nh_encoding_compareUTF32(Menu_p->Parent_p->Name.p, program_p)) {
        for (int i = 0; i < Menu_p->Parent_p->Items.size; ++i) {
            ttyr_tty_ContextMenu *Prog_p = Menu_p->Parent_p->Items.pp[i];
            if (Prog_p->active) {
                TTYR_TTY_MACRO_TAB(((ttyr_tty_TTY*)nh_core_getWorkloadArg())->Window_p->Tile_p)->MicroWindow.current = i;
            } 
        }
    } else if (nh_encoding_compareUTF32(Menu_p->Name.p, close_p)) {
        TTYR_TTY_MICRO_TAB(TTYR_TTY_MACRO_TAB(TTY_p->Window_p->Tile_p))->Tile_p->close = NH_TRUE;
    } else {
        // Handle command execution if necessary.
        for (int i = 0; i < Menu_p->Parent_p->Items.size; ++i) {
            ttyr_tty_ContextMenu *Cmd_p = Menu_p->Parent_p->Items.pp[i];
            if (Cmd_p->active) {
                ttyr_tty_Program *Program_p = ttyr_tty_getCurrentProgram(&TTYR_TTY_MACRO_TAB(TTY_p->Window_p->Tile_p)->MicroWindow);
                if (Program_p->Prototype_p->Callbacks.handleCommand_f) {
                    Program_p->command = i;
                    Program_p->Prototype_p->Callbacks.handleCommand_f(Program_p, NULL);
                }
            } 
        }
    }

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

// DRAW ============================================================================================

TTYR_TTY_RESULT ttyr_tty_drawContextMenuRecursively(
    ttyr_tty_ContextMenu *Menu_p, ttyr_tty_Row *Grid_p)
{
TTYR_TTY_BEGIN()

    if (Menu_p == NULL || Menu_p->Items.size == 0 || Menu_p->hit == NH_FALSE && Menu_p->active == NH_FALSE) {
        TTYR_TTY_END(TTYR_TTY_SUCCESS)
    }

    int width = 0;
    int height = Menu_p->Items.size;

    // Get menu width.
    for (int i = 0; i < Menu_p->Items.size; ++i) {
        nh_encoding_UTF32String *String_p = &((ttyr_tty_ContextMenu*)Menu_p->Items.pp[i])->Name;
        if (String_p->length > width) {width = String_p->length;}
    }

    // Draw item names.
    for (int row = Menu_p->Position.y, i = 0; row < Menu_p->Position.y+height; ++row, ++i) {
        ttyr_tty_ContextMenu *Child_p = Menu_p->Items.pp[i];
        if (Child_p->hit) {
            for (int col = Menu_p->Position.x, j = 0; j < width && Child_p->Name.p[j] != 1; ++col, ++j) {
                Grid_p[row].Glyphs_p[col].Attributes.reverse = NH_TRUE;
            }
        }
        for (int col = Menu_p->Position.x, j = 0; col < Menu_p->Position.x+width; ++col, ++j) {
            Grid_p[row].Glyphs_p[col].codepoint = Child_p->Name.length > j ? Child_p->Name.p[j] : ' ';
            // Draw horizontal separator if required.
            if (Child_p->Name.length == 0) {
                Grid_p[row].Glyphs_p[col].codepoint = 'q';
                Grid_p[row].Glyphs_p[col].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
            }
            // Draw vertical separator if required.
            else if (Child_p->Name.p[j] == 1) {
                Grid_p[row].Glyphs_p[col].codepoint = 'x';
                Grid_p[row].Glyphs_p[col].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
            }
        }
    }

    // Add basic outer lines.
    for (int row = Menu_p->Position.y, i = 0; row < Menu_p->Position.y+height; ++row, ++i) {
        Grid_p[row].Glyphs_p[Menu_p->Position.x-1].codepoint = 'x';
        Grid_p[row].Glyphs_p[Menu_p->Position.x-1].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
        Grid_p[row].Glyphs_p[Menu_p->Position.x+width].codepoint = 'x';
        Grid_p[row].Glyphs_p[Menu_p->Position.x+width].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
    }

    for (int col = Menu_p->Position.x-1, j = 0; col < Menu_p->Position.x+width+1; ++col, ++j) {
        Grid_p[Menu_p->Position.y-1].Glyphs_p[col].codepoint = 'q';
        Grid_p[Menu_p->Position.y-1].Glyphs_p[col].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
        Grid_p[Menu_p->Position.y+height].Glyphs_p[col].codepoint = 'q';
        Grid_p[Menu_p->Position.y+height].Glyphs_p[col].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
    }

    Grid_p[Menu_p->Position.y-1].Glyphs_p[Menu_p->Position.x-1].codepoint = 'l';
    Grid_p[Menu_p->Position.y-1].Glyphs_p[Menu_p->Position.x-1].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
    Grid_p[Menu_p->Position.y-1].Glyphs_p[Menu_p->Position.x+width].codepoint = 'k';
    Grid_p[Menu_p->Position.y-1].Glyphs_p[Menu_p->Position.x+width].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
    Grid_p[Menu_p->Position.y+height].Glyphs_p[Menu_p->Position.x-1].codepoint = 'm';
    Grid_p[Menu_p->Position.y+height].Glyphs_p[Menu_p->Position.x-1].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
    Grid_p[Menu_p->Position.y+height].Glyphs_p[Menu_p->Position.x+width].codepoint = 'j';
    Grid_p[Menu_p->Position.y+height].Glyphs_p[Menu_p->Position.x+width].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;

    // Add horizontal outer-line separator |- piece.
    for (int row = Menu_p->Position.y; row < Menu_p->Position.y+height; ++row) {
        if (Grid_p[row].Glyphs_p[Menu_p->Position.x].codepoint == 'q' 
        &&  Grid_p[row].Glyphs_p[Menu_p->Position.x].mark & TTYR_TTY_MARK_LINE_GRAPHICS) {
            Grid_p[row].Glyphs_p[Menu_p->Position.x-1].codepoint = 't';
        }
    } 

    // Add horizontal outer-line separator -| piece.
    for (int row = Menu_p->Position.y; row < Menu_p->Position.y+height; ++row) {
        if (Grid_p[row].Glyphs_p[Menu_p->Position.x+width-1].codepoint == 'q' 
        &&  Grid_p[row].Glyphs_p[Menu_p->Position.x+width-1].mark & TTYR_TTY_MARK_LINE_GRAPHICS) {
            Grid_p[row].Glyphs_p[Menu_p->Position.x+width].codepoint = 'u';
        }
    }
 
    // Add vertical outer-line separator T piece.
    for (int col = Menu_p->Position.x; col < Menu_p->Position.x+width; ++col) {
        if (Grid_p[Menu_p->Position.y].Glyphs_p[col].codepoint == 'x' 
        &&  Grid_p[Menu_p->Position.y].Glyphs_p[col].mark & TTYR_TTY_MARK_LINE_GRAPHICS) {
            Grid_p[Menu_p->Position.y-1].Glyphs_p[col].codepoint = 'w';
        }
    }
 
    // Add vertical outer-line separator bottom piece.
    for (int col = Menu_p->Position.x; col < Menu_p->Position.x+width; ++col) {
        if (Grid_p[Menu_p->Position.y+height-1].Glyphs_p[col].codepoint == 'x' 
        &&  Grid_p[Menu_p->Position.y+height-1].Glyphs_p[col].mark & TTYR_TTY_MARK_LINE_GRAPHICS) {
            Grid_p[Menu_p->Position.y+height].Glyphs_p[col].codepoint = 'v';
        }
    }
 
    // Recursion.
    for (int i = 0; i < Menu_p->Items.size; ++i) {
        ttyr_tty_ContextMenu *Child_p = Menu_p->Items.pp[i];
        TTYR_TTY_CHECK(ttyr_tty_drawContextMenuRecursively(Child_p, Grid_p))
    }

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

