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
#include "Titlebar.h"

#include "../Common/Macros.h"

#include "nh-core/System/Memory.h"
#include "nh-core/System/Thread.h"

#include "nh-encoding/Encodings/UTF32.h"

#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

// MACRO TILE ======================================================================================
// In and exit points of a macro tile.

ttyr_core_Tile *ttyr_core_createMacroTile(
    ttyr_core_Tile *Parent_p, nh_core_List **MicroTabs_pp, int index)
{
    ttyr_core_MacroTile *Tile_p = nh_core_allocate(sizeof(ttyr_core_MacroTile));
    TTYR_CHECK_MEM_2(NULL, Tile_p)

    nh_core_List MacroTabs = nh_core_initList(9);
    ttyr_core_Config Config = ttyr_core_getConfig();
 
    for (int i = 0; i < Config.tabs; ++i) {
        ttyr_core_MacroTab *MacroTab_p = nh_core_allocate(sizeof(ttyr_core_MacroTab));
        TTYR_CHECK_MEM_2(NULL, MacroTab_p)
        MacroTab_p->Topbar = ttyr_core_initTopbar();
        MacroTab_p->MicroWindow = MicroTabs_pp == NULL || Parent_p == NULL ?
            ttyr_core_initMicroWindow(MicroTabs_pp != NULL ? MicroTabs_pp[i] : NULL) : TTYR_CORE_MACRO_TAB_2(Parent_p, i)->MicroWindow;
        nh_core_appendToList(&MacroTabs, MacroTab_p);
    }

    Tile_p->MacroTabs = MacroTabs;
    Tile_p->current = 0;

    return ttyr_core_createTile(Tile_p, TTYR_CORE_TILE_TYPE_MACRO, Parent_p, index);
}

// Helper function for destroying a tile without side effects.
void ttyr_core_destroyMacroTile(
    ttyr_core_MacroTile *Tile_p)
{
    for (int i = 0; i < Tile_p->MacroTabs.size; ++i) {
        ttyr_core_MacroTab *Tab_p = Tile_p->MacroTabs.pp[i];
        ttyr_core_freeTopbar(&Tab_p->Topbar);
        ttyr_core_destroyMicroWindow(&Tab_p->MicroWindow);
    }

    nh_core_freeList(&Tile_p->MacroTabs, true);
    nh_core_free(Tile_p);

    return;
}

// TAB =============================================================================================
// In and exit points of a tab tile.

static ttyr_core_MacroWindow *ttyr_core_createMacroWindow(
    void *TTY_p)
{
    ttyr_core_MacroWindow *Window_p = nh_core_allocate(sizeof(ttyr_core_MacroWindow));
    TTYR_CHECK_MEM_2(NULL, Window_p)

    ttyr_core_Config Config = ttyr_core_getConfig();

    nh_core_List *MicroTabs_pp[9] = {};
    for (int i = 0; i < Config.tabs; ++i) {
        MicroTabs_pp[i] = ttyr_core_createMicroTabs(TTY_p);
        TTYR_CHECK_NULL_2(NULL, MicroTabs_pp[i])
    }

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    Window_p->RootTile_p      = ttyr_core_createMacroTile(NULL, MicroTabs_pp, 0);
    Window_p->Tile_p          = Window_p->RootTile_p;
    Window_p->LastFocus_p     = Window_p->RootTile_p;
    Window_p->refreshGrid1    = false;
    Window_p->refreshCursor   = false;
    Window_p->refreshTitlebar = false;
    Window_p->close           = false;

    Window_p->Tiling.mode  = TTYR_CORE_TILING_MODE_MICRO;
    Window_p->Tiling.stage = TTYR_CORE_TILING_STAGE_DONE;

    Window_p->MouseMenu_p = NULL;

    return Window_p;
}

static TTYR_CORE_RESULT ttyr_core_destroyMacroWindow(
    ttyr_core_MacroWindow *Window_p)
{
    if (Window_p->RootTile_p != NULL) {
        return TTYR_CORE_ERROR_BAD_STATE;
    }

    ttyr_core_closeTile(Window_p->RootTile_p, Window_p);
    nh_core_free(Window_p);

    return TTYR_CORE_SUCCESS;
}

// ADD/REMOVE ======================================================================================

