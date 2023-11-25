// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "TopBar.h"
#include "Program.h"
#include "TTY.h"

#include "../Common/Macros.h"

#include "../../../../external/Netzhaut/src/lib/nhcore/System/Memory.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/System/Thread.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/Util/Time.h"

#include "../../../../external/Netzhaut/src/lib/nhencoding/Encodings/UTF32.h"
#include "../../../../external/Netzhaut/src/lib/nhencoding/Encodings/UTF8.h"

#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

// INIT/FREE =======================================================================================

ttyr_tty_TopBar ttyr_tty_initTopBar()
{
TTYR_TTY_BEGIN()

    ttyr_tty_TopBar TopBar;
    memset(&TopBar, 0, sizeof(ttyr_tty_TopBar));

    TopBar.Suggestions.Time = nh_core_getSystemTime();
    TopBar.Suggestions.interval = 0.5;
    TopBar.Suggestions.programIndex = -1;
    TopBar.Suggestions.commandIndex = -1;

    TopBar.Message.Text = nh_encoding_initUTF32(32);
    TopBar.state = -1;
    TopBar.Command  = nh_encoding_initUTF32(32);
    TopBar.History  = nh_core_initArray(sizeof(nh_encoding_UTF32String), 255);

TTYR_TTY_END(TopBar)
}

