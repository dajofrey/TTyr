// LICENSE NOTICE ==================================================================================

/**
 * Termoskanne - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Terminal.h"
#include "Vertices.h"

#include "../Common/Macros.h"

#include "nh-core/System/Thread.h"
#include "nh-core/System/Memory.h"
#include "nh-core/System/Process.h"
#include "nh-core/Util/RingBuffer.h"
#include "nh-core/Util/List.h"
#include "nh-core/Config/Config.h"
#include "nh-encoding/Encodings/UTF8.h"
#include "nh-encoding/Encodings/UTF32.h"
#include "nh-gfx/Base/Viewport.h"
#include "nh-gfx/Fonts/FontManager.h"
#include "nh-gfx/Fonts/Text.h"

#include "nh-wsi/Window/Window.h"
#include "nh-wsi/Window/WindowSettings.h"

#include "../../tk-core/TTY/TTY.h"

#include <string.h>
#include <stdio.h>

// INIT/FREE =======================================================================================
// The next functions comprise the in/exit points of nhterminal.

typedef struct tk_terminal_Args {
    tk_core_TTY *TTY_p;
    char *namespace_p;
} tk_terminal_Args;

static void *tk_terminal_initTerminal(
    nh_core_Workload *Workload_p)
{
    static char *name_p = "Terminal Emulator";
    static char *path_p = "tk-terminal/Terminal/Terminal.c";

    Workload_p->name_p = name_p;
    Workload_p->path_p = path_p;
    Workload_p->module = -1;

    tk_terminal_Terminal *Terminal_p = (tk_terminal_Terminal*)nh_core_allocate(sizeof(tk_terminal_Terminal));
    TK_TERMINAL_CHECK_MEM_2(NULL, Terminal_p)

    memset(Terminal_p, 0, sizeof(tk_terminal_Terminal));

    tk_terminal_updateConfig(Terminal_p);

    Terminal_p->ctrl = false;
    Terminal_p->leftMouse = false;
    Terminal_p->TTY_p = ((tk_terminal_Args*)Workload_p->args_p)->TTY_p;
    Terminal_p->View_p = tk_core_createView(Terminal_p->TTY_p, Terminal_p, false);

    TK_TERMINAL_CHECK_MEM_2(NULL, Terminal_p->View_p)

    if (((tk_terminal_Args*)Workload_p->args_p)->namespace_p) {
        strcpy(Terminal_p->namespace_p, ((tk_terminal_Args*)Workload_p->args_p)->namespace_p);
    } else {
        sprintf(Terminal_p->namespace_p, "%p", Terminal_p);
    }

    TK_TERMINAL_CHECK_2(NULL, tk_terminal_initGraphics(&Terminal_p->Config, &Terminal_p->Graphics))
 
    TK_TERMINAL_CHECK_2(NULL, tk_terminal_initGrid(&Terminal_p->Grid))
    TK_TERMINAL_CHECK_2(NULL, tk_terminal_initGrid(&Terminal_p->ElevatedGrid))

    return Terminal_p;
}

static void tk_terminal_freeTerminal(
    void *terminal_p)
{
    tk_terminal_Terminal *Terminal_p = terminal_p;

    tk_terminal_freeGraphics(&Terminal_p->Graphics);
    tk_terminal_freeGrid(&Terminal_p->Grid);
    tk_terminal_freeGrid(&Terminal_p->ElevatedGrid);
    tk_terminal_freeGrid(&Terminal_p->BackdropGrid);

    nh_gfx_freeText(&Terminal_p->Text);

    nh_core_free(Terminal_p);
}

// RUN LOOP ========================================================================================
// The next functions comprise the top-level of the nhterminal run loop.

static TK_TERMINAL_RESULT tk_terminal_updateBoxes( 
    tk_terminal_Terminal *Terminal_p, nh_core_Array *NewBoxes_p, int fontSize)
{ 
    if (Terminal_p->Graphics.Boxes.Data.length > 0) { 
        nh_core_freeArray(&Terminal_p->Graphics.Boxes.Data); 
    } 

    for (int i = 0; i < NewBoxes_p->length; ++i) 
    { 
        tk_terminal_Box *Box_p = (tk_terminal_Box*)nh_core_incrementArray(&Terminal_p->Graphics.Boxes.Data);
        TK_TERMINAL_CHECK_MEM(Box_p) 
        *Box_p = ((tk_terminal_Box*)NewBoxes_p->p)[i]; 

        if (Box_p->UpperLeft.x == Box_p->LowerRight.x && Box_p->UpperLeft.y == Box_p->UpperLeft.y) {
            TK_TERMINAL_CHECK(tk_terminal_getOutlineVertices(&Terminal_p->Graphics.State, &Terminal_p->Grid, Box_p, true, fontSize))
            TK_TERMINAL_CHECK(tk_terminal_getOutlineVertices(&Terminal_p->Graphics.State, &Terminal_p->Grid, Box_p, false, fontSize))
        } else { 
            TK_TERMINAL_CHECK(tk_terminal_getBoxVertices(&Terminal_p->Graphics.State, &Terminal_p->Grid, Box_p, true, fontSize))
            TK_TERMINAL_CHECK(tk_terminal_getBoxVertices(&Terminal_p->Graphics.State, &Terminal_p->Grid, Box_p, false, fontSize))
        } 
    } 

    return TK_TERMINAL_SUCCESS; 
} 

static TK_TERMINAL_RESULT tk_terminal_updateSize(
    tk_terminal_Terminal *Terminal_p)
{
    nh_gfx_FontInstance *FontInstance_p = nh_gfx_claimFontInstance(
        Terminal_p->Graphics.State.Fonts.pp[Terminal_p->Graphics.State.font], 
        Terminal_p->Config.fontSize
    );

    nh_gfx_freeText(&Terminal_p->Text);

    NH_API_UTF32 c = 'e';
    NH_API_RESULT failure = 1;
    for (int i = 0; i < Terminal_p->Graphics.State.Fonts.size; ++i) {
        failure = nh_gfx_createTextFromFont(&Terminal_p->Text, &c, 1, Terminal_p->Config.fontSize,
            Terminal_p->Graphics.State.Fonts.pp[i]);
        if (!failure) {
            Terminal_p->Graphics.State.font = i;
            break;
        }
    }

    if (failure) {return TK_TERMINAL_ERROR_BAD_STATE;}
    
    TK_TERMINAL_CHECK(tk_terminal_updateGrid(&Terminal_p->Config, &Terminal_p->Grid, &Terminal_p->Graphics.State, &Terminal_p->Text))
    TK_TERMINAL_CHECK(tk_terminal_updateGrid(&Terminal_p->Config, &Terminal_p->ElevatedGrid, &Terminal_p->Graphics.State, &Terminal_p->Text))
    TK_TERMINAL_CHECK(tk_terminal_updateBackdropGrid(&Terminal_p->Config, &Terminal_p->BackdropGrid, &Terminal_p->Graphics.State, &Terminal_p->Text))

    // Update view size, subtract gap tiles.
    Terminal_p->View_p->cols = Terminal_p->Grid.cols-1;
    Terminal_p->View_p->rows = Terminal_p->Grid.rows-1;
    Terminal_p->View_p->Size = Terminal_p->Grid.Size;
    Terminal_p->View_p->TileSize = Terminal_p->Grid.TileSize;

    Terminal_p->Graphics.BackdropData.update = true;
    Terminal_p->Graphics.ElevatedData.update = true;

    TK_TERMINAL_CHECK(tk_terminal_updateGraphics(
        &Terminal_p->Config, &Terminal_p->Graphics, &Terminal_p->Grid, &Terminal_p->BackdropGrid, 
        &Terminal_p->ElevatedGrid, Terminal_p->TTY_p->Config.Titlebar.on))

    return TK_TERMINAL_SUCCESS;
}

static TK_TERMINAL_RESULT tk_terminal_updateSizeIfRequired(
    tk_terminal_Terminal *Terminal_p, bool *update_p)
{
    if (Terminal_p->Graphics.State.Viewport_p->Settings.Size.width - (Terminal_p->Grid.borderPixel*2) == Terminal_p->Grid.Size.width
    &&  Terminal_p->Graphics.State.Viewport_p->Settings.Size.height - (Terminal_p->Grid.borderPixel*2) == Terminal_p->Grid.Size.height) {
        return TK_TERMINAL_SUCCESS;
    }

    TK_TERMINAL_CHECK(tk_terminal_updateSize(Terminal_p))

    *update_p = true;

    return TK_TERMINAL_SUCCESS;
}

static TK_TERMINAL_RESULT tk_terminal_handleEvent(
    tk_terminal_Terminal *Terminal_p, nh_api_WSIEvent *Event_p)
{
    if (Event_p->type == NH_API_WSI_EVENT_MOUSE) {
        if (Event_p->Mouse.type == NH_API_MOUSE_SCROLL && Terminal_p->ctrl) {
            int newFontSize = Terminal_p->Config.fontSize + (Event_p->Mouse.trigger == NH_API_TRIGGER_UP ? 1 : -1);

            if (newFontSize < 10 || newFontSize > 60) {
                // Out of bounds.
                return TK_TERMINAL_SUCCESS;
            }

            nh_core_overwriteGlobalConfigSettingInt(
                Terminal_p->namespace_p, -1, "tk-terminal.font.size", newFontSize);
            tk_terminal_updateConfig(Terminal_p);

            nh_gfx_unclaimFontInstance(nh_gfx_claimFontInstance(
                Terminal_p->Graphics.State.Fonts.pp[Terminal_p->Graphics.State.font], 
                newFontSize));

            TK_TERMINAL_CHECK(tk_terminal_updateSize(Terminal_p))
        }
        if (Event_p->Mouse.type == NH_API_MOUSE_LEFT && Event_p->Mouse.trigger == NH_API_TRIGGER_PRESS) {
            Terminal_p->leftMouse = true;
            nh_core_SystemTime Now = nh_core_getSystemTime();
            if (nh_core_getSystemTimeDiffInSeconds(Terminal_p->LastClick, Now) <= 0.3f) {
                nh_wsi_toggleWindowSize_f toggleWindowSize_f = nh_core_loadExistingSymbol(NH_MODULE_WSI, 0, "nh_wsi_toggleWindowSize");
                if (toggleWindowSize_f && Event_p->Mouse.Position.y < Terminal_p->Grid.TileSize.height) {
                    toggleWindowSize_f(Terminal_p->Graphics.State.Viewport_p->Surface_p->Window_p);
                } 
            }
            Terminal_p->LastClick = Now;
        } 
        if (Event_p->Mouse.type == NH_API_MOUSE_LEFT && Event_p->Mouse.trigger == NH_API_TRIGGER_RELEASE) {
            Terminal_p->leftMouse = false;
        } 
        if (Event_p->Mouse.type == NH_API_MOUSE_MOVE && Terminal_p->leftMouse) {
            if (Event_p->Mouse.Position.y < Terminal_p->Grid.TileSize.height) {
                nh_api_moveWindow((nh_api_Window*)Terminal_p->Graphics.State.Viewport_p->Surface_p->Window_p);
                if (Terminal_p->Graphics.State.Viewport_p->Surface_p->Window_p->type == NH_WSI_TYPE_X11) {
                    Terminal_p->leftMouse = false;
                }
            } 
        }
    }

    if (Event_p->type == NH_API_WSI_EVENT_KEYBOARD) {
        if (Event_p->Keyboard.special == NH_API_KEY_CONTROL_L || Event_p->Keyboard.special == NH_API_KEY_CONTROL_R) {
            if (Event_p->Keyboard.trigger == NH_API_TRIGGER_PRESS) {
                Terminal_p->ctrl = true;
            }
            if (Event_p->Keyboard.trigger == NH_API_TRIGGER_RELEASE) {
                Terminal_p->ctrl = false;
            }
        }
    }

    if (Event_p->type == NH_API_WSI_EVENT_WINDOW) {
        Terminal_p->leftMouse = false;
    }

    return TK_TERMINAL_SUCCESS;
}

static TK_TERMINAL_RESULT tk_terminal_handleInputIfRequired(
    tk_terminal_Terminal *Terminal_p, bool *update_p)
{
    nh_core_Array *Array_p = NULL;
    nh_api_WSIEvent *Event_p = NULL;

    for (int row = 0; row < Terminal_p->Grid.rows; ++row) {
        memset(Terminal_p->Grid.updates_pp[row], false, Terminal_p->Grid.cols*sizeof(bool));
    }
    for (int row = 0; row < Terminal_p->ElevatedGrid.rows; ++row) {
        memset(Terminal_p->ElevatedGrid.updates_pp[row], false, Terminal_p->ElevatedGrid.cols*sizeof(bool));
    }

    do {
        Array_p = (nh_core_Array*)nh_core_incrementRingBufferMarker(
            &Terminal_p->View_p->Forward.Tiles, &Terminal_p->View_p->Forward.Tiles.Marker);
        if (!Array_p) {break;}
        for (int i = 0; i < Array_p->length; ++i) {
            tk_terminal_TileUpdate *Update_p = ((tk_terminal_TileUpdate*)Array_p->p)+i;
            if (Update_p->row >= Terminal_p->Grid.rows || Update_p->col >= Terminal_p->Grid.cols) {
                // Ignore invalid update.
                continue;
            }
            if (Update_p->cursor) {
                TK_TERMINAL_CHECK(tk_terminal_updateTile(
                    &Terminal_p->Grid, &Terminal_p->Graphics.State, Update_p, update_p, Terminal_p->Config.fontSize))
            } else if (Update_p->Glyph.mark & TK_CORE_MARK_ELEVATED) {
                Terminal_p->ElevatedGrid.Updates_pp[Update_p->row][Update_p->col] = *Update_p;
                Terminal_p->ElevatedGrid.updates_pp[Update_p->row][Update_p->col] = true;
            } else {
                Terminal_p->Grid.Updates_pp[Update_p->row][Update_p->col] = *Update_p;
                Terminal_p->Grid.updates_pp[Update_p->row][Update_p->col] = true;
            }
        }
    } while (Array_p);

    // Update tiles.
    for (int row = 0; row < Terminal_p->Grid.rows; ++row) {
        for (int col = 0; col < Terminal_p->Grid.cols; ++col) {
            if (Terminal_p->Grid.updates_pp[row][col] == false) {continue;}
            TK_TERMINAL_CHECK(tk_terminal_updateTile(
                &Terminal_p->Grid, &Terminal_p->Graphics.State, &Terminal_p->Grid.Updates_pp[row][col],
                update_p, Terminal_p->Config.fontSize))
            // update backdrop grid tile if necessarry
            if (row == 0 && Terminal_p->TTY_p->Config.Topbar.on && !Terminal_p->TTY_p->Config.Titlebar.on) {
                tk_terminal_TileUpdate Update = Terminal_p->Grid.Updates_pp[row][col];
                Update.row = 2;
                Update.col += 1;
                bool updateBackdrop = false;
                if (Update.Glyph.codepoint == 'x') {
                    TK_TERMINAL_CHECK(tk_terminal_updateTile(
                        &Terminal_p->BackdropGrid, &Terminal_p->Graphics.State, &Update, &updateBackdrop, Terminal_p->Config.fontSize))
                } else {
                    Update.Glyph.codepoint = 0;
                    Update.Glyph.mark = TK_CORE_MARK_ACCENT | TK_CORE_MARK_LINE_GRAPHICS;
                    Update.Glyph.Attributes.reverse = true;
                    TK_TERMINAL_CHECK(tk_terminal_updateTile(
                        &Terminal_p->BackdropGrid, &Terminal_p->Graphics.State, &Update, &updateBackdrop, Terminal_p->Config.fontSize))
                }
                if (updateBackdrop) {
                    Terminal_p->Graphics.BackdropData.update = true;
                    *update_p = true;
                }
            }
        }
    }

    for (int row = 0; row < Terminal_p->ElevatedGrid.rows; ++row) {
        for (int col = 0; col < Terminal_p->ElevatedGrid.cols; ++col) {
            if (Terminal_p->ElevatedGrid.updates_pp[row][col] == false) {continue;}
            bool update = false;
            TK_TERMINAL_CHECK(tk_terminal_updateTile(
                &Terminal_p->ElevatedGrid, &Terminal_p->Graphics.State, &Terminal_p->ElevatedGrid.Updates_pp[row][col],
                &update, Terminal_p->Config.fontSize))
            if (update) {
                *update_p = true;
                Terminal_p->Graphics.ElevatedData.update = true;
            }
        }
    }

    do {
        Array_p = (nh_core_Array*)nh_core_incrementRingBufferMarker(
            &Terminal_p->View_p->Forward.Boxes, &Terminal_p->View_p->Forward.Boxes.Marker);
        if (!Array_p) {break;}
        TK_TERMINAL_CHECK(tk_terminal_updateBoxes(Terminal_p, Array_p, Terminal_p->Config.fontSize))
    } while (Array_p);

    do {
        Event_p = (nh_api_WSIEvent*)nh_core_incrementRingBufferMarker(
            &Terminal_p->View_p->Forward.Events, &Terminal_p->View_p->Forward.Events.Marker);
        if (!Event_p) {break;}
        TK_TERMINAL_CHECK(tk_terminal_handleEvent(Terminal_p, Event_p))
    } while (Event_p);

    return TK_TERMINAL_SUCCESS;
}

static NH_SIGNAL tk_terminal_runTerminal(
    void *args_p) 
{
    TK_TERMINAL_CHECK_NULL_2(NH_SIGNAL_ERROR, args_p)
    tk_terminal_Terminal *Terminal_p = args_p;

    if (!Terminal_p->Graphics.State.Viewport_p) {return NH_SIGNAL_IDLE;}

    bool update = false;

    TK_TERMINAL_CHECK_2(NH_SIGNAL_ERROR, tk_terminal_updateSizeIfRequired(Terminal_p, &update))
    TK_TERMINAL_CHECK_2(NH_SIGNAL_ERROR, tk_terminal_handleInputIfRequired(Terminal_p, &update))
    if (tk_terminal_updateBlinkOrGradient(&Terminal_p->Config, &Terminal_p->Graphics.State)) {
        update = true;
    }

    Terminal_p->Graphics.State.Viewport_p->Settings.BorderColor.r = Terminal_p->Graphics.State.AccentGradient.Color.r;
    Terminal_p->Graphics.State.Viewport_p->Settings.BorderColor.g = Terminal_p->Graphics.State.AccentGradient.Color.g;
    Terminal_p->Graphics.State.Viewport_p->Settings.BorderColor.b = Terminal_p->Graphics.State.AccentGradient.Color.b;
 
    if (update) {
        TK_TERMINAL_CHECK_2(NH_SIGNAL_ERROR, tk_terminal_updateGraphics(
            &Terminal_p->Config, &Terminal_p->Graphics, &Terminal_p->Grid, &Terminal_p->BackdropGrid, 
            &Terminal_p->ElevatedGrid, Terminal_p->TTY_p->Config.Titlebar.on))
        TK_TERMINAL_CHECK_2(NH_SIGNAL_ERROR, tk_terminal_renderGraphics(&Terminal_p->Config, &Terminal_p->Graphics, 
            &Terminal_p->Grid, &Terminal_p->ElevatedGrid, &Terminal_p->BackdropGrid))
        return NH_SIGNAL_OK;
    }

    return update == true ? NH_SIGNAL_OK : NH_SIGNAL_IDLE;
}

// COMMANDS ========================================================================================
// The next functions are executed by tk_terminal_cmd_* functions.

typedef enum TK_TERMINAL_COMMAND_E {
    TK_TERMINAL_COMMAND_SET_VIEWPORT,
} TK_TERMINAL_COMMAND_E;

/**
 * Here, most commands that came in through the API are handled.
 */