ttyr_core_MacroWindow *ttyr_core_insertAndFocusWindow(
    void *TTY_p, int index)
{
    // Close mouse menu.
    if (((ttyr_core_TTY*)TTY_p)->Window_p && ((ttyr_core_TTY*)TTY_p)->Window_p->MouseMenu_p) {
        ttyr_core_freeContextMenu(((ttyr_core_TTY*)TTY_p)->Window_p->MouseMenu_p);
        ((ttyr_core_TTY*)TTY_p)->Window_p->MouseMenu_p = NULL;
    }

    ttyr_core_MacroWindow *Window_p = nh_core_getFromList(&((ttyr_core_TTY*)TTY_p)->Windows, index);
    if (Window_p) {
        // Only focus window.
        ((ttyr_core_TTY*)TTY_p)->Window_p = Window_p;
        return Window_p;
    } 

    for (int i = ((ttyr_core_TTY*)TTY_p)->Windows.size; i <= index; ++i) {
        Window_p = ttyr_core_createMacroWindow(TTY_p);
        TTYR_CHECK_NULL(Window_p)
        nh_core_appendToList(&((ttyr_core_TTY*)TTY_p)->Windows, Window_p);
        // Automatically switch to new window. 
        ((ttyr_core_TTY*)TTY_p)->Window_p = Window_p;
    }

    return Window_p;
}

TTYR_CORE_RESULT ttyr_core_destroyWindows(
    void *TTY_p)
{
    for (int i = 0; i < ((ttyr_core_TTY*)TTY_p)->Windows.size; ++i) {
        ttyr_core_destroyMacroWindow(((ttyr_core_TTY*)TTY_p)->Windows.pp[i]);
    }

    ((ttyr_core_TTY*)TTY_p)->Window_p = NULL;

    nh_core_freeList(&((ttyr_core_TTY*)TTY_p)->Windows, false);

    return TTYR_CORE_SUCCESS;
}

// UPDATE ==========================================================================================

static TTYR_CORE_RESULT ttyr_core_updateMacroTab(
    ttyr_core_MacroTab *Tab_p, bool *refresh_p)
{
    if (Tab_p->Topbar.hasFocus) {
        TTYR_CHECK(ttyr_core_updateTopbar(&Tab_p->Topbar))
    }

    if (Tab_p->MicroWindow.Tabs_p == NULL || Tab_p->MicroWindow.Tabs_p->size == 0) {
        return TTYR_CORE_SUCCESS;
    }

    nh_core_List Tiles = ttyr_core_getTiles(TTYR_CORE_MICRO_TAB(Tab_p)->RootTile_p);
    for (int i = 0; i < Tiles.size; ++i) {
        ttyr_core_Program *Program_p = TTYR_CORE_MICRO_TILE(Tiles.pp[i])->Program_p;
        if (!Program_p) {continue;}
    
        if (Program_p->Prototype_p->Callbacks.update_f) {
            TTYR_CHECK(Program_p->Prototype_p->Callbacks.update_f(Program_p))
        }
    
        if (Program_p->close) {
            *refresh_p = true;
            ((ttyr_core_Tile*)Tiles.pp[i])->close = true;
        }
        if (Program_p->refresh) {
            *refresh_p = true;
            Program_p->refresh = false;
        }
        if (Tab_p->Topbar.refresh) {
            *refresh_p = true;
            Tab_p->Topbar.refresh = false;
        }
    }
    nh_core_freeList(&Tiles, false);

    return TTYR_CORE_SUCCESS;
}

