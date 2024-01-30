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

#include "nhcore/System/Thread.h"
#include "nhcore/System/Memory.h"
#include "nhcore/System/Process.h"
#include "nhcore/System/Logger.h"
#include "nhcore/Util/RingBuffer.h"

#include "nhencoding/Base/UnicodeDataHelper.h"
#include "nhencoding/Encodings/UTF8.h"
#include "nhencoding/Encodings/UTF32.h"

#include <string.h>
#include <stdio.h>

// CLIPBOARD =======================================================================================

TTYR_TTY_RESULT ttyr_tty_resetClipboard()
{
    ttyr_tty_TTY *TTY_p = nh_core_getWorkloadArg();

    for (int i = 0; i < TTY_p->Clipboard.Lines.length; ++i) {
        nh_encoding_UTF32String *String_p = 
            &((nh_encoding_UTF32String*)TTY_p->Clipboard.Lines.p)[i];
        nh_encoding_freeUTF32(String_p);
    }

    nh_core_freeArray(&TTY_p->Clipboard.Lines);
    TTY_p->Clipboard.Lines = nh_core_initArray(sizeof(nh_encoding_UTF32String), 32);

    return TTYR_TTY_SUCCESS;
}

nh_encoding_UTF32String *ttyr_tty_newClipboardLine()
{
    ttyr_tty_TTY *TTY_p = nh_core_getWorkloadArg();

    nh_encoding_UTF32String *Line_p = nh_core_incrementArray(&TTY_p->Clipboard.Lines);

    TTYR_CHECK_NULL_2(NULL, Line_p)
    *Line_p = nh_encoding_initUTF32(32);

    return Line_p;
}

ttyr_tty_Clipboard *ttyr_tty_getClipboard()
{
    return &((ttyr_tty_TTY*)nh_core_getWorkloadArg())->Clipboard;
}

// INIT/FREE =======================================================================================
// The next functions comprise the in/exit points of nhtty.

typedef struct ttyr_tty_OpenTTY {
    char *config_p;
    ttyr_tty_Interface *Interface_p;
} ttyr_tty_OpenTTY;

static void *ttyr_tty_initTTY(
    nh_core_Workload *Workload_p)
{
    ttyr_tty_OpenTTY *Args_p = Workload_p->args_p;

    static NH_BYTE *path_p = "nhtty/TTY/TTY.c";
    static NH_BYTE *name_p = "TTY Workload";
    Workload_p->path_p = path_p;
    Workload_p->name_p = name_p;
    Workload_p->module = -1;

    ttyr_tty_TTY *TTY_p = nh_core_allocate(sizeof(ttyr_tty_TTY));
    TTYR_CHECK_MEM_2(NULL, TTY_p)

    memset(TTY_p, 0, sizeof(ttyr_tty_TTY));

    if (Workload_p->args_p) {
        strcpy(TTY_p->namespace_p, Workload_p->args_p);
    } else {
        sprintf(TTY_p->namespace_p, "%p", TTY_p);
    }

    TTY_p->Windows = nh_core_initList(10);
    TTY_p->Views = nh_core_initList(8);
    TTY_p->Prototypes = nh_core_initList(8);
    TTY_p->Clipboard.Lines = nh_core_initArray(sizeof(nh_encoding_UTF32String), 32);

    TTY_p->hasFocus = NH_TRUE;
    TTY_p->Borders.on = NH_TRUE;
    TTY_p->Topbars.on = NH_TRUE;

    TTY_p->Preview.blink = NH_TRUE;
    TTY_p->Preview.LastBlink = nh_core_getSystemTime();

    TTYR_CHECK_2(NULL, nh_core_initRingBuffer(
        &TTY_p->Events, 128, sizeof(nh_wsi_Event), NULL 
    ))

    TTYR_CHECK_NULL_2(NULL, ttyr_tty_insertAndFocusWindow(TTY_p, 0))

    if (Args_p->Interface_p == NULL) {
        Args_p->Interface_p = ttyr_tty_createShellInterface();
    }
    ttyr_tty_addProgram(TTY_p, Args_p->Interface_p, NH_FALSE);
 
    return TTY_p;
}

