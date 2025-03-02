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

#include "TTY.h"
#include "View.h"
#include "StandardIO.h"
#include "Draw.h"
#include "Macro.h"
#include "ContextMenu.h"
#include "Titlebar.h"

#include "../Shell/Shell.h"
#include "../Common/Macros.h"

#include "nh-core/System/Thread.h"
#include "nh-core/System/Memory.h"
#include "nh-core/System/Process.h"
#include "nh-core/Logger/Logger.h"
#include "nh-core/Util/RingBuffer.h"

#include "nh-encoding/Base/UnicodeDataHelper.h"
#include "nh-encoding/Encodings/UTF8.h"
#include "nh-encoding/Encodings/UTF32.h"

#include <string.h>
#include <stdio.h>

// CLIPBOARD =======================================================================================

TTYR_CORE_RESULT ttyr_core_resetClipboard()
{
    ttyr_core_TTY *TTY_p = nh_core_getWorkloadArg();

    for (int i = 0; i < TTY_p->Clipboard.Lines.length; ++i) {
        nh_encoding_UTF32String *String_p = 
            &((nh_encoding_UTF32String*)TTY_p->Clipboard.Lines.p)[i];
        nh_encoding_freeUTF32(String_p);
    }

    nh_core_freeArray(&TTY_p->Clipboard.Lines);
    TTY_p->Clipboard.Lines = nh_core_initArray(sizeof(nh_encoding_UTF32String), 32);

    return TTYR_CORE_SUCCESS;
}

nh_encoding_UTF32String *ttyr_core_newClipboardLine()
{
    ttyr_core_TTY *TTY_p = nh_core_getWorkloadArg();

    nh_encoding_UTF32String *Line_p = nh_core_incrementArray(&TTY_p->Clipboard.Lines);

    TTYR_CHECK_NULL_2(NULL, Line_p)
    *Line_p = nh_encoding_initUTF32(32);

    return Line_p;
}

ttyr_core_Clipboard *ttyr_core_getClipboard()
{
    return &((ttyr_core_TTY*)nh_core_getWorkloadArg())->Clipboard;
}

// INIT/FREE =======================================================================================
// The next functions comprise the in/exit points of nhtty.

typedef struct ttyr_core_OpenTTY {
    char *config_p;
    ttyr_core_Interface *Interface_p;
} ttyr_core_OpenTTY;

static void *ttyr_core_initTTY(
    nh_core_Workload *Workload_p)
{
    ttyr_core_OpenTTY *Args_p = Workload_p->args_p;

    static char *path_p = "nhtty/TTY/TTY.c";
    static char *name_p = "TTY Workload";
    Workload_p->path_p = path_p;
    Workload_p->name_p = name_p;
    Workload_p->module = -1;

    ttyr_core_TTY *TTY_p = nh_core_allocate(sizeof(ttyr_core_TTY));
    TTYR_CHECK_MEM_2(NULL, TTY_p)

    memset(TTY_p, 0, sizeof(ttyr_core_TTY));

    if (Workload_p->args_p) {
        strcpy(TTY_p->namespace_p, Workload_p->args_p);
    } else {
        sprintf(TTY_p->namespace_p, "%p", TTY_p);
    }

    TTY_p->Windows = nh_core_initList(10);
    TTY_p->Views = nh_core_initList(8);
    TTY_p->Prototypes = nh_core_initList(8);
    TTY_p->Clipboard.Lines = nh_core_initArray(sizeof(nh_encoding_UTF32String), 32);

    TTY_p->hasFocus = true;
    TTY_p->Borders.on = true;
    TTY_p->Topbars.on = true;

    TTY_p->Preview.blink = true;
    TTY_p->Preview.LastBlink = nh_core_getSystemTime();

    TTYR_CHECK_2(NULL, nh_core_initRingBuffer(
        &TTY_p->Events, 128, sizeof(nh_api_WSIEvent), NULL 
    ))

    ttyr_core_Config Config = ttyr_core_getConfig();
//    for (int i = 0; i < Config.windows; i++) {
        TTYR_CHECK_NULL_2(NULL, ttyr_core_insertAndFocusWindow(TTY_p, 0))
//    }
//    TTY_p->Window_p = TTY_p->Windows.pp[0];

    if (Args_p->Interface_p == NULL) {
        Args_p->Interface_p = ttyr_core_createShellInterface();
    }
    ttyr_core_addProgram(TTY_p, Args_p->Interface_p, false);
 
    return TTY_p;
}