static TTYR_CORE_RESULT ttyr_core_updateMacroTile(
    ttyr_core_MacroTile *Tile_p, bool *refresh_p)
{
    for (int i = 0; i < Tile_p->MacroTabs.size; ++i) {
        bool refresh = false;
        ttyr_core_updateMacroTab(Tile_p->MacroTabs.pp[i], &refresh);
        if (refresh && i == Tile_p->current) {
            *refresh_p = true;
        }
    }
    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT ttyr_core_updateMacroWindow(
    ttyr_core_MacroWindow *Window_p)
{
    // Get macro tiles.
    nh_core_List Tiles = ttyr_core_getTiles(Window_p->RootTile_p);

    for (int i = 0; i < Tiles.size; ++i) 
    {
        ttyr_core_Tile *Tile_p = Tiles.pp[i];

        if (((ttyr_core_MacroTile*)Tile_p->p)->current == -1 || ((ttyr_core_MacroTile*)Tile_p->p)->MacroTabs.size == 0) {
            // Parent tiles have to be ignored.
            continue;
        }

        TTYR_CHECK(ttyr_core_updateMacroTile(Tile_p->p, &Tile_p->refresh))

        if (Tile_p->refresh) {
            Tile_p->refresh = false;
            Window_p->refreshGrid1 = true;
            Window_p->refreshCursor = true;
        }

        if (TTYR_CORE_MACRO_TAB(Tile_p)->MicroWindow.Tabs_p == NULL || TTYR_CORE_MACRO_TAB(Tile_p)->MicroWindow.Tabs_p->size == 0) {
            // Micro window has no micro tabs so we don't need to check for closure.
            // This case might happen for example with preview tiles during tiling configuration.
            continue;
        }

        // Close tiles and window if requested.
        if (TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(Tile_p))->Tile_p->close) {

            // Close micro tile.
            TTYR_CHECK(ttyr_core_closeTile(TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(Tile_p))->Tile_p,
                TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(Tile_p))))

            // If micro tab does not contain any tiles anymore, close macro tile.
            if (!TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(Tile_p))->RootTile_p) {
                TTYR_CHECK(ttyr_core_closeTile(Tile_p, Window_p))
            } 

            // If macro window RootTile_p does not contain any macro tiles anymore, close macro window.
            if (!Window_p->RootTile_p) {
                Window_p->refreshGrid1 = false;
                Window_p->refreshCursor = false;
                Window_p->close = true;
                break;
            }

            break;
        }
    }

    nh_core_freeList(&Tiles, false);

    return TTYR_CORE_SUCCESS;
}

// INPUT ===========================================================================================

static bool ttyr_core_isTilingInput(
    ttyr_core_TTY *TTY_p, nh_api_KeyboardEvent Event)
{
    bool tiling = false;

    if (TTY_p->Window_p->Tiling.stage > 0) {
        tiling = true;
    } else if (Event.trigger == NH_API_TRIGGER_PRESS) {
        tiling = TTYR_CORE_TILING_KEY == Event.codepoint;
    }

    // If there are no programs to create, tiling is useless.
    if (TTY_p->Prototypes.size == 0) {
        tiling = false;
    }

    return tiling;
}

static int ttyr_core_getCurrentWindowIndex(
    ttyr_core_TTY *TTY_p)
{
    int i = 0;
    for (; i < TTY_p->Windows.size && TTY_p->Windows.pp[i] != TTY_p->Window_p; ++i);
    return i;
}

