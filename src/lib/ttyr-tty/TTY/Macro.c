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

#include "Macro.h"
#include "Program.h"
#include "TTY.h"
#include "SideBar.h"
#include "Titlebar.h"

#include "../Common/Macros.h"

#include "../../../../external/Netzhaut/src/lib/nhcore/System/Memory.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/System/Thread.h"

#include "../../../../external/Netzhaut/src/lib/nhencoding/Encodings/UTF32.h"

#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

// MACRO TILE ======================================================================================
// In and exit points of a macro tile.

ttyr_tty_Tile *ttyr_tty_createMacroTile(
    ttyr_tty_Tile *Parent_p, nh_List **MicroTabs_pp, int index)
{
TTYR_TTY_BEGIN()

    ttyr_tty_MacroTile *Tile_p = nh_core_allocate(sizeof(ttyr_tty_MacroTile));
    TTYR_TTY_CHECK_MEM_2(NULL, Tile_p)

    nh_List MacroTabs = nh_core_initList(9);
 
    for (int i = 0; i < 9; ++i) {
        ttyr_tty_MacroTab *MacroTab_p = nh_core_allocate(sizeof(ttyr_tty_MacroTab));
        TTYR_TTY_CHECK_MEM_2(NULL, MacroTab_p)
        MacroTab_p->TopBar = ttyr_tty_initTopBar();
        MacroTab_p->MicroWindow = MicroTabs_pp == NULL || Parent_p == NULL ?
            ttyr_tty_initMicroWindow(MicroTabs_pp != NULL ? MicroTabs_pp[i] : NULL) : TTYR_TTY_MACRO_TAB_2(Parent_p, i)->MicroWindow;
        nh_core_appendToList(&MacroTabs, MacroTab_p);
     }

     Tile_p->MacroTabs = MacroTabs;
     Tile_p->current = 0;

TTYR_TTY_END(ttyr_tty_createTile(Tile_p, TTYR_TTY_TILE_TYPE_MACRO, Parent_p, index))
}

// Helper function for destroying a tile without side effects.
void ttyr_tty_destroyMacroTile(
    ttyr_tty_MacroTile *Tile_p)
{
TTYR_TTY_BEGIN()

    for (int i = 0; i < Tile_p->MacroTabs.size; ++i) {
        ttyr_tty_MacroTab *Tab_p = Tile_p->MacroTabs.pp[i];
        ttyr_tty_freeTopBar(&Tab_p->TopBar);
        ttyr_tty_destroyMicroWindow(&Tab_p->MicroWindow);
    }

    nh_core_freeList(&Tile_p->MacroTabs, NH_TRUE);
    nh_core_free(Tile_p);

TTYR_TTY_SILENT_END()
}

// TAB =============================================================================================
// In and exit points of a tab tile.

static ttyr_tty_MacroWindow *ttyr_tty_createMacroWindow(
    void *TTY_p)
{
TTYR_TTY_BEGIN()

    ttyr_tty_MacroWindow *Window_p = nh_core_allocate(sizeof(ttyr_tty_MacroWindow));
    TTYR_TTY_CHECK_MEM_2(NULL, Window_p)

    nh_List *MicroTabs_pp[9] = {};
    for (int i = 0; i < 9; ++i) {
        MicroTabs_pp[i] = ttyr_tty_createMicroTabs(TTY_p);
        TTYR_TTY_CHECK_NULL_2(NULL, MicroTabs_pp[i])
    }

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    Window_p->RootTile_p      = ttyr_tty_createMacroTile(NULL, MicroTabs_pp, 0);
    Window_p->Tile_p          = Window_p->RootTile_p;
    Window_p->LastFocus_p     = Window_p->RootTile_p;
    Window_p->refreshGrid1    = NH_FALSE;
    Window_p->refreshCursor   = NH_FALSE;
    Window_p->refreshTitlebar = NH_FALSE;
    Window_p->close           = NH_FALSE;

    Window_p->Tiling.mode  = TTYR_TTY_TILING_MODE_MICRO;
    Window_p->Tiling.stage = TTYR_TTY_TILING_STAGE_DONE;

    Window_p->MouseMenu_p = NULL;

TTYR_TTY_END(Window_p)
}