static void ttyr_core_freeTTY(
    void *p)
{
    ttyr_core_TTY *TTY_p = p;

    ttyr_core_destroyWindows(TTY_p);
 
    nh_core_freeArray(&(TTY_p->Clipboard.Lines));
    nh_core_freeRingBuffer(&(TTY_p->Events));

    if (ttyr_core_claimsStandardIO(TTY_p)) {
        ttyr_core_unclaimStandardIO(TTY_p);
    }

    for (int i = 0; i < TTY_p->Prototypes.size; ++i) {
        if (((ttyr_core_Interface*)TTY_p->Prototypes.pp[i])->Callbacks.destroyPrototype_f) {
            ((ttyr_core_Interface*)TTY_p->Prototypes.pp[i])->Callbacks.destroyPrototype_f(TTY_p->Prototypes.pp[i]);
        }
    }
    nh_core_freeList(&TTY_p->Prototypes, false);

    for (int i = 0; i < TTY_p->Views.size; ++i) {
        ttyr_core_destroyView(TTY_p, TTY_p->Views.pp[i]);
    }
    nh_core_freeList(&TTY_p->Views, false);

    nh_core_free(TTY_p);
}

// RUN LOOP ========================================================================================
// The next functions comprise the top-level of the nhtty run loop.

static TTYR_CORE_RESULT ttyr_core_handleInput(
    ttyr_core_TTY *TTY_p)
{
    if (ttyr_core_claimsStandardIO(TTY_p)) {
        TTYR_CHECK(ttyr_core_readStandardInput(TTY_p))
    }

    while (1)
    {
        nh_api_WSIEvent *Event_p =
            nh_core_incrementRingBufferMarker(&TTY_p->Events, &TTY_p->Events.Marker);

        if (Event_p == NULL) {break;}

        TTYR_CHECK(ttyr_core_handleMacroWindowInput(TTY_p->Window_p, *Event_p))

        for (int i = 0; i < TTY_p->Views.size; ++i) {
            TTYR_CHECK(ttyr_core_forwardEvent(TTY_p->Views.pp[i], *Event_p))
        }
    }

    return TTYR_CORE_SUCCESS;
}

static TTYR_CORE_RESULT ttyr_core_handleWindowResize(
    ttyr_core_TTY *TTY_p, ttyr_core_View *View_p)
{
    bool updated = false;

    TTYR_CHECK(ttyr_core_getViewSize(View_p))
    TTYR_CHECK(ttyr_core_updateView(View_p, &updated, true))

    if (updated) {
        if (TTY_p->Window_p->MouseMenu_p) {
            TTY_p->Window_p->MouseMenu_p = NULL;
        }

	TTY_p->Window_p->refreshGrid1 = true;
	TTY_p->Window_p->refreshGrid2 = true;
	TTY_p->Window_p->refreshTitlebar = true;
    }

    return TTYR_CORE_SUCCESS;
}

static NH_SIGNAL ttyr_core_runTTY(
    void *tty_p)
{
    ttyr_core_TTY *TTY_p = tty_p;
    bool idle = true;

    for (int i = 0; i < TTY_p->Views.size; ++i) {
        TTYR_CHECK_2(NH_SIGNAL_ERROR, ttyr_core_handleWindowResize(TTY_p, TTY_p->Views.pp[i]))
    }
    if (TTY_p->Views.size <= 0 || ((ttyr_core_View*)TTY_p->Views.pp[0])->cols <= 0) {return NH_SIGNAL_IDLE;}

    TTYR_CHECK_2(NH_SIGNAL_ERROR, ttyr_core_handleInput(TTY_p))
    TTYR_CHECK_2(NH_SIGNAL_ERROR, ttyr_core_updateMacroWindow(TTY_p->Window_p))
    ttyr_core_checkTitlebar(&TTY_p->Titlebar, &TTY_p->Window_p->refreshGrid1);

    if (TTY_p->Window_p->refreshGrid1) {
        TTYR_CHECK_2(NH_SIGNAL_ERROR, ttyr_core_drawTitlebar(TTY_p))
        TTYR_CHECK_2(NH_SIGNAL_ERROR, ttyr_core_refreshGrid1(TTY_p))
    }
    if (TTY_p->Window_p->refreshGrid2) {
        TTYR_CHECK_2(NH_SIGNAL_ERROR, ttyr_core_refreshGrid2(TTY_p))
    }

    if (TTY_p->Window_p->refreshCursor || TTY_p->Window_p->refreshGrid1) {
        TTYR_CHECK_2(NH_SIGNAL_ERROR, ttyr_core_refreshCursor(TTY_p))
        idle = false;
    }

    TTY_p->Window_p->refreshGrid1 = false;
    TTY_p->Window_p->refreshGrid2 = false;
    TTY_p->Window_p->refreshCursor = false;
 
    if (TTY_p->Window_p->close) {
        // First, close all dependent workloads.
        for (int i = 0; i < TTY_p->Views.size; ++i) {
            ttyr_core_View *View_p = TTY_p->Views.pp[i];
            nh_core_Workload *Workload_p = nh_core_getWorkload(View_p->p);
            if (Workload_p) {
                nh_core_deactivateWorkload(Workload_p);
            }
        }
        return NH_SIGNAL_FINISH;
    }

    return idle ? NH_SIGNAL_IDLE : NH_SIGNAL_OK;
}