static TTYR_CORE_RESULT ttyr_core_handleKeyboardInput(
    ttyr_core_MacroWindow *Window_p, nh_api_WSIEvent Event)
{
    ttyr_core_Config Config = ttyr_core_getConfig();
    ttyr_core_MacroTile *MacroTile_p = Window_p->Tile_p->p;
    ttyr_core_MacroTab *MacroTab_p = MacroTile_p->MacroTabs.pp[MacroTile_p->current];

    if (MacroTab_p->Topbar.Message.block) {
        bool _continue = false;
        TTYR_CHECK(MacroTab_p->Topbar.Message.callback_f(Event.Keyboard, &_continue))
        Window_p->Tile_p->refresh = true;
        if (!_continue) {return TTYR_CORE_SUCCESS;}
    }

    ttyr_core_TTY *TTY_p = nh_core_getWorkloadArg();

    // Switch window or tab.
//    if (nh_encoding_isASCIIDigit(Event.Keyboard.codepoint) && Event.Keyboard.trigger == NH_API_TRIGGER_PRESS && TTY_p->alt && TTY_p->ctrl) {
//        if (Config.windows > Event.Keyboard.codepoint - '1') {
//            Window_p = ttyr_core_insertAndFocusWindow(TTY_p, Event.Keyboard.codepoint - '1');
//            Window_p->refreshGrid2 = true;
//            Window_p->refreshGrid1 = true;
//            Window_p->refreshTitlebar = true;
//            Window_p->Tile_p->refresh = true;
//            return TTYR_CORE_SUCCESS;
//        }
//        TTY_p->ignoreNext = true;
//        return TTYR_CORE_SUCCESS;
//    }
    if (nh_encoding_isASCIIDigit(Event.Keyboard.codepoint) && Event.Keyboard.trigger == NH_API_TRIGGER_PRESS && (TTY_p->alt || TTY_p->ctrl)) {
        if (Config.tabs > Event.Keyboard.codepoint - '1') {
            Window_p->refreshGrid2 = true;
            Window_p->refreshGrid1 = true;
            Window_p->Tile_p->refresh = true;
            MacroTile_p->current = Event.Keyboard.codepoint - '1';
        }
        return TTYR_CORE_SUCCESS;
    }
 
    // Toggle special keys.
    if (Event.Keyboard.special == NH_API_KEY_ALT_L && Event.Keyboard.trigger == NH_API_TRIGGER_PRESS) {
        TTY_p->alt = true;
        Window_p->refreshGrid1 = true;
        Window_p->refreshGrid2 = true;
    }
    if (Event.Keyboard.special == NH_API_KEY_ALT_L && Event.Keyboard.trigger == NH_API_TRIGGER_RELEASE) {
        TTY_p->alt = false;
        Window_p->refreshGrid1 = true;
        Window_p->refreshGrid2 = true;
    }
    if (Event.Keyboard.special == NH_API_KEY_CONTROL_L && Event.Keyboard.trigger == NH_API_TRIGGER_PRESS) {
        TTY_p->ctrl = true;
        Window_p->refreshGrid1 = true;
        Window_p->refreshGrid2 = true;
    }
    if (Event.Keyboard.special == NH_API_KEY_CONTROL_L && Event.Keyboard.trigger == NH_API_TRIGGER_RELEASE) {
        TTY_p->ctrl = false;
        Window_p->refreshGrid1 = true;
        Window_p->refreshGrid2 = true;
    }

    if (ttyr_core_isTilingInput(TTY_p, Event.Keyboard)) {
        TTYR_CHECK(ttyr_core_handleTilingInput(Window_p, Event.Keyboard))
        return TTYR_CORE_SUCCESS;
    }

    // Switch tile.
    if (Event.Keyboard.trigger == NH_API_TRIGGER_PRESS && (TTY_p->alt || (TTY_p->alt && TTY_p->ctrl))) {
        ttyr_core_Tile *New_p = NULL;
        switch (Event.Keyboard.codepoint) {
            case CTRL_KEY('w') :
            case 'w' : New_p = ttyr_core_switchTile(Window_p, Window_p->Tile_p, 0); break;
            case CTRL_KEY('d') :
            case 'd' : New_p = ttyr_core_switchTile(Window_p, Window_p->Tile_p, 1); break;
            case CTRL_KEY('s') :
            case 's' : New_p = ttyr_core_switchTile(Window_p, Window_p->Tile_p, 2); break;
            case CTRL_KEY('a') :
            case 'a' : New_p = ttyr_core_switchTile(Window_p, Window_p->Tile_p, 3); break;
        }
        if (New_p) {
            Window_p->refreshGrid1 = true;
            Window_p->refreshGrid2 = true;
            return TTYR_CORE_SUCCESS;
        } else {
            // switch window if possible
            if (Event.Keyboard.codepoint == 'd' || (Event.Keyboard.codepoint == CTRL_KEY('d') && TTY_p->ctrl && TTY_p->alt)) {
                Window_p = ttyr_core_insertAndFocusWindow(TTY_p, ttyr_core_getCurrentWindowIndex(TTY_p)+1);
            } else if (Event.Keyboard.codepoint == 'a' || (Event.Keyboard.codepoint == CTRL_KEY('a') && TTY_p->ctrl && TTY_p->alt)) {
                int window = ttyr_core_getCurrentWindowIndex(TTY_p);
                if (window > 0) {
                    Window_p = ttyr_core_insertAndFocusWindow(TTY_p, window-1);
                }
            }
        }
    }

    if (TTYR_CORE_TOPBAR_KEY == Event.Keyboard.codepoint && Event.Keyboard.trigger == NH_API_TRIGGER_PRESS) {
        ttyr_core_toggleTopbar(&MacroTab_p->Topbar);
        Window_p->Tile_p->refresh = true;
    }
    else if (MacroTab_p->Topbar.hasFocus) {
        TTYR_CHECK(ttyr_core_handleTopbarInput(Window_p->Tile_p, Event))
        Window_p->Tile_p->refresh = true;
    }
    else if (ttyr_core_getCurrentProgram(&MacroTab_p->MicroWindow) != NULL) {
        TTYR_CHECK(ttyr_core_getCurrentProgram(&MacroTab_p->MicroWindow)->Prototype_p->Callbacks.handleInput_f(
            ttyr_core_getCurrentProgram(&MacroTab_p->MicroWindow), Event
        ))
    }
    else {
        TTYR_CHECK(ttyr_core_setDefaultMessage(&MacroTab_p->Topbar, TTYR_CORE_MESSAGE_MISC_NO_CURRENT_PROGRAM))
    }

    if (ttyr_core_getCurrentProgram(&MacroTab_p->MicroWindow)->refresh) {
        Window_p->Tile_p->refresh = true;
        ttyr_core_getCurrentProgram(&MacroTab_p->MicroWindow)->refresh = false;
    }
    if (ttyr_core_getCurrentProgram(&MacroTab_p->MicroWindow)->close) {
        Window_p->Tile_p->refresh = true;
        Window_p->Tile_p->close = true;
    }

    return TTYR_CORE_SUCCESS;
}