TTYR_TTY_RESULT ttyr_tty_freeTopBar(
    ttyr_tty_TopBar *TopBar_p)
{
TTYR_TTY_BEGIN()

    for (int i = 0; i < TopBar_p->History.length; ++i) {
        nh_encoding_freeUTF32(((nh_encoding_UTF32String*)TopBar_p->History.p)+i);
    }
    nh_core_freeArray(&TopBar_p->History);
    nh_encoding_freeUTF32(&TopBar_p->Command);
    nh_encoding_freeUTF32(&TopBar_p->Message.Text);

    *TopBar_p = ttyr_tty_initTopBar(NULL);

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

TTYR_TTY_RESULT ttyr_tty_getTopBarCursor(
    ttyr_tty_TopBar *TopBar_p, int *x_p, int *y_p, NH_BOOL topTile)
{
TTYR_TTY_BEGIN()

    *x_p = TopBar_p->cursorX;
    *y_p = 1;

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

// COMMAND =========================================================================================

static TTYR_TTY_RESULT ttyr_tty_appendToCommand(
    ttyr_tty_TopBar *TopBar_p, NH_ENCODING_UTF32 *str_p, int length)
{
TTYR_TTY_BEGIN()

    nh_encoding_appendUTF32(&TopBar_p->Command, str_p, length);
    TopBar_p->cursorX += length;

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

static void ttyr_tty_clearCommand(
    ttyr_tty_TopBar *TopBar_p)
{
TTYR_TTY_BEGIN()

    nh_encoding_freeUTF32(&TopBar_p->Command);
    TopBar_p->Command = nh_encoding_initUTF32(32);
    TopBar_p->cursorX = 0;

TTYR_TTY_SILENT_END()
}

// CONSOLE STATE ===================================================================================

static TTYR_TTY_RESULT ttyr_tty_resetTopBar(
    ttyr_tty_TopBar *TopBar_p)
{
TTYR_TTY_BEGIN()

    TopBar_p->state = -1;
    TopBar_p->hasFocus = NH_FALSE;
    ttyr_tty_clearCommand(TopBar_p);

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

void ttyr_tty_toggleTopBar(
    ttyr_tty_TopBar *TopBar_p)
{
TTYR_TTY_BEGIN()

    if (!TopBar_p->hasFocus) {
        TopBar_p->hasFocus = NH_TRUE;
        TopBar_p->state = 0;
    }
    else if (TopBar_p->state == 0) {
        if (((ttyr_tty_TTY*)nh_core_getWorkloadArg())->Prototypes.size < 2) {
            ttyr_tty_resetTopBar(TopBar_p);
        } else {
            TopBar_p->state = 1;
        }
    }
    else if (TopBar_p->state == 1) {
        ttyr_tty_resetTopBar(TopBar_p);
    }

    ttyr_tty_clearCommand(TopBar_p);

TTYR_TTY_SILENT_END()
}

// INTERNAL COMMAND ================================================================================

static int ttyr_tty_matchPrograms(
    nh_List *Tabs_p, nh_encoding_UTF32String *Line_p)
{
TTYR_TTY_BEGIN()

    nh_List Names = nh_core_initList(16);

    for (int i = 0; i < Tabs_p->size; ++i) {
        nh_core_appendToList(&Names, &((ttyr_tty_MicroTab*)Tabs_p->pp[i])->Prototype_p->Name); 
    } 

    nh_encoding_UTF32String *String_p = nh_encoding_getBestMatch(&Names, Line_p);
    nh_core_freeList(&Names, NH_FALSE);

    for (int i = 0; i < Tabs_p->size; ++i) {
        if (String_p ==  &((ttyr_tty_MicroTab*)Tabs_p->pp[i])->Prototype_p->Name) {
            TTYR_TTY_END(i)
        }
    } 

TTYR_TTY_END(-1)
}

static TTYR_TTY_RESULT ttyr_tty_executeProgramSwitchCommand(
    ttyr_tty_TopBar *TopBar_p, ttyr_tty_MicroWindow *Window_p)
{
TTYR_TTY_BEGIN()

    int program = TopBar_p->Command.length > 0 ?
        ttyr_tty_matchPrograms(Window_p->Tabs_p, &TopBar_p->Command) : -1;

    if (program >= 0)
    {
        Window_p->current = program;
        TTYR_TTY_CHECK(ttyr_tty_resetTopBar(TopBar_p))
    }

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

// PROGRAM COMMAND ==================================================================================

static int ttyr_tty_matchProgramCommands(
    nh_encoding_UTF32String *CommandNames_p, int commands, nh_encoding_UTF32String *Line_p)
{
TTYR_TTY_BEGIN()

    nh_List Commands = nh_core_initList(commands);
    for (int i = 0; i < commands; ++i) {
        nh_core_appendToList(&Commands, CommandNames_p+i); 
    } 

    nh_encoding_UTF32String *Command_p = nh_encoding_getBestMatch(&Commands, Line_p);

    int index = -1;
    for (int i = 0; i < Commands.size; ++i) {
        if (Command_p == Commands.pp[i]) {index = i; break;}
    } 

    nh_core_freeList(&Commands, NH_FALSE);

TTYR_TTY_END(index)
}

static TTYR_TTY_RESULT ttyr_tty_getArguments(
    ttyr_tty_TopBar *TopBar_p, nh_List *Arguments_p)
{
TTYR_TTY_BEGIN()

    *Arguments_p = nh_core_initList(8);
    nh_encoding_UTF32String *Argument_p = NULL;

    int offset = 0;
    for (offset = 0; offset < TopBar_p->Command.length && TopBar_p->Command.p[offset] != ' '; ++offset);
    offset++;

    for (int i = offset; i < TopBar_p->Command.length; ++i) {
        if (Argument_p && TopBar_p->Command.p[i] == ' ') {
            nh_core_appendToList(Arguments_p, Argument_p);
            Argument_p = NULL;
        }
        if (!Argument_p && TopBar_p->Command.p[i] != ' ') {
            Argument_p = nh_core_allocate(sizeof(nh_encoding_UTF32String));
            TTYR_TTY_CHECK_MEM(Argument_p)
            *Argument_p = nh_encoding_initUTF32(64);
        }
        if (Argument_p) {
            nh_encoding_appendUTF32Codepoint(Argument_p, TopBar_p->Command.p[i]);
        }
    }

    if (Argument_p) {nh_core_appendToList(Arguments_p, Argument_p);}

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

static TTYR_TTY_RESULT ttyr_tty_executeProgramCommand(
    ttyr_tty_TopBar *TopBar_p, ttyr_tty_MicroWindow *Window_p)
{
TTYR_TTY_BEGIN()

    ttyr_tty_Program *Program_p = ttyr_tty_getCurrentProgram(Window_p);

    if (Program_p->Prototype_p->Callbacks.handleCommand_f != NULL) 
    {
        nh_List Arguments = nh_core_initList(8);
        TTYR_TTY_CHECK(ttyr_tty_getArguments(TopBar_p, &Arguments))

        Program_p->command = TopBar_p->Suggestions.commandIndex;
        TTYR_TTY_RESULT result = Program_p->Prototype_p->Callbacks.handleCommand_f(Program_p, &Arguments);

        switch (result)
        {
            case TTYR_TTY_ERROR_UNKNOWN_COMMAND :
                TTYR_TTY_CHECK(ttyr_tty_setDefaultMessage(TopBar_p, TTYR_TTY_MESSAGE_MISC_UNKNOWN_COMMAND))
                break;
             case TTYR_TTY_ERROR_INVALID_ARGUMENT :
                TTYR_TTY_CHECK(ttyr_tty_setDefaultMessage(TopBar_p, TTYR_TTY_MESSAGE_MISC_INVALID_ARGUMENT))
                break;
        }

        for (int i = 0; i < Arguments.size; ++i) {
            nh_encoding_freeUTF32(Arguments.pp[i]);
        }
        nh_core_freeList(&Arguments, NH_TRUE);
    }
    else {
        TTYR_TTY_CHECK(ttyr_tty_setDefaultMessage(TopBar_p, TTYR_TTY_MESSAGE_MISC_UNKNOWN_COMMAND))
    }

    TTYR_TTY_CHECK(ttyr_tty_resetTopBar(TopBar_p))

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

// INPUT ===========================================================================================

TTYR_TTY_RESULT ttyr_tty_handleTopBarMouseInput(
    ttyr_tty_Tile *Tile_p, nh_wsi_MouseEvent Event)
{
TTYR_TTY_BEGIN()

    ttyr_tty_Config Config = ttyr_tty_getConfig();

    if (Event.trigger == NH_WSI_TRIGGER_PRESS && Event.Position.x-1 < Config.tabs && Event.Position.x > 0) {
        ((ttyr_tty_MacroTile*)Tile_p->p)->current = Event.Position.x-1;
    }
    if (Event.trigger == NH_WSI_TRIGGER_PRESS && Event.Position.x == TTYR_TTY_MACRO_TAB(Tile_p)->TopBar.quitPosition) {
        TTYR_TTY_MICRO_TAB(TTYR_TTY_MACRO_TAB(Tile_p))->Tile_p->close = NH_TRUE;
    }
    if (Event.trigger == NH_WSI_TRIGGER_MOVE && Event.Position.x == TTYR_TTY_MACRO_TAB(Tile_p)->TopBar.quitPosition) {
        TTYR_TTY_MACRO_TAB(Tile_p)->TopBar.quitHover = NH_TRUE;
    }
    if (Event.trigger == NH_WSI_TRIGGER_MOVE && Event.Position.x != TTYR_TTY_MACRO_TAB(Tile_p)->TopBar.quitPosition) {
        TTYR_TTY_MACRO_TAB(Tile_p)->TopBar.quitHover = NH_FALSE;
    }

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

TTYR_TTY_RESULT ttyr_tty_handleTopBarKeyboardInput(
    ttyr_tty_TopBar *TopBar_p, ttyr_tty_MicroWindow *Window_p, nh_wsi_KeyboardEvent Event)
{
TTYR_TTY_BEGIN()

    if (Event.trigger != NH_WSI_TRIGGER_PRESS) {TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)}

    NH_ENCODING_UTF32 c = Event.codepoint;

    switch (c)
    {
        case '\n' :
        case '\r' :
            if (TopBar_p->state == 0) {
                TTYR_TTY_DIAGNOSTIC_END(ttyr_tty_executeProgramCommand(TopBar_p, Window_p))
            }
            else {
                TTYR_TTY_CHECK(ttyr_tty_executeProgramSwitchCommand(TopBar_p, Window_p))
            }

        case '\x1b':
            TTYR_TTY_DIAGNOSTIC_END(ttyr_tty_resetTopBar(TopBar_p))

        case 127:
        case 8:
            if (TopBar_p->Command.length > 0) {
                nh_encoding_removeUTF32Tail(&TopBar_p->Command, 1);
                TopBar_p->cursorX -= 1;
            }
            TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS) 
    }
    
    TTYR_TTY_CHECK(ttyr_tty_appendToCommand(TopBar_p, &c, 1))

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

TTYR_TTY_RESULT ttyr_tty_handleTopBarInput(
    ttyr_tty_Tile *Tile_p, nh_wsi_Event Event)
{
TTYR_TTY_BEGIN()

    ttyr_tty_MacroTile *MacroTile_p = Tile_p->p;
    ttyr_tty_MacroTab *MacroTab_p = MacroTile_p->MacroTabs.pp[MacroTile_p->current];

    switch (Event.type) {
        case NH_WSI_EVENT_KEYBOARD :
            ttyr_tty_handleTopBarKeyboardInput(&MacroTab_p->TopBar, &MacroTab_p->MicroWindow, Event.Keyboard);
            break;
        case NH_WSI_EVENT_MOUSE :
            ttyr_tty_handleTopBarMouseInput(Tile_p, Event.Mouse);
            break;
    }

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

// DRAW ============================================================================================

static ttyr_tty_Glyph ttyr_tty_getGlyphHelper(
    NH_ENCODING_UTF32 codepoint)
{
TTYR_TTY_BEGIN()

    ttyr_tty_Glyph Glyph;
    memset(&Glyph, 0, sizeof(ttyr_tty_Glyph));
    Glyph.Attributes.reverse = NH_TRUE;
    Glyph.codepoint = codepoint;
    Glyph.mark |= TTYR_TTY_MARK_LINE_HORIZONTAL | TTYR_TTY_MARK_ACCENT;

TTYR_TTY_END(Glyph)
}

static int ttyr_tty_drawTopBarCommand(
    ttyr_tty_TopBar *TopBar_p, ttyr_tty_Glyph **Glyphs_pp, int *cols_p)
{
TTYR_TTY_BEGIN()

    ttyr_tty_Glyph Glyph = ttyr_tty_getGlyphHelper(' ');

    int length = TopBar_p->Command.length > *cols_p ? *cols_p : TopBar_p->Command.length;

    for (int i = 0; i < TopBar_p->Command.length && i < length; ++i) {
        Glyph.codepoint = TopBar_p->Command.p[i];
        (*Glyphs_pp)[i] = Glyph;
    }

    *cols_p = *cols_p - length;
    *Glyphs_pp = (*Glyphs_pp) + length;

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

static void ttyr_tty_setNextCommandSuggestionIndex(
    ttyr_tty_TopBar *TopBar_p, ttyr_tty_ProgramPrototype *Prototype_p)
{
TTYR_TTY_BEGIN() 

    if (TopBar_p->Suggestions.commandIndex == -1) {
        TopBar_p->Suggestions.commandIndex = 0;
        TTYR_TTY_SILENT_END()
    }
    if (TopBar_p->Suggestions.commandIndex >= Prototype_p->commands-1) {
        TopBar_p->Suggestions.commandIndex = 0;
        TTYR_TTY_SILENT_END()
    }
    TopBar_p->Suggestions.commandIndex++;

TTYR_TTY_SILENT_END()
}

static void ttyr_tty_setNextProgramSuggestionIndex(
    ttyr_tty_TopBar *TopBar_p, ttyr_tty_MicroWindow *Window_p)
{
TTYR_TTY_BEGIN() 

    if (TopBar_p->Suggestions.programIndex == -1) {
        TopBar_p->Suggestions.programIndex = 0;
	TTYR_TTY_SILENT_END()
    }
    if (TopBar_p->Suggestions.programIndex >= Window_p->Tabs_p->size-1) {
	TopBar_p->Suggestions.programIndex = 0;
	TTYR_TTY_SILENT_END()
    }
    TopBar_p->Suggestions.programIndex++;

TTYR_TTY_SILENT_END()
}

static TTYR_TTY_RESULT ttyr_tty_drawTopBarCommandSuggestion(
    ttyr_tty_TopBar *TopBar_p, ttyr_tty_MicroWindow *Window_p, ttyr_tty_Glyph *Glyphs_p, int cols)
{
TTYR_TTY_BEGIN()

    ttyr_tty_Program *Program_p = ttyr_tty_getCurrentProgram(Window_p);

    Glyphs_p[2] = ttyr_tty_getGlyphHelper(':');
    Glyphs_p[3] = ttyr_tty_getGlyphHelper(' ');
 
    if (TopBar_p->Suggestions.commandIndex == -1 || !Program_p || !Program_p->Prototype_p->commands) {
        Glyphs_p[4] = ttyr_tty_getGlyphHelper('n');
        Glyphs_p[5] = ttyr_tty_getGlyphHelper('o');
        Glyphs_p[6] = ttyr_tty_getGlyphHelper('n');
        Glyphs_p[7] = ttyr_tty_getGlyphHelper('e');
        TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
    }

    Glyphs_p += 4;
    cols -= 4;

    int length = Program_p->Prototype_p->CommandNames_p[TopBar_p->Suggestions.commandIndex].length;
    for (int i = 0; i < length; ++i) {
        Glyphs_p[i] = ttyr_tty_getGlyphHelper(
            Program_p->Prototype_p->CommandNames_p[TopBar_p->Suggestions.commandIndex].p[i]);
    }

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

static TTYR_TTY_RESULT ttyr_tty_drawTopBarProgramSuggestion(
    ttyr_tty_TopBar *TopBar_p, ttyr_tty_MicroWindow *Window_p, ttyr_tty_Glyph *Glyphs_p, int cols)
{
TTYR_TTY_BEGIN()

    Glyphs_p[2] = ttyr_tty_getGlyphHelper(':');
    Glyphs_p[3] = ttyr_tty_getGlyphHelper(' ');
 
    if (TopBar_p->Suggestions.programIndex == -1 || !Window_p->Tabs_p) {
        Glyphs_p[4] = ttyr_tty_getGlyphHelper('n');
        Glyphs_p[5] = ttyr_tty_getGlyphHelper('o');
        Glyphs_p[6] = ttyr_tty_getGlyphHelper('n');
        Glyphs_p[7] = ttyr_tty_getGlyphHelper('e');
        TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
    }

    ttyr_tty_ProgramPrototype *Prototype_p = 
        ((ttyr_tty_MicroTab*)Window_p->Tabs_p->pp[TopBar_p->Suggestions.programIndex])->Prototype_p;
    TTYR_TTY_CHECK_NULL(Prototype_p)

    Glyphs_p += 4;
    cols -= 4;

    for (int i = 0; i < Prototype_p->Name.length && i < cols; ++i) {
        Glyphs_p[i] = ttyr_tty_getGlyphHelper(Prototype_p->Name.p[i]);
    }

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

static TTYR_TTY_RESULT ttyr_tty_setSuggestionIndex(
    ttyr_tty_TopBar *TopBar_p, ttyr_tty_MicroWindow *Window_p, NH_BOOL command)
{
TTYR_TTY_BEGIN()

    ttyr_tty_ProgramPrototype *Prototype_p = ttyr_tty_getCurrentProgram(Window_p)->Prototype_p;
 
    // Check if current is perfect match.
    NH_BOOL match = TopBar_p->Command.length ? NH_TRUE : NH_FALSE;
    if (command) {
        if (TopBar_p->Suggestions.commandIndex == -1) {
            match = NH_FALSE;
        }
        else if (!Prototype_p->CommandNames_p) {
            match = NH_FALSE;
        }
        else if (TopBar_p->Command.length != 
            Prototype_p->CommandNames_p[TopBar_p->Suggestions.commandIndex].length) {
                match = NH_FALSE;
        }
        for (int i = 0; match && i < TopBar_p->Command.length; ++i) {
            if (TopBar_p->Command.p[i] != 
                Prototype_p->CommandNames_p[TopBar_p->Suggestions.commandIndex].p[i]) {
                match = NH_FALSE;
            }
        }
    } else {
        if (TopBar_p->Suggestions.programIndex == -1) {
            match = NH_FALSE;
        }
        else {
            Prototype_p = 
                ((ttyr_tty_MicroTab*)Window_p->Tabs_p->pp[TopBar_p->Suggestions.programIndex])->Prototype_p;
            TTYR_TTY_CHECK_NULL(Prototype_p)
        }
        if (match && TopBar_p->Command.length != Prototype_p->Name.length) {
            match = NH_FALSE;
        }
        for (int i = 0; match && i < TopBar_p->Command.length && i < Prototype_p->Name.length; ++i) {
            if (TopBar_p->Command.p[i] != Prototype_p->Name.p[i]) {
                match = NH_FALSE;
            }
        }
    }

    // If perfect match, we don't need a new index.
    if (match) {
        TTYR_TTY_END(TTYR_TTY_SUCCESS)
    }

    // Find semi match or none.
    NH_BOOL ok = NH_FALSE;
    int count = 0;
    int previousCommandSuggestionIndex = TopBar_p->Suggestions.commandIndex; 
    while (!ok) {
        if (command) {
            ttyr_tty_setNextCommandSuggestionIndex(TopBar_p, Prototype_p);
        } else {
            ttyr_tty_setNextProgramSuggestionIndex(TopBar_p, Window_p);
        }
        if (TopBar_p->Command.length == 0) {
            ok = NH_TRUE;
        }
        match = NH_TRUE;
        if (command) {
            if (!Prototype_p->CommandNames_p) {
                match = NH_FALSE;
            }
            else if (TopBar_p->Command.length >
                Prototype_p->CommandNames_p[TopBar_p->Suggestions.commandIndex].length) {
                    match = NH_FALSE;
            }
            for (int i = 0; match && i < TopBar_p->Command.length; ++i) {
                if (TopBar_p->Command.p[i] != 
                    Prototype_p->CommandNames_p[TopBar_p->Suggestions.commandIndex].p[i]) {
                    match = NH_FALSE;
                }
            }
            if (match) {ok = NH_TRUE;}
            if (++count > Prototype_p->commands) {
                TopBar_p->Suggestions.commandIndex = previousCommandSuggestionIndex;
                break;
            }
        } else {
            Prototype_p = 
                ((ttyr_tty_MicroTab*)Window_p->Tabs_p->pp[TopBar_p->Suggestions.programIndex])->Prototype_p;
            TTYR_TTY_CHECK_NULL(Prototype_p)
            for (int i = 0; i < TopBar_p->Command.length && i < Prototype_p->Name.length; ++i) {
                if (TopBar_p->Command.p[i] != Prototype_p->Name.p[i]) {
                    match = NH_FALSE;
                }
            }
            if (match) {ok = NH_TRUE;}
            if (++count > Window_p->Tabs_p->size) {
                TopBar_p->Suggestions.programIndex = -1;
                break;
            }
        }
    }
 
TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

static TTYR_TTY_RESULT ttyr_tty_drawTopBarSuggestion(
    ttyr_tty_TopBar *TopBar_p, ttyr_tty_MicroWindow *Window_p, ttyr_tty_Glyph *Glyphs_p, int cols)
{
TTYR_TTY_BEGIN()

    if (TopBar_p->Suggestions.update) {
        ttyr_tty_setSuggestionIndex(TopBar_p, Window_p, NH_TRUE);
        ttyr_tty_setSuggestionIndex(TopBar_p, Window_p, NH_FALSE);
        TopBar_p->Suggestions.update = NH_FALSE;
    }

    if (TopBar_p->state == 0) {
	TTYR_TTY_DIAGNOSTIC_END(ttyr_tty_drawTopBarCommandSuggestion(TopBar_p, Window_p, Glyphs_p, cols))
    }
    if (TopBar_p->state == 1) {
	TTYR_TTY_DIAGNOSTIC_END(ttyr_tty_drawTopBarProgramSuggestion(TopBar_p, Window_p, Glyphs_p, cols))
    }
    if (TopBar_p->state == 2) {
	TTYR_TTY_DIAGNOSTIC_END(ttyr_tty_drawTopBarProgramSuggestion(TopBar_p, Window_p, Glyphs_p, cols))
    }

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_ERROR_BAD_STATE)
}

static TTYR_TTY_RESULT ttyr_tty_drawTopBarMessage(
    ttyr_tty_TopBar *TopBar_p, ttyr_tty_MicroWindow *Window_p, ttyr_tty_Glyph *Glyphs_p, int cols)
{
TTYR_TTY_BEGIN()

    if (TopBar_p->Message.Text.length == 0) {
        ttyr_tty_Program *Program_p = ttyr_tty_getCurrentProgram(Window_p);
        if (!Program_p || !Program_p->Prototype_p) {TTYR_TTY_END(TTYR_TTY_SUCCESS)}
        if (Program_p->Prototype_p->Callbacks.drawTopbar_f) {
            TTYR_TTY_CHECK(Program_p->Prototype_p->Callbacks.drawTopbar_f(Program_p, Glyphs_p, cols))
        } else {
            ttyr_tty_ProgramPrototype *Prototype_p = Program_p->Prototype_p;
            for (int i = 0; i < cols && i < Program_p->Prototype_p->Name.length; ++i) {
                Glyphs_p[i] = ttyr_tty_getGlyphHelper(Program_p->Prototype_p->Name.p[i]);
            }
        }
    } else {
        for (int i = 0, j = 0; i < cols && j < TopBar_p->Message.Text.length; ++i, ++j) {
            Glyphs_p[i] = ttyr_tty_getGlyphHelper(TopBar_p->Message.Text.p[j]);
        }
    }

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

static TTYR_TTY_RESULT ttyr_tty_drawTopBarText(
     ttyr_tty_Tile *Tile_p, ttyr_tty_Glyph *Glyphs_p, int cols)
{
TTYR_TTY_BEGIN()

    ttyr_tty_Config Config = ttyr_tty_getConfig();
    ttyr_tty_TopBar *TopBar_p = &TTYR_TTY_MACRO_TAB(Tile_p)->TopBar;
    ttyr_tty_MicroWindow *MicroWindow_p = &TTYR_TTY_MACRO_TAB(Tile_p)->MicroWindow;

    if (!TopBar_p->hasFocus) 
    {
        ttyr_tty_TTY *TTY_p = nh_core_getWorkloadArg();

        NH_ENCODING_UTF32 t1_p[15] = {' ', '(', 'M', 'I', 'C', 'R', 'O', ' ', 'T', 'I', 'L', 'I', 'N', 'G', ')'};
        NH_ENCODING_UTF32 t2_p[15] = {' ', '(', 'M', 'A', 'C', 'R', 'O', ' ', 'T', 'I', 'L', 'I', 'N', 'G', ')'};

        ttyr_tty_MacroWindow *MacroWindow_p = TTY_p->Window_p;
        if (MacroWindow_p->Tiling.stage == TTYR_TTY_TILING_STAGE_DONE) {
            ttyr_tty_clearMessage(TopBar_p);
        }

        TTYR_TTY_CHECK(ttyr_tty_drawTopBarMessage(TopBar_p, MicroWindow_p, Glyphs_p, cols))

        ttyr_tty_TopBar *TopBarCompare_p = &TTYR_TTY_MACRO_TAB(MacroWindow_p->Tile_p)->TopBar;

        if (MacroWindow_p->Tiling.stage != TTYR_TTY_TILING_STAGE_DONE && TopBarCompare_p == TopBar_p) {
            if (MacroWindow_p->Tiling.mode == TTYR_TTY_TILING_MODE_MICRO) {
                for (int i = cols-15, j = 0; j < 15; ++i, ++j) {
                    Glyphs_p[i].codepoint = t1_p[j];
                }
            } else if (MacroWindow_p->Tiling.mode == TTYR_TTY_TILING_MODE_MACRO) {
                for (int i = cols-15, j = 0; j < 15; ++i, ++j) {
                    Glyphs_p[i].codepoint = t2_p[j];
                }
            }
        } else {
            for (int i = 0; i < Config.tabs; ++i) {
                Glyphs_p[i+1].codepoint = 0x25a1;
            }
            TopBar_p->quitPosition = cols-2;
            Glyphs_p[TTYR_TTY_MACRO_TILE(Tile_p)->current+1].codepoint = 0x25a0;
            Glyphs_p[TopBar_p->quitPosition].codepoint = TopBar_p->quitHover ? 0x25cf : 0x25cb;
        }

        TTYR_TTY_END(TTYR_TTY_SUCCESS)
    }

    if (TopBar_p->Command.length > 0) {
        TTYR_TTY_CHECK(ttyr_tty_drawTopBarCommand(TopBar_p, &Glyphs_p, &cols))
    }

    TTYR_TTY_CHECK(ttyr_tty_drawTopBarSuggestion(TopBar_p, MicroWindow_p, Glyphs_p, cols))

    NH_ENCODING_UTF32 cmd_p[14]  = {' ', '(', 'E', 'X', 'E', 'C', 'U', 'T', 'E', ' ', 'C', 'M', 'D', ')'};
    NH_ENCODING_UTF32 prog_p[14] = {' ', '(', 'S', 'W', 'I', 'T', 'C', 'H', ' ', 'P', 'R', 'O', 'G', ')'};

    if (TopBar_p->state == 0 && cols > 14) {
        for (int i = cols-15, j = 0; j < 14; ++i, ++j) {
            Glyphs_p[i].codepoint = cmd_p[j];
        }
    }
    if (TopBar_p->state == 1 && cols > 14) {
        for (int i = cols-15, j = 0; j < 14; ++i, ++j) {
            Glyphs_p[i].codepoint = prog_p[j];
        }
    }

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

TTYR_TTY_RESULT ttyr_tty_drawTopBarRow(
    ttyr_tty_Tile *Tile_p, ttyr_tty_Glyph *Glyphs_p, int cols, int row, NH_BOOL standardIO)
{
TTYR_TTY_BEGIN()

    ttyr_tty_TTY *TTY_p = nh_core_getWorkloadArg();

    for (int i = 0; i < cols; ++i) {
        Glyphs_p[i] = ttyr_tty_getGlyphHelper(' ');
        if (!Tile_p && !standardIO) {
            Glyphs_p[i].codepoint = 'q';
            Glyphs_p[i].Attributes.reverse = NH_FALSE;
            Glyphs_p[i].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
        }
        Glyphs_p[i].mark |= TTYR_TTY_MARK_LINE_HORIZONTAL;
    }

    if (Tile_p) {
        TTYR_TTY_CHECK(ttyr_tty_drawTopBarText(Tile_p, Glyphs_p, cols))
    }

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

// UPDATE ==========================================================================================

TTYR_TTY_RESULT ttyr_tty_updateTopBar(
    ttyr_tty_TopBar *TopBar_p)
{
TTYR_TTY_BEGIN()

    nh_SystemTime Now = nh_core_getSystemTime();

    if (nh_core_getSystemTimeDiffInSeconds(TopBar_p->Suggestions.Time, Now) 
	>= TopBar_p->Suggestions.interval) 
    {
        TopBar_p->Suggestions.Time = Now;
        TopBar_p->Suggestions.update = NH_TRUE;
        TopBar_p->refresh = NH_TRUE;
    }

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