static TTYR_TTY_RESULT ttyr_tty_destroyMacroWindow(
    ttyr_tty_MacroWindow *Window_p)
{
TTYR_TTY_BEGIN()

    if (Window_p->RootTile_p != NULL) {
        TTYR_TTY_END(TTYR_TTY_ERROR_BAD_STATE)
    }

    ttyr_tty_closeTile(Window_p->RootTile_p, Window_p);
    nh_core_free(Window_p);

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

// ADD/REMOVE ======================================================================================

ttyr_tty_MacroWindow *ttyr_tty_insertAndFocusWindow(
    void *TTY_p, int index)
{
TTYR_TTY_BEGIN()

    // Close mouse menu.
    if (((ttyr_tty_TTY*)TTY_p)->Window_p && ((ttyr_tty_TTY*)TTY_p)->Window_p->MouseMenu_p) {
        ttyr_tty_freeContextMenu(((ttyr_tty_TTY*)TTY_p)->Window_p->MouseMenu_p);
        ((ttyr_tty_TTY*)TTY_p)->Window_p->MouseMenu_p = NULL;
    }

    ttyr_tty_MacroWindow *Window_p = nh_core_getFromList(&((ttyr_tty_TTY*)TTY_p)->Windows, index);
    if (Window_p) {
        // Only focus window.
        ((ttyr_tty_TTY*)TTY_p)->Window_p = Window_p;
        TTYR_TTY_END(Window_p)
    } 

    Window_p = ttyr_tty_createMacroWindow(TTY_p);
    TTYR_TTY_CHECK_NULL(Window_p)

    nh_core_insertIntoList(&((ttyr_tty_TTY*)TTY_p)->Windows, Window_p, index);

    // Automatically switch to new window. 
    ((ttyr_tty_TTY*)TTY_p)->Window_p = Window_p;
 
TTYR_TTY_END(Window_p)
}

TTYR_TTY_RESULT ttyr_tty_destroyWindows(
    void *TTY_p)
{
TTYR_TTY_BEGIN()

    for (int i = 0; i < ((ttyr_tty_TTY*)TTY_p)->Windows.size; ++i) {
        ttyr_tty_destroyMacroWindow(((ttyr_tty_TTY*)TTY_p)->Windows.pp[i]);
    }

    ((ttyr_tty_TTY*)TTY_p)->Window_p = NULL;

    nh_core_freeList(&((ttyr_tty_TTY*)TTY_p)->Windows, NH_FALSE);

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

// UPDATE ==========================================================================================

static TTYR_TTY_RESULT ttyr_tty_updateMacroTile(
    ttyr_tty_MacroTile *Tile_p, NH_BOOL *refresh_p)
{
TTYR_TTY_BEGIN()

    ttyr_tty_MacroTab *Tab_p = Tile_p->MacroTabs.pp[Tile_p->current];

    if (Tab_p->TopBar.hasFocus) {
        TTYR_TTY_CHECK(ttyr_tty_updateTopBar(&Tab_p->TopBar))
    }

    if (Tab_p->MicroWindow.Tabs_p == NULL || Tab_p->MicroWindow.Tabs_p->size == 0) {
        TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
    }

    nh_List Tiles = ttyr_tty_getTiles(TTYR_TTY_MICRO_TAB(Tab_p)->RootTile_p);
    for (int i = 0; i < Tiles.size; ++i) {
        ttyr_tty_Program *Program_p = TTYR_TTY_MICRO_TILE(Tiles.pp[i])->Program_p;
        if (!Program_p) {continue;}
    
        if (Program_p->Prototype_p->Callbacks.update_f) {
            TTYR_TTY_CHECK(Program_p->Prototype_p->Callbacks.update_f(Program_p))
        }
    
        if (Program_p->close) {
            *refresh_p = NH_TRUE;
            ((ttyr_tty_Tile*)Tiles.pp[i])->close = NH_TRUE;
        }
        if (Program_p->refresh) {
            *refresh_p = NH_TRUE;
            Program_p->refresh = NH_FALSE;
        }
        if (Tab_p->TopBar.refresh) {
            *refresh_p = NH_TRUE;
            Tab_p->TopBar.refresh = NH_FALSE;
        }
    }
    nh_core_freeList(&Tiles, NH_FALSE);

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

TTYR_TTY_RESULT ttyr_tty_updateMacroWindow(
    ttyr_tty_MacroWindow *Window_p)
{
TTYR_TTY_BEGIN()

    // Get macro tiles.
    nh_List Tiles = ttyr_tty_getTiles(Window_p->RootTile_p);

    for (int i = 0; i < Tiles.size; ++i) 
    {
        ttyr_tty_Tile *Tile_p = Tiles.pp[i];

        if (((ttyr_tty_MacroTile*)Tile_p->p)->current == -1 || ((ttyr_tty_MacroTile*)Tile_p->p)->MacroTabs.size == 0) {
            // Parent tiles have to be ignored.
            continue;
        }

        TTYR_TTY_CHECK(ttyr_tty_updateMacroTile(Tile_p->p, &Tile_p->refresh))

        if (Tile_p->refresh) {
            Tile_p->refresh = NH_FALSE;
            Window_p->refreshGrid1 = NH_TRUE;
            Window_p->refreshCursor = NH_TRUE;
        }

        if (TTYR_TTY_MACRO_TAB(Tile_p)->MicroWindow.Tabs_p == NULL || TTYR_TTY_MACRO_TAB(Tile_p)->MicroWindow.Tabs_p->size == 0) {
            // Micro window has no micro tabs so we don't need to check for closure.
            // This case might happen for example with preview tiles during tiling configuration.
            continue;
        }

        // Close tiles and window if requested.
        if (TTYR_TTY_MICRO_TAB(TTYR_TTY_MACRO_TAB(Tile_p))->Tile_p->close) {

            // Close micro tile.
            TTYR_TTY_CHECK(ttyr_tty_closeTile(TTYR_TTY_MICRO_TAB(TTYR_TTY_MACRO_TAB(Tile_p))->Tile_p,
                TTYR_TTY_MICRO_TAB(TTYR_TTY_MACRO_TAB(Tile_p))))

            // If micro tab does not contain any tiles anymore, close macro tile.
            if (!TTYR_TTY_MICRO_TAB(TTYR_TTY_MACRO_TAB(Tile_p))->RootTile_p) {
                TTYR_TTY_CHECK(ttyr_tty_closeTile(Tile_p, Window_p))
            } 

            // If macro window RootTile_p does not contain any macro tiles anymore, close macro window.
            if (!Window_p->RootTile_p) {
                Window_p->refreshGrid1 = NH_FALSE;
                Window_p->refreshCursor = NH_FALSE;
                Window_p->close = NH_TRUE;
                break;
            }

            break;
        }
    }

    nh_core_freeList(&Tiles, NH_FALSE);

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

// INPUT ===========================================================================================

static NH_BOOL ttyr_tty_isTilingInput(
    ttyr_tty_TTY *TTY_p, nh_wsi_KeyboardEvent Event)
{
TTYR_TTY_BEGIN()

    NH_BOOL tiling = NH_FALSE;

    if (TTY_p->Window_p->Tiling.stage > 0) {
        tiling = NH_TRUE;
    } else if (Event.trigger == NH_WSI_TRIGGER_PRESS) {
        tiling = TTYR_TTY_TILING_KEY == Event.codepoint;
    }

    // If there are no programs to create, tiling is useless.
    if (TTY_p->Prototypes.size == 0) {
        tiling = NH_FALSE;
    }

TTYR_TTY_END(tiling)
}

static TTYR_TTY_RESULT ttyr_tty_handleKeyboardInput(
    ttyr_tty_MacroWindow *Window_p, nh_wsi_Event Event)
{
TTYR_TTY_BEGIN()

    ttyr_tty_Config Config = ttyr_tty_getConfig();
    ttyr_tty_MacroTile *MacroTile_p = Window_p->Tile_p->p;
    ttyr_tty_MacroTab *MacroTab_p = MacroTile_p->MacroTabs.pp[MacroTile_p->current];

    if (MacroTab_p->TopBar.Message.block) {
        NH_BOOL _continue = NH_FALSE;
        TTYR_TTY_CHECK(MacroTab_p->TopBar.Message.callback_f(Event.Keyboard, &_continue))
        Window_p->Tile_p->refresh = NH_TRUE;
        if (!_continue) {TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)}
    }

    ttyr_tty_TTY *TTY_p = nh_core_getWorkloadArg();

    // Switch window or tab.
    if (nh_encoding_isASCIIDigit(Event.Keyboard.codepoint) && Event.Keyboard.trigger == NH_WSI_TRIGGER_PRESS && TTY_p->alt && TTY_p->ctrl) {
        if (Config.windows > Event.Keyboard.codepoint - '1') {
            Window_p = ttyr_tty_insertAndFocusWindow(TTY_p, Event.Keyboard.codepoint - '1');
            Window_p->refreshGrid2 = NH_TRUE;
            Window_p->refreshTitlebar = NH_TRUE;
            Window_p->Tile_p->refresh = NH_TRUE;
            TTYR_TTY_END(TTYR_TTY_SUCCESS)
        }
        TTYR_TTY_END(TTYR_TTY_SUCCESS)
    }
    else if (nh_encoding_isASCIIDigit(Event.Keyboard.codepoint) && Event.Keyboard.trigger == NH_WSI_TRIGGER_PRESS && TTY_p->alt) {
        if (Config.tabs > Event.Keyboard.codepoint - '1') {
            Window_p->Tile_p->refresh = NH_TRUE;
            MacroTile_p->current = Event.Keyboard.codepoint - '1';
        }
        TTYR_TTY_END(TTYR_TTY_SUCCESS)
    }
 
    // Toggle special keys.
    if (Event.Keyboard.special == NH_WSI_KEY_ALT_L && Event.Keyboard.trigger == NH_WSI_TRIGGER_PRESS) {
        TTY_p->alt = NH_TRUE;
    }
    if (Event.Keyboard.special == NH_WSI_KEY_ALT_L && Event.Keyboard.trigger == NH_WSI_TRIGGER_RELEASE) {
        TTY_p->alt = NH_FALSE;
    }
    if (Event.Keyboard.special == NH_WSI_KEY_CONTROL_L && Event.Keyboard.trigger == NH_WSI_TRIGGER_PRESS) {
        TTY_p->ctrl = NH_TRUE;
    }
    if (Event.Keyboard.special == NH_WSI_KEY_CONTROL_L && Event.Keyboard.trigger == NH_WSI_TRIGGER_RELEASE) {
        TTY_p->ctrl = NH_FALSE;
    }

    if (ttyr_tty_isTilingInput(TTY_p, Event.Keyboard)) {
        TTYR_TTY_CHECK(ttyr_tty_handleTilingInput(Window_p, Event.Keyboard))
        TTYR_TTY_END(TTYR_TTY_SUCCESS)
    }

    // Switch tile.
    if (Event.Keyboard.trigger == NH_WSI_TRIGGER_PRESS && TTY_p->alt) {
        ttyr_tty_Tile *New_p = NULL;
        switch (Event.Keyboard.codepoint) {
            case 'w' : New_p = ttyr_tty_switchTile(Window_p, Window_p->Tile_p, 0); break;
            case 'd' : New_p = ttyr_tty_switchTile(Window_p, Window_p->Tile_p, 1); break;
            case 's' : New_p = ttyr_tty_switchTile(Window_p, Window_p->Tile_p, 2); break;
            case 'a' : New_p = ttyr_tty_switchTile(Window_p, Window_p->Tile_p, 3); break;
        }
        if (New_p) {TTYR_TTY_END(TTYR_TTY_SUCCESS)}
    }

    if (TTYR_TTY_QUIT_KEY == Event.Keyboard.codepoint && Event.Keyboard.trigger == NH_WSI_TRIGGER_PRESS) {
        TTYR_TTY_MICRO_TAB(MacroTab_p)->Tile_p->close = NH_TRUE;
        Window_p->Tile_p->refresh = NH_TRUE;
    }
    else if (TTYR_TTY_TOPBAR_KEY == Event.Keyboard.codepoint && Event.Keyboard.trigger == NH_WSI_TRIGGER_PRESS) {
        ttyr_tty_toggleTopBar(&MacroTab_p->TopBar);
        Window_p->Tile_p->refresh = NH_TRUE;
    }
    else if (MacroTab_p->TopBar.hasFocus) {
        TTYR_TTY_CHECK(ttyr_tty_handleTopBarInput(Window_p->Tile_p, Event))
        Window_p->Tile_p->refresh = NH_TRUE;
    }
    else if (ttyr_tty_getCurrentProgram(&MacroTab_p->MicroWindow) != NULL) {
        TTYR_TTY_CHECK(ttyr_tty_getCurrentProgram(&MacroTab_p->MicroWindow)->Prototype_p->Callbacks.handleInput_f(
            ttyr_tty_getCurrentProgram(&MacroTab_p->MicroWindow), Event
        ))
    }
    else {
        TTYR_TTY_CHECK(ttyr_tty_setDefaultMessage(&MacroTab_p->TopBar, TTYR_TTY_MESSAGE_MISC_NO_CURRENT_PROGRAM))
    }

    if (ttyr_tty_getCurrentProgram(&MacroTab_p->MicroWindow)->refresh) {
        Window_p->Tile_p->refresh = NH_TRUE;
        ttyr_tty_getCurrentProgram(&MacroTab_p->MicroWindow)->refresh = NH_FALSE;
    }
    if (ttyr_tty_getCurrentProgram(&MacroTab_p->MicroWindow)->close) {
        Window_p->Tile_p->refresh = NH_TRUE;
        Window_p->Tile_p->close = NH_TRUE;
    }

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

static TTYR_TTY_RESULT ttyr_tty_handleMouseInput(
    ttyr_tty_MacroWindow *Window_p, nh_wsi_Event Event, int col, int row)
{
TTYR_TTY_BEGIN()

    ttyr_tty_TTY *TTY_p = nh_core_getWorkloadArg();
    ttyr_tty_Tile *MacroTile_p = NULL, *MicroTile_p = NULL;
    nh_List MacroTiles = ttyr_tty_getTiles(Window_p->RootTile_p);

    ttyr_tty_Config Config = ttyr_tty_getConfig();

    int cCol = col;
    int cCol2 = col;
    int cRow = row;

    // Get tile that is being hovered over.
    for (int i = 0; i < MacroTiles.size; ++i) {
        MacroTile_p = MacroTiles.pp[i];
        if (MacroTile_p->Children.count > 0) {continue;}
        if (MacroTile_p->rowPosition <= row
        &&  MacroTile_p->rowPosition  + MacroTile_p->rowSize > row
        &&  MacroTile_p->colPosition <= col
        &&  MacroTile_p->colPosition  + MacroTile_p->colSize > col) {
            cCol = col - MacroTile_p->colPosition;
            cCol2 = cCol;
            cRow = row - MacroTile_p->rowPosition;
            if (MacroTile_p->rowPosition <= cRow
            &&  MacroTile_p->rowPosition  + MacroTile_p->rowSize > cRow
            &&  MacroTile_p->colPosition <= cCol
            &&  MacroTile_p->colPosition  + MacroTile_p->colSize > cCol) {
                if (!MacroTile_p->p || !TTYR_TTY_MACRO_TAB(MacroTile_p)->MicroWindow.Tabs_p) {break;}
            }
            if (!MacroTile_p->p || !TTYR_TTY_MACRO_TAB(MacroTile_p)->MicroWindow.Tabs_p) {continue;}
            nh_List MicroTiles = ttyr_tty_getTiles(TTYR_TTY_MICRO_TAB(TTYR_TTY_MACRO_TAB(MacroTile_p))->RootTile_p);
            for (int j = 0; j < MicroTiles.size; ++j) {
                MicroTile_p = MicroTiles.pp[j];
                if (MicroTile_p->Children.count > 0) {continue;}
                if (MicroTile_p->rowPosition <= cRow
                &&  MicroTile_p->rowPosition  + MicroTile_p->rowSize > cRow
                &&  MicroTile_p->colPosition <= cCol
                &&  MicroTile_p->colPosition  + MicroTile_p->colSize > cCol) {
                    cCol -= MicroTile_p->colPosition;
                    cRow -= MicroTile_p->rowPosition;
                    break;
                }
                MicroTile_p = NULL;
            }
            nh_core_freeList(&MicroTiles, NH_FALSE);
            if (MicroTile_p) {break;}
        }
        MacroTile_p = NULL;
        MicroTile_p = NULL;
    }

    if (cCol == -2) {
        // Forward sidebar hit.
        if (Event.Mouse.trigger == NH_WSI_TRIGGER_PRESS) {
            TTYR_TTY_CHECK(ttyr_tty_handleSideBarHit(Event.Mouse.type, cRow))
            TTY_p->Window_p->refreshGrid2 = NH_TRUE;
        }
    }

    if (row == -1 && Config.Titlebar.on == true) {
        // Forward titlebar hit.
        if (Event.Mouse.trigger == NH_WSI_TRIGGER_PRESS) {
            ttyr_tty_handleTitlebarHit(Event.Mouse, cCol);
            TTY_p->Window_p->refreshGrid2 = NH_TRUE;
        }
    }
 
    if (MacroTile_p == NULL || MicroTile_p == NULL) {TTYR_TTY_END(TTYR_TTY_SUCCESS)}

    // Handle mouse-menu input.
    if (Window_p->MouseMenu_p) {
        ttyr_tty_ContextMenu *Hit_p = 
            ttyr_tty_isContextMenuHit(Window_p->MouseMenu_p, NULL, NH_TRUE, col, Config.Titlebar.on ? row+1 : row);
        if (Hit_p) {
            if (Event.Mouse.type == NH_WSI_MOUSE_LEFT && Event.Mouse.trigger == NH_WSI_TRIGGER_PRESS) {
                ttyr_tty_handleMouseMenuPress(Window_p->MouseMenu_p, Hit_p);
                if (Window_p->MouseMenu_p) {
                    ttyr_tty_freeContextMenu(Window_p->MouseMenu_p);
                    Window_p->MouseMenu_p = NULL; // Otherwise we end up with an invalid pointer!
                }
            }
            if (Event.Mouse.trigger == NH_WSI_TRIGGER_MOVE) {
                ttyr_tty_updateContextMenuHit(Window_p->MouseMenu_p, NULL, col, Config.Titlebar.on ? row+1 : row, NH_FALSE);
            }
            Window_p->refreshGrid2 = NH_TRUE;
            ((ttyr_tty_TTY*)nh_core_getWorkloadArg())->Window_p->refreshGrid2 = NH_TRUE; // Focused Window might have changed.
            goto SKIP;
        }
    }

    // Close mouse-menu on left-click and switch tile.
    if (Event.Mouse.type == NH_WSI_MOUSE_LEFT && Event.Mouse.trigger == NH_WSI_TRIGGER_PRESS && MicroTile_p) {
        TTYR_TTY_MICRO_TAB(TTYR_TTY_MACRO_TAB(MacroTile_p))->Tile_p = MicroTile_p;
        Window_p->Tile_p = MacroTile_p;

        if (Window_p->MouseMenu_p) {
            ttyr_tty_freeContextMenu(Window_p->MouseMenu_p);
            Window_p->MouseMenu_p = NULL;
        }

        Window_p->refreshGrid2 = NH_TRUE;
    }

    // Create mouse-menu on right-click and switch tiles.
    if (Event.Mouse.type == NH_WSI_MOUSE_RIGHT && Event.Mouse.trigger == NH_WSI_TRIGGER_PRESS && MicroTile_p) {
        TTYR_TTY_MICRO_TAB(TTYR_TTY_MACRO_TAB(MacroTile_p))->Tile_p = MicroTile_p;
        Window_p->Tile_p = MacroTile_p;

        if (Window_p->MouseMenu_p) {ttyr_tty_freeContextMenu(Window_p->MouseMenu_p);}
        Window_p->MouseMenu_p = ttyr_tty_createMouseMenu(col, Config.Titlebar.on ? row+1 : row);
        TTYR_TTY_CHECK_NULL(Window_p->MouseMenu_p)
        Window_p->MouseMenu_p->active = NH_TRUE;
        Window_p->MouseMenu_p->cCol = cCol;
        Window_p->MouseMenu_p->cRow = cRow;

        Window_p->refreshGrid2 = NH_TRUE;
    }

    // Close mouse-menu on scroll.
    if (Event.Mouse.type == NH_WSI_MOUSE_SCROLL && Window_p->MouseMenu_p) {
        ttyr_tty_freeContextMenu(Window_p->MouseMenu_p);
        Window_p->MouseMenu_p = NULL;
        Window_p->refreshGrid2 = NH_TRUE;
    }

    if (cRow == 0) {
        // Forward topbar hit.
        Event.Mouse.Position.x = cCol2;
        TTYR_TTY_CHECK(ttyr_tty_handleTopBarInput(MacroTile_p, Event))
    } else if (MicroTile_p && TTYR_TTY_MICRO_TILE(MicroTile_p)->Program_p) {
        // Forward program hit.
        Event.Window.Position.x = Event.Mouse.Position.x;
        Event.Window.Position.y = Event.Mouse.Position.y;
        Event.Mouse.Position.x = cCol;
        Event.Mouse.Position.y = cRow - 1; // Subtract topbar. 
        TTYR_TTY_CHECK(TTYR_TTY_MICRO_TILE(MicroTile_p)->Program_p->Prototype_p->Callbacks.handleInput_f(
            TTYR_TTY_MICRO_TILE(MicroTile_p)->Program_p, Event
        ))
    }

    if (cRow != 0) {
        for (int i = 0; i < MacroTiles.size; ++i) {
            if (((ttyr_tty_Tile*)MacroTiles.pp[i])->Children.count > 0) {continue;}
            TTYR_TTY_MACRO_TAB(MacroTiles.pp[i])->TopBar.quitHover = NH_FALSE;
        }
    }

SKIP:

    nh_core_freeList(&MacroTiles, NH_FALSE);

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

TTYR_TTY_RESULT ttyr_tty_handleMacroWindowInput(
    ttyr_tty_MacroWindow *Window_p, nh_wsi_Event Event)
{
TTYR_TTY_BEGIN()

    ttyr_tty_Config Config = ttyr_tty_getConfig();

    switch (Event.type)
    {
        case NH_WSI_EVENT_KEYBOARD :
        {
            TTYR_TTY_CHECK(ttyr_tty_handleKeyboardInput(Window_p, Event))
            break;
        }
        case NH_WSI_EVENT_MOUSE :
        {
            int col = -1;
            int row = -1;
            ttyr_tty_translateMousePosition(((ttyr_tty_TTY*)nh_core_getWorkloadArg())->Views.pp[0], Event.Mouse, &col, &row);
            if (Config.Titlebar.on) {row--;}
            if (col < 0) {break;}
            TTYR_TTY_CHECK(ttyr_tty_handleMouseInput(Window_p, Event, col, row))
            break;
        }
        case NH_WSI_EVENT_WINDOW :
        {
            ttyr_tty_TTY *TTY_p = nh_core_getWorkloadArg();
            if (Event.Window.type == NH_WSI_WINDOW_FOCUS_OUT) {
                TTY_p->alt = NH_FALSE;
                TTY_p->ctrl = NH_FALSE;
                TTY_p->hasFocus = NH_FALSE;
            }
            if (Event.Window.type == NH_WSI_WINDOW_FOCUS_IN) {
                TTY_p->hasFocus = NH_TRUE;
            }
            break;
        }
    }

    Window_p->refreshGrid1 = NH_TRUE;
    Window_p->refreshCursor = NH_TRUE;
 
TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