static void ttyr_tty_freeTTY(
    void *p)
{
    ttyr_tty_TTY *TTY_p = p;

    ttyr_tty_destroyWindows(TTY_p);
 
    nh_core_freeArray(&(TTY_p->Clipboard.Lines));
    nh_core_freeRingBuffer(&(TTY_p->Events));

    if (ttyr_tty_claimsStandardIO(TTY_p)) {
        ttyr_tty_unclaimStandardIO(TTY_p);
    }

    for (int i = 0; i < TTY_p->Prototypes.size; ++i) {
        ((ttyr_tty_Interface*)TTY_p->Prototypes.pp[i])
            ->Callbacks.destroyPrototype_f(TTY_p->Prototypes.pp[i]);
    }
    nh_core_freeList(&TTY_p->Prototypes, NH_FALSE);

    for (int i = 0; i < TTY_p->Views.size; ++i) {
        ttyr_tty_destroyView(TTY_p, TTY_p->Views.pp[i]);
    }
    nh_core_freeList(&TTY_p->Views, NH_FALSE);

    nh_core_free(TTY_p);
}

// RUN LOOP ========================================================================================
// The next functions comprise the top-level of the nhtty run loop.

static TTYR_TTY_RESULT ttyr_tty_handleInput(
    ttyr_tty_TTY *TTY_p)
{
    if (ttyr_tty_claimsStandardIO(TTY_p)) {
        TTYR_CHECK(ttyr_tty_readStandardInput(TTY_p))
    }

    while (1)
    {
        nh_wsi_Event *Event_p =
            nh_core_incrementRingBufferMarker(&TTY_p->Events, &TTY_p->Events.Marker);

        if (Event_p == NULL) {break;}

        TTYR_CHECK(ttyr_tty_handleMacroWindowInput(TTY_p->Window_p, *Event_p))

        for (int i = 0; i < TTY_p->Views.size; ++i) {
            TTYR_CHECK(ttyr_tty_forwardEvent(TTY_p->Views.pp[i], *Event_p))
        }
    }

    return TTYR_TTY_SUCCESS;
}

static TTYR_TTY_RESULT ttyr_tty_handleWindowResize(
    ttyr_tty_TTY *TTY_p, ttyr_tty_View *View_p)
{
    NH_BOOL updated = NH_FALSE;

    TTYR_CHECK(ttyr_tty_getViewSize(View_p))
    TTYR_CHECK(ttyr_tty_updateView(View_p, &updated, NH_TRUE))

    if (updated) {
        if (TTY_p->Window_p->MouseMenu_p) {
            TTY_p->Window_p->MouseMenu_p = NULL;
        }
	TTY_p->Window_p->refreshGrid1 = NH_TRUE;
	TTY_p->Window_p->refreshGrid2 = NH_TRUE;
	TTY_p->Window_p->refreshTitlebar = NH_TRUE;
    }

    return TTYR_TTY_SUCCESS;
}