static TTYR_CORE_RESULT ttyr_core_handleMouseInput(
    ttyr_core_MacroWindow *Window_p, nh_api_WSIEvent Event, int col, int row)
{
    ttyr_core_TTY *TTY_p = nh_core_getWorkloadArg();
    ttyr_core_Tile *MacroTile_p = NULL, *MicroTile_p = NULL;
    nh_core_List MacroTiles = ttyr_core_getTiles(Window_p->RootTile_p);

    ttyr_core_Config Config = ttyr_core_getConfig();

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
                if (!MacroTile_p->p || !TTYR_CORE_MACRO_TAB(MacroTile_p)->MicroWindow.Tabs_p) {break;}
            }
            if (!MacroTile_p->p || !TTYR_CORE_MACRO_TAB(MacroTile_p)->MicroWindow.Tabs_p) {continue;}
            nh_core_List MicroTiles = ttyr_core_getTiles(TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(MacroTile_p))->RootTile_p);
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
            nh_core_freeList(&MicroTiles, false);
            if (MicroTile_p) {break;}
        }
        MacroTile_p = NULL;
        MicroTile_p = NULL;
    }

    if (row == -1 && Config.Titlebar.on == true) {
        // Forward titlebar hit.
        if (Event.Mouse.trigger == NH_API_TRIGGER_PRESS) {
            ttyr_core_handleTitlebarHit(Event.Mouse, cCol);
            TTY_p->Window_p->refreshGrid2 = true;
        }
    }
 
    if (MacroTile_p == NULL || MicroTile_p == NULL) {return TTYR_CORE_SUCCESS;}

    // Handle mouse-menu input.
    if (Window_p->MouseMenu_p) {
        ttyr_core_ContextMenu *Hit_p = 
            ttyr_core_isContextMenuHit(Window_p->MouseMenu_p, NULL, true, col, Config.Titlebar.on ? row+1 : row);
        if (Hit_p) {
            if (Event.Mouse.type == NH_API_MOUSE_LEFT && Event.Mouse.trigger == NH_API_TRIGGER_PRESS) {
                ttyr_core_handleMouseMenuPress(Window_p->MouseMenu_p, Hit_p);
                if (Window_p->MouseMenu_p) {
                    ttyr_core_freeContextMenu(Window_p->MouseMenu_p);
                    Window_p->MouseMenu_p = NULL; // Otherwise we end up with an invalid pointer!
                }
            }
            if (Event.Mouse.trigger == NH_API_TRIGGER_MOVE) {
                ttyr_core_updateContextMenuHit(Window_p->MouseMenu_p, NULL, col, Config.Titlebar.on ? row+1 : row, false);
            }
            Window_p->refreshGrid2 = true;
            ((ttyr_core_TTY*)nh_core_getWorkloadArg())->Window_p->refreshGrid2 = true; // Focused Window might have changed.
            goto SKIP;
        }
    }

    // Close mouse-menu on left-click and switch tile.
    if (Event.Mouse.type == NH_API_MOUSE_LEFT && Event.Mouse.trigger == NH_API_TRIGGER_PRESS && MicroTile_p) {
        TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(MacroTile_p))->Tile_p = MicroTile_p;
        Window_p->Tile_p = MacroTile_p;

        if (Window_p->MouseMenu_p) {
            ttyr_core_freeContextMenu(Window_p->MouseMenu_p);
            Window_p->MouseMenu_p = NULL;
        }

        Window_p->refreshGrid2 = true;
    }

    // Create mouse-menu on right-click and switch tiles.
    if (Event.Mouse.type == NH_API_MOUSE_RIGHT && Event.Mouse.trigger == NH_API_TRIGGER_PRESS && MicroTile_p) {
        TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(MacroTile_p))->Tile_p = MicroTile_p;
        Window_p->Tile_p = MacroTile_p;

        if (Window_p->MouseMenu_p) {ttyr_core_freeContextMenu(Window_p->MouseMenu_p);}
        Window_p->MouseMenu_p = ttyr_core_createMouseMenu(col, Config.Titlebar.on ? row+1 : row);
        TTYR_CHECK_NULL(Window_p->MouseMenu_p)
        Window_p->MouseMenu_p->active = true;
        Window_p->MouseMenu_p->cCol = cCol;
        Window_p->MouseMenu_p->cRow = cRow;

        Window_p->refreshGrid2 = true;
    }

    // Close mouse-menu on scroll.
    if (Event.Mouse.type == NH_API_MOUSE_SCROLL && Window_p->MouseMenu_p) {
        ttyr_core_freeContextMenu(Window_p->MouseMenu_p);
        Window_p->MouseMenu_p = NULL;
        Window_p->refreshGrid2 = true;
    }

    if (cRow == 0) {
        // Forward topbar hit.
        Event.Mouse.Position.x = cCol2;
        TTYR_CHECK(ttyr_core_handleTopbarInput(MacroTile_p, Event))
    } else if (MicroTile_p && TTYR_CORE_MICRO_TILE(MicroTile_p)->Program_p) {
        // Forward program hit.
        Event.Window.Position.x = Event.Mouse.Position.x;
        Event.Window.Position.y = Event.Mouse.Position.y;
        Event.Mouse.Position.x = cCol;
        Event.Mouse.Position.y = cRow - 1; // Subtract topbar. 
        TTYR_CHECK(TTYR_CORE_MICRO_TILE(MicroTile_p)->Program_p->Prototype_p->Callbacks.handleInput_f(
            TTYR_CORE_MICRO_TILE(MicroTile_p)->Program_p, Event
        ))
    }

    if (cRow != 0) {
        for (int i = 0; i < MacroTiles.size; ++i) {
            if (((ttyr_core_Tile*)MacroTiles.pp[i])->Children.count > 0) {continue;}
            TTYR_CORE_MACRO_TAB(MacroTiles.pp[i])->Topbar.quitHover = false;
        }
    }

