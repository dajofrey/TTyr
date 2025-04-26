// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Terminal.h"

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

#include <string.h>
#include <stdio.h>

// INIT/FREE =======================================================================================
// The next functions comprise the in/exit points of nhterminal.

typedef struct ttyr_terminal_Args {
    ttyr_core_TTY *TTY_p;
    char *namespace_p;
} ttyr_terminal_Args;

static void *ttyr_terminal_initTerminal(
    nh_core_Workload *Workload_p)
{
    static char *name_p = "Terminal Emulator";
    static char *path_p = "nhterminal/Terminal/Terminal.c";

    Workload_p->name_p = name_p;
    Workload_p->path_p = path_p;
    Workload_p->module = -1;

    ttyr_terminal_Terminal *Terminal_p = nh_core_allocate(sizeof(ttyr_terminal_Terminal));
    TTYR_TERMINAL_CHECK_MEM_2(NULL, Terminal_p)

    memset(Terminal_p, 0, sizeof(ttyr_terminal_Terminal));

    Terminal_p->ctrl = false;
    Terminal_p->leftMouse = false;
    Terminal_p->TTY_p = ((ttyr_terminal_Args*)Workload_p->args_p)->TTY_p;
    Terminal_p->View_p = ttyr_core_createView(Terminal_p->TTY_p, Terminal_p, false);
    TTYR_TERMINAL_CHECK_MEM_2(NULL, Terminal_p->View_p)

    if (((ttyr_terminal_Args*)Workload_p->args_p)->namespace_p) {
        strcpy(Terminal_p->namespace_p, ((ttyr_terminal_Args*)Workload_p->args_p)->namespace_p);
    } else {
        sprintf(Terminal_p->namespace_p, "%p", Terminal_p);
    }

    TTYR_TERMINAL_CHECK_2(NULL, ttyr_terminal_initGraphics(&Terminal_p->Graphics))
 
    TTYR_TERMINAL_CHECK_2(NULL, ttyr_terminal_initGrid(&Terminal_p->Grid))
    TTYR_TERMINAL_CHECK_2(NULL, ttyr_terminal_initGrid(&Terminal_p->Grid2))

    return Terminal_p;
}

static void ttyr_terminal_freeTerminal(
    void *terminal_p)
{
    ttyr_terminal_Terminal *Terminal_p = terminal_p;

    ttyr_terminal_freeGraphics(&Terminal_p->Graphics);
    ttyr_terminal_freeGrid(&Terminal_p->Grid);
    ttyr_terminal_freeGrid(&Terminal_p->Grid2);

    nh_core_free(Terminal_p);
}

// RUN LOOP ========================================================================================
// The next functions comprise the top-level of the nhterminal run loop.