static NH_SIGNAL ttyr_tty_runTTY(
    void *tty_p)
{
    ttyr_tty_TTY *TTY_p = tty_p;
    NH_BOOL idle = NH_TRUE;

    for (int i = 0; i < TTY_p->Views.size; ++i) {
        TTYR_CHECK_2(NH_SIGNAL_ERROR, ttyr_tty_handleWindowResize(TTY_p, TTY_p->Views.pp[i]))
    }
    if (TTY_p->Views.size <= 0 || ((ttyr_tty_View*)TTY_p->Views.pp[0])->cols <= 0) {return NH_SIGNAL_IDLE;}

    TTYR_CHECK_2(NH_SIGNAL_ERROR, ttyr_tty_handleInput(TTY_p))
    TTYR_CHECK_2(NH_SIGNAL_ERROR, ttyr_tty_updateMacroWindow(TTY_p->Window_p))
    ttyr_tty_checkTitlebar(&TTY_p->Titlebar, &TTY_p->Window_p->refreshGrid1);

    if (TTY_p->Window_p->refreshGrid1) {
        TTYR_CHECK_2(NH_SIGNAL_ERROR, ttyr_tty_drawTitlebar(TTY_p))
        TTYR_CHECK_2(NH_SIGNAL_ERROR, ttyr_tty_refreshGrid1(TTY_p))
    }
    if (TTY_p->Window_p->refreshGrid2) {
        TTYR_CHECK_2(NH_SIGNAL_ERROR, ttyr_tty_refreshGrid2(TTY_p))
    }

    if (TTY_p->Window_p->refreshCursor || TTY_p->Window_p->refreshGrid1) {
        TTYR_CHECK_2(NH_SIGNAL_ERROR, ttyr_tty_refreshCursor(TTY_p))
        idle = NH_FALSE;
    }

    TTY_p->Window_p->refreshGrid1 = NH_FALSE;
    TTY_p->Window_p->refreshGrid2 = NH_FALSE;
    TTY_p->Window_p->refreshCursor = NH_FALSE;
 
    if (TTY_p->Window_p->close) {
        // First, close all dependent workloads.
        for (int i = 0; i < TTY_p->Views.size; ++i) {
            ttyr_tty_View *View_p = TTY_p->Views.pp[i];
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
// The next functions are executed by ttyr_tty_cmd_* functions.

typedef enum TTYR_TTY_COMMAND_E {
    TTYR_TTY_COMMAND_CLAIM_STANDARD_IO,
    TTYR_TTY_COMMAND_UNCLAIM_STANDARD_IO,
    TTYR_TTY_COMMAND_SEND_EVENT,
} TTYR_TTY_COMMAND_E;

typedef struct ttyr_tty_AddProgramData {
    char name_p[255];
} ttyr_tty_AddProgramData;

/**
 * Here, most commands that came in through the API are handled.
 */
static NH_SIGNAL ttyr_tty_runTTYCommand(
    void *tty_p, nh_core_WorkloadCommand *Command_p)
{
    ttyr_tty_TTY *TTY_p = tty_p;

    switch (Command_p->type)
    {
        case TTYR_TTY_COMMAND_CLAIM_STANDARD_IO :
            TTYR_CHECK(ttyr_tty_claimStandardIO(TTY_p))
            break;
        case TTYR_TTY_COMMAND_UNCLAIM_STANDARD_IO :
            TTYR_CHECK(ttyr_tty_unclaimStandardIO(TTY_p))
            break;
        case TTYR_TTY_COMMAND_SEND_EVENT :
            nh_wsi_Event *Event2_p = nh_core_advanceRingBuffer(&TTY_p->Events);
            TTYR_CHECK_NULL(Event2_p)
            *Event2_p = *((nh_wsi_Event*)Command_p->p);
            break;
    }

    return NH_SIGNAL_OK;
}

// API =============================================================================================
// The next functions are called by lib/netzhaut/nhtty.h functions.

ttyr_tty_TTY *ttyr_tty_openTTY(
    NH_BYTE *config_p, ttyr_tty_Interface *Interface_p)
{
    ttyr_tty_OpenTTY OpenTTY;
    OpenTTY.config_p = config_p;
    OpenTTY.Interface_p = Interface_p;

    ttyr_tty_TTY *TTY_p = nh_core_activateWorkload(
        ttyr_tty_initTTY, ttyr_tty_runTTY, ttyr_tty_freeTTY, ttyr_tty_runTTYCommand, &OpenTTY, NH_TRUE
    );

    return TTY_p;
}

TTYR_TTY_RESULT ttyr_tty_closeTTY(
    ttyr_tty_TTY *TTY_p)
{
    nh_core_deactivateWorkload(nh_core_getWorkload(TTY_p));
    return TTYR_TTY_SUCCESS;
}

TTYR_TTY_RESULT ttyr_tty_cmd_claimStandardIO(
    ttyr_tty_TTY *TTY_p)
{
    nh_core_executeWorkloadCommand(TTY_p, TTYR_TTY_COMMAND_CLAIM_STANDARD_IO, NULL, 0);
    return TTYR_TTY_SUCCESS;
}

TTYR_TTY_RESULT ttyr_tty_cmd_unclaimStandardIO(
    ttyr_tty_TTY *TTY_p)
{
    nh_core_executeWorkloadCommand(TTY_p, TTYR_TTY_COMMAND_UNCLAIM_STANDARD_IO, NULL, 0);
    return TTYR_TTY_SUCCESS;
}

TTYR_TTY_RESULT ttyr_tty_cmd_sendEvent(
    ttyr_tty_TTY *TTY_p, nh_wsi_Event Event)
{
    switch (Event.type) {
        case NH_WSI_EVENT_KEYBOARD :
        case NH_WSI_EVENT_MOUSE :
        case NH_WSI_EVENT_WINDOW :
            nh_core_executeWorkloadCommand(TTY_p, TTYR_TTY_COMMAND_SEND_EVENT, &Event, sizeof(nh_wsi_Event));
        default :
            // Ignore other events.
    }

    return TTYR_TTY_SUCCESS;
}