SKIP:

    nh_core_freeList(&MacroTiles, false);

    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT ttyr_core_handleMacroWindowInput(
    ttyr_core_MacroWindow *Window_p, nh_api_WSIEvent Event)
{
    ttyr_core_Config Config = ttyr_core_getConfig();

    switch (Event.type)
    {
        case NH_API_WSI_EVENT_KEYBOARD :
        {
            TTYR_CHECK(ttyr_core_handleKeyboardInput(Window_p, Event))
            break;
        }
        case NH_API_WSI_EVENT_MOUSE :
        {
            int col = -1;
            int row = -1;
            ttyr_core_translateMousePosition(((ttyr_core_TTY*)nh_core_getWorkloadArg())->Views.pp[0], Event.Mouse, &col, &row);
            if (Config.Titlebar.on) {row--;}
            if (col < 0) {break;}
            TTYR_CHECK(ttyr_core_handleMouseInput(Window_p, Event, col, row))
            break;
        }
        case NH_API_WSI_EVENT_WINDOW :
        {
            ttyr_core_TTY *TTY_p = nh_core_getWorkloadArg();
            if (Event.Window.type == NH_API_WINDOW_FOCUS_OUT) {
                TTY_p->alt = false;
                TTY_p->ctrl = false;
                TTY_p->hasFocus = false;
            }
            if (Event.Window.type == NH_API_WINDOW_FOCUS_IN) {
                TTY_p->hasFocus = true;
            }
            break;
        }
    }

    Window_p->refreshGrid1 = true;
    Window_p->refreshGrid2 = true;
    Window_p->refreshCursor = true;
 
    return TTYR_CORE_SUCCESS;
}