// COMMANDS ========================================================================================
// The next functions are executed by ttyr_core_cmd_* functions.

typedef enum TTYR_CORE_COMMAND_E {
    TTYR_CORE_COMMAND_CLAIM_STANDARD_IO,
    TTYR_CORE_COMMAND_UNCLAIM_STANDARD_IO,
    TTYR_CORE_COMMAND_SEND_EVENT,
} TTYR_CORE_COMMAND_E;

typedef struct ttyr_core_AddProgramData {
    char name_p[255];
} ttyr_core_AddProgramData;

/**
 * Here, most commands that came in through the API are handled.
 */
static NH_SIGNAL ttyr_core_runTTYCommand(
    void *tty_p, nh_core_WorkloadCommand *Command_p)
{
    ttyr_core_TTY *TTY_p = tty_p;

    switch (Command_p->type)
    {
        case TTYR_CORE_COMMAND_CLAIM_STANDARD_IO :
            TTYR_CHECK(ttyr_core_claimStandardIO(TTY_p))
            break;
        case TTYR_CORE_COMMAND_UNCLAIM_STANDARD_IO :
            TTYR_CHECK(ttyr_core_unclaimStandardIO(TTY_p))
            break;
        case TTYR_CORE_COMMAND_SEND_EVENT :
            nh_api_WSIEvent *Event2_p = nh_core_advanceRingBuffer(&TTY_p->Events);
            TTYR_CHECK_NULL(Event2_p)
            *Event2_p = *((nh_api_WSIEvent*)Command_p->p);
            break;
    }

    return NH_SIGNAL_OK;
}

// API =============================================================================================
// The next functions are called by lib/netzhaut/nhtty.h functions.

ttyr_core_TTY *ttyr_core_openTTY(
    char *config_p, ttyr_core_Interface *Interface_p)
{
    ttyr_core_OpenTTY OpenTTY;
    OpenTTY.config_p = config_p;
    OpenTTY.Interface_p = Interface_p;

    ttyr_core_TTY *TTY_p = nh_core_activateWorkload(
        ttyr_core_initTTY, ttyr_core_runTTY, ttyr_core_freeTTY, ttyr_core_runTTYCommand, &OpenTTY, true
    );

    return TTY_p;
}

TTYR_CORE_RESULT ttyr_core_closeTTY(
    ttyr_core_TTY *TTY_p)
{
    nh_core_deactivateWorkload(nh_core_getWorkload(TTY_p));
    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT ttyr_core_cmd_claimStandardIO(
    ttyr_core_TTY *TTY_p)
{
    nh_core_executeWorkloadCommand(TTY_p, TTYR_CORE_COMMAND_CLAIM_STANDARD_IO, NULL, 0);
    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT ttyr_core_cmd_unclaimStandardIO(
    ttyr_core_TTY *TTY_p)
{
    nh_core_executeWorkloadCommand(TTY_p, TTYR_CORE_COMMAND_UNCLAIM_STANDARD_IO, NULL, 0);
    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT ttyr_core_cmd_sendEvent(
    ttyr_core_TTY *TTY_p, nh_api_WSIEvent Event)
{
    switch (Event.type) {
        case NH_API_WSI_EVENT_KEYBOARD :
        case NH_API_WSI_EVENT_MOUSE :
        case NH_API_WSI_EVENT_WINDOW :
            nh_core_executeWorkloadCommand(TTY_p, TTYR_CORE_COMMAND_SEND_EVENT, &Event, sizeof(nh_api_WSIEvent));
        default :
            // Ignore other events.
    }

    return TTYR_CORE_SUCCESS;
}