static NH_SIGNAL tk_terminal_runTerminalCommand(
    void *terminal_p, nh_core_WorkloadCommand *Command_p)
{
    tk_terminal_Terminal *Terminal_p = terminal_p;

    switch (Command_p->type)
    {
        case TK_TERMINAL_COMMAND_SET_VIEWPORT :
            if (nh_gfx_claimViewport(Command_p->p, NH_GFX_VIEWPORT_OWNER_TERMINAL, Terminal_p) != NH_API_SUCCESS) {
                return NH_SIGNAL_ERROR;
            }
            tk_terminal_handleViewportChange(&Terminal_p->Graphics, Command_p->p);
            break;
    }

    return NH_SIGNAL_OK;
}

// API =============================================================================================
// The next functions are called by lib/netzhaut/nhterminal.h functions.

tk_terminal_Terminal *tk_terminal_openTerminal(
    char *namespace_p, tk_core_TTY *TTY_p)
{
    tk_terminal_Args Args;
    Args.TTY_p = TTY_p;
    Args.namespace_p = namespace_p;

    tk_terminal_Terminal *Terminal_p = nh_core_activateWorkload(
        tk_terminal_initTerminal, tk_terminal_runTerminal, tk_terminal_freeTerminal,
        tk_terminal_runTerminalCommand, &Args, true);

    return Terminal_p;
}

TK_TERMINAL_RESULT tk_terminal_cmd_setViewport(
    tk_terminal_Terminal *Terminal_p, nh_gfx_Viewport *Viewport_p)
{
    nh_core_executeWorkloadCommand(Terminal_p, TK_TERMINAL_COMMAND_SET_VIEWPORT, Viewport_p, 0);

    return TK_TERMINAL_SUCCESS;
}