static TTYR_TERMINAL_RESULT ttyr_terminal_updateSize(
    ttyr_terminal_Terminal *Terminal_p)
{
    ttyr_terminal_Config Config = ttyr_terminal_getConfig();
 
    nh_gfx_FontInstance *FontInstance_p = nh_gfx_claimFontInstance(
        Terminal_p->Graphics.State.Fonts.pp[Terminal_p->Graphics.State.font], 
        Config.fontSize
    );

    nh_gfx_Text Text;
    NH_API_UTF32 c = 'e';

    NH_API_RESULT failure = 1;
    for (int i = 0; i < Terminal_p->Graphics.State.Fonts.size; ++i) {
        failure = nh_gfx_createTextFromFont(&Text, &c, 1, Config.fontSize,
            Terminal_p->Graphics.State.Fonts.pp[i]);
        if (!failure) {
            Terminal_p->Graphics.State.font = i;
            break;
        }
    }

    if (failure) {return TTYR_TERMINAL_ERROR_BAD_STATE;}
    
    TTYR_TERMINAL_CHECK(ttyr_terminal_updateGrid(&Terminal_p->Grid, &Terminal_p->Graphics.State, &Text))
    TTYR_TERMINAL_CHECK(ttyr_terminal_updateGrid(&Terminal_p->Grid2, &Terminal_p->Graphics.State, &Text))
    TTYR_TERMINAL_CHECK(ttyr_terminal_updateBorderGrid(&Terminal_p->BorderGrid, &Terminal_p->Graphics.State, &Text))

    TTYR_TERMINAL_CHECK(ttyr_terminal_updateGraphicsData(&Terminal_p->Graphics.State, &Terminal_p->Graphics.Data1, &Terminal_p->Grid, 1))
    TTYR_TERMINAL_CHECK(ttyr_terminal_updateGraphicsData(&Terminal_p->Graphics.State, &Terminal_p->Graphics.Data2, &Terminal_p->Grid2, 1))
    TTYR_TERMINAL_CHECK(ttyr_terminal_updateGraphicsData(&Terminal_p->Graphics.State, &Terminal_p->Graphics.BorderData, &Terminal_p->BorderGrid, 0))

    // Update view size, subtract gap tiles.
    Terminal_p->View_p->cols = Terminal_p->Grid.cols-1;
    Terminal_p->View_p->rows = Terminal_p->Grid.rows-1;
    Terminal_p->View_p->Size = Terminal_p->Grid.Size;
    Terminal_p->View_p->TileSize = Terminal_p->Grid.TileSize;

    TTYR_TERMINAL_CHECK(ttyr_terminal_updateGraphicsData(&Terminal_p->Graphics.State, &Terminal_p->Graphics.Data1, &Terminal_p->Grid, 1))
    TTYR_TERMINAL_CHECK(ttyr_terminal_updateGraphicsData(&Terminal_p->Graphics.State, &Terminal_p->Graphics.Data2, &Terminal_p->Grid2, 1))
    TTYR_TERMINAL_CHECK(ttyr_terminal_updateGraphicsData(&Terminal_p->Graphics.State, &Terminal_p->Graphics.BorderData, &Terminal_p->BorderGrid, 0))

    nh_gfx_freeText(&Text);

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_updateSizeIfRequired(
    ttyr_terminal_Terminal *Terminal_p, bool *update_p)
{
    ttyr_terminal_Config Config = ttyr_terminal_getConfig();

    if (Terminal_p->Graphics.State.Viewport_p->Settings.Size.width - (Terminal_p->Grid.borderPixel*2) == Terminal_p->Grid.Size.width
    &&  Terminal_p->Graphics.State.Viewport_p->Settings.Size.height - (Terminal_p->Grid.borderPixel*2) == Terminal_p->Grid.Size.height) {
        return TTYR_TERMINAL_SUCCESS;
    }

    TTYR_TERMINAL_CHECK(ttyr_terminal_updateSize(Terminal_p))

    *update_p = true;

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_handleEvent(
    ttyr_terminal_Terminal *Terminal_p, nh_api_WSIEvent *Event_p)
{
    ttyr_terminal_Config Config = ttyr_terminal_getConfig();
 
    if (Event_p->type == NH_API_WSI_EVENT_MOUSE) {
        if (Event_p->Mouse.type == NH_API_MOUSE_SCROLL && Terminal_p->ctrl) {
            int newFontSize = Config.fontSize + (Event_p->Mouse.trigger == NH_API_TRIGGER_UP ? 1 : -1);

            if (newFontSize < 10 || newFontSize > 60) {
                // Out of bounds.
                return TTYR_TERMINAL_SUCCESS;
            }

            nh_core_overwriteGlobalConfigSettingInt(
                Terminal_p->namespace_p, -1, ttyr_terminal_getSettingName(TTYR_TERMINAL_SETTING_FONT_SIZE), newFontSize);

            nh_gfx_unclaimFontInstance(nh_gfx_claimFontInstance(
                Terminal_p->Graphics.State.Fonts.pp[Terminal_p->Graphics.State.font], 
                newFontSize));

            TTYR_TERMINAL_CHECK(ttyr_terminal_updateSize(Terminal_p))
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
                nh_api_moveWindow(Terminal_p->Graphics.State.Viewport_p->Surface_p->Window_p);
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

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_handleInputIfRequired(
    ttyr_terminal_Terminal *Terminal_p, bool *update_p)
{
    nh_core_Array *Array_p = NULL;
    nh_api_WSIEvent *Event_p = NULL;

    for (int row = 0; row < Terminal_p->Grid.rows; ++row) {
        memset(Terminal_p->Grid.updates_pp[row], false, Terminal_p->Grid.cols*sizeof(bool));
    }
    for (int row = 0; row < Terminal_p->Grid2.rows; ++row) {
        memset(Terminal_p->Grid2.updates_pp[row], false, Terminal_p->Grid2.cols*sizeof(bool));
    }

    do {
        Array_p = nh_core_incrementRingBufferMarker(
            &Terminal_p->View_p->Forward.Tiles, &Terminal_p->View_p->Forward.Tiles.Marker);

        if (!Array_p) {break;}

        for (int i = 0; i < Array_p->length; ++i) {
            ttyr_terminal_TileUpdate *Update_p = ((ttyr_terminal_TileUpdate*)Array_p->p)+i;
            if (Update_p->row >= Terminal_p->Grid.rows || Update_p->col >= Terminal_p->Grid.cols) {
                // Ignore invalid update.
                continue;
            }
            if (Update_p->cursor) {
                TTYR_TERMINAL_CHECK(ttyr_terminal_updateTile(
                    &Terminal_p->Grid, &Terminal_p->Graphics.State, Update_p, update_p))
            } else if (Update_p->Glyph.mark & TTYR_CORE_MARK_ELEVATED) {
                Terminal_p->Grid2.Updates_pp[Update_p->row][Update_p->col] = *Update_p;
                Terminal_p->Grid2.updates_pp[Update_p->row][Update_p->col] = true;
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
            TTYR_TERMINAL_CHECK(ttyr_terminal_updateTile(
                &Terminal_p->Grid, &Terminal_p->Graphics.State, &Terminal_p->Grid.Updates_pp[row][col], update_p))
        }
    }
    for (int row = 0; row < Terminal_p->Grid2.rows; ++row) {
        for (int col = 0; col < Terminal_p->Grid2.cols; ++col) {
            if (Terminal_p->Grid2.updates_pp[row][col] == false) {continue;}
            TTYR_TERMINAL_CHECK(ttyr_terminal_updateTile(
                &Terminal_p->Grid2, &Terminal_p->Graphics.State, &Terminal_p->Grid2.Updates_pp[row][col], update_p))
        }
    }

    do {
        Array_p = nh_core_incrementRingBufferMarker(
            &Terminal_p->View_p->Forward.Boxes, &Terminal_p->View_p->Forward.Boxes.Marker);

        if (!Array_p) {break;}

        TTYR_TERMINAL_CHECK(ttyr_terminal_updateBoxes(&Terminal_p->Grid, &Terminal_p->Graphics.State, Array_p))

    } while (Array_p);

    do {
        Event_p = nh_core_incrementRingBufferMarker(
            &Terminal_p->View_p->Forward.Events, &Terminal_p->View_p->Forward.Events.Marker);

        if (!Event_p) {break;}

        TTYR_TERMINAL_CHECK(ttyr_terminal_handleEvent(Terminal_p, Event_p))

    } while (Event_p);

    return TTYR_TERMINAL_SUCCESS;
}

static NH_SIGNAL ttyr_terminal_runTerminal(
    void *args_p) 
{
    TTYR_TERMINAL_CHECK_NULL_2(NH_SIGNAL_ERROR, args_p)
    ttyr_terminal_Terminal *Terminal_p = args_p;

    if (!Terminal_p->Graphics.State.Viewport_p) {return NH_SIGNAL_IDLE;}

    bool update = false;

    TTYR_TERMINAL_CHECK_2(NH_SIGNAL_ERROR, ttyr_terminal_updateSizeIfRequired(Terminal_p, &update))
    TTYR_TERMINAL_CHECK_2(NH_SIGNAL_ERROR, ttyr_terminal_handleInputIfRequired(Terminal_p, &update))
    if (ttyr_terminal_updateBlinkOrGradient(&Terminal_p->Graphics)) {
        update = true;
    }

    Terminal_p->Graphics.State.Viewport_p->Settings.BorderColor.r = Terminal_p->Graphics.State.AccentGradient.Color.r;
    Terminal_p->Graphics.State.Viewport_p->Settings.BorderColor.g = Terminal_p->Graphics.State.AccentGradient.Color.g;
    Terminal_p->Graphics.State.Viewport_p->Settings.BorderColor.b = Terminal_p->Graphics.State.AccentGradient.Color.b;
 
    if (update) {
        TTYR_TERMINAL_CHECK_2(NH_SIGNAL_ERROR, ttyr_terminal_updateGraphicsData(&Terminal_p->Graphics.State, &Terminal_p->Graphics.Data1,
            &Terminal_p->Grid, 1))
        TTYR_TERMINAL_CHECK_2(NH_SIGNAL_ERROR, ttyr_terminal_updateGraphicsData(&Terminal_p->Graphics.State, &Terminal_p->Graphics.Data2,
            &Terminal_p->Grid2, 1))
        TTYR_TERMINAL_CHECK_2(NH_SIGNAL_ERROR, ttyr_terminal_updateGraphicsData(&Terminal_p->Graphics.State, &Terminal_p->Graphics.BorderData, 
            &Terminal_p->BorderGrid, 0))
        TTYR_TERMINAL_CHECK_2(NH_SIGNAL_ERROR, ttyr_terminal_renderGraphics(&Terminal_p->Graphics, 
            &Terminal_p->Grid, &Terminal_p->Grid2, &Terminal_p->BorderGrid))
        return NH_SIGNAL_OK;
    }

    return update == true ? NH_SIGNAL_OK : NH_SIGNAL_IDLE;
}

// COMMANDS ========================================================================================
// The next functions are executed by ttyr_terminal_cmd_* functions.

typedef enum TTYR_TERMINAL_COMMAND_E {
    TTYR_TERMINAL_COMMAND_SET_VIEWPORT,
} TTYR_TERMINAL_COMMAND_E;

/**
 * Here, most commands that came in through the API are handled.
 */
static NH_SIGNAL ttyr_terminal_runTerminalCommand(
    void *terminal_p, nh_core_WorkloadCommand *Command_p)
{
    ttyr_terminal_Terminal *Terminal_p = terminal_p;

    switch (Command_p->type)
    {
        case TTYR_TERMINAL_COMMAND_SET_VIEWPORT :
            if (nh_gfx_claimViewport(Command_p->p, NH_GFX_VIEWPORT_OWNER_TERMINAL, Terminal_p) != NH_API_SUCCESS) {
                return NH_SIGNAL_ERROR;
            }
            ttyr_terminal_handleViewportChange(&Terminal_p->Graphics, Command_p->p);
            ttyr_terminal_handleViewportChange(&Terminal_p->Graphics2, Command_p->p);
            break;
    }

    return NH_SIGNAL_OK;
}

// API =============================================================================================
// The next functions are called by lib/netzhaut/nhterminal.h functions.

ttyr_terminal_Terminal *ttyr_terminal_openTerminal(
    char *namespace_p, ttyr_core_TTY *TTY_p)
{
    ttyr_terminal_Args Args;
    Args.TTY_p = TTY_p;
    Args.namespace_p = namespace_p;

    ttyr_terminal_Terminal *Terminal_p = nh_core_activateWorkload(
        ttyr_terminal_initTerminal, ttyr_terminal_runTerminal, ttyr_terminal_freeTerminal,
        ttyr_terminal_runTerminalCommand, &Args, true);

    return Terminal_p;
}

TTYR_TERMINAL_RESULT ttyr_terminal_cmd_setViewport(
    ttyr_terminal_Terminal *Terminal_p, nh_gfx_Viewport *Viewport_p)
{
    nh_core_executeWorkloadCommand(Terminal_p, TTYR_TERMINAL_COMMAND_SET_VIEWPORT, Viewport_p, 0);

    return TTYR_TERMINAL_SUCCESS;
}
