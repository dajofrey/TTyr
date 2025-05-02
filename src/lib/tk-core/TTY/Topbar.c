// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Topbar.h"
#include "Program.h"
#include "TTY.h"

#include "../Common/Macros.h"

#include "nh-core/System/Memory.h"
#include "nh-core/System/Thread.h"
#include "nh-core/Util/Time.h"

#include "nh-encoding/Encodings/UTF32.h"
#include "nh-encoding/Encodings/UTF8.h"

#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

// INIT/FREE =======================================================================================

tk_core_Topbar tk_core_initTopbar()
{
    tk_core_Topbar Topbar;
    memset(&Topbar, 0, sizeof(tk_core_Topbar));

    Topbar.Suggestions.Time = nh_core_getSystemTime();
    Topbar.Suggestions.interval = 0.5;
    Topbar.Suggestions.programIndex = -1;
    Topbar.Suggestions.commandIndex = -1;

    Topbar.Message.Text = nh_encoding_initUTF32(32);
    Topbar.state = -1;
    Topbar.Command  = nh_encoding_initUTF32(32);
    Topbar.History  = nh_core_initArray(sizeof(nh_encoding_UTF32String), 255);

    return Topbar;
}

TK_CORE_RESULT tk_core_freeTopbar(
    tk_core_Topbar *Topbar_p)
{
    for (int i = 0; i < Topbar_p->History.length; ++i) {
        nh_encoding_freeUTF32(((nh_encoding_UTF32String*)Topbar_p->History.p)+i);
    }
    nh_core_freeArray(&Topbar_p->History);
    nh_encoding_freeUTF32(&Topbar_p->Command);
    nh_encoding_freeUTF32(&Topbar_p->Message.Text);

    *Topbar_p = tk_core_initTopbar(NULL);

    return TK_CORE_SUCCESS;
}

TK_CORE_RESULT tk_core_getTopbarCursor(
    tk_core_Topbar *Topbar_p, int *x_p, int *y_p, bool topTile)
{
    *x_p = Topbar_p->cursorX;
    *y_p = 1;
    return TK_CORE_SUCCESS;
}

// COMMAND =========================================================================================

static TK_CORE_RESULT tk_core_appendToCommand(
    tk_core_Topbar *Topbar_p, NH_API_UTF32 *str_p, int length)
{
    nh_encoding_appendUTF32(&Topbar_p->Command, str_p, length);
    Topbar_p->cursorX += length;
    return TK_CORE_SUCCESS;
}

static void tk_core_clearCommand(
    tk_core_Topbar *Topbar_p)
{
    nh_encoding_freeUTF32(&Topbar_p->Command);
    Topbar_p->Command = nh_encoding_initUTF32(32);
    Topbar_p->cursorX = 0;
}

// CONSOLE STATE ===================================================================================

static TK_CORE_RESULT tk_core_resetTopbar(
    tk_core_Topbar *Topbar_p)
{
    Topbar_p->state = -1;
    Topbar_p->hasFocus = false;
    tk_core_clearCommand(Topbar_p);
    return TK_CORE_SUCCESS;
}

void tk_core_toggleTopbar(
    tk_core_Topbar *Topbar_p)
{
    if (!Topbar_p->hasFocus) {
        Topbar_p->hasFocus = true;
        Topbar_p->state = 0;
    }
    else if (Topbar_p->state == 0) {
        if (((tk_core_TTY*)nh_core_getWorkloadArg())->Prototypes.size < 2) {
            tk_core_resetTopbar(Topbar_p);
        } else {
            Topbar_p->state = 1;
        }
    }
    else if (Topbar_p->state == 1) {
        tk_core_resetTopbar(Topbar_p);
    }

    tk_core_clearCommand(Topbar_p);
}

// INTERNAL COMMAND ================================================================================

//static int tk_core_matchPrograms(
//    nh_core_List *Tabs_p, nh_encoding_UTF32String *Line_p)
//{
//    nh_core_List Names = nh_core_initList(16);
//
//    for (int i = 0; i < Tabs_p->size; ++i) {
//        nh_core_appendToList(&Names, &((tk_core_MicroTab*)Tabs_p->pp[i])->Prototype_p->Name); 
//    } 
//
//    nh_encoding_UTF32String *String_p = nh_encoding_getBestMatch(&Names, Line_p);
//    nh_core_freeList(&Names, false);
//
//    for (int i = 0; i < Tabs_p->size; ++i) {
//        if (String_p ==  &((tk_core_MicroTab*)Tabs_p->pp[i])->Prototype_p->Name) {
//            return i;
//        }
//    } 
//
//    return -1;
//}
//
//static TK_CORE_RESULT tk_core_executeProgramSwitchCommand(
//    tk_core_Topbar *Topbar_p, tk_core_MicroWindow *Window_p)
//{
//    int program = Topbar_p->Command.length > 0 ?
//        tk_core_matchPrograms(Window_p->Tabs_p, &Topbar_p->Command) : -1;
//
//    if (program >= 0)
//    {
//        Window_p->current = program;
//        TK_CHECK(tk_core_resetTopbar(Topbar_p))
//    }
//
//    return TK_CORE_SUCCESS;
//}

// PROGRAM COMMAND ==================================================================================

static int tk_core_matchProgramCommands(
    nh_encoding_UTF32String *CommandNames_p, int commands, nh_encoding_UTF32String *Line_p)
{
    nh_core_List Commands = nh_core_initList(commands);
    for (int i = 0; i < commands; ++i) {
        nh_core_appendToList(&Commands, CommandNames_p+i); 
    } 

    nh_encoding_UTF32String *Command_p = nh_encoding_getBestMatch(&Commands, Line_p);

    int index = -1;
    for (int i = 0; i < Commands.size; ++i) {
        if (Command_p == Commands.pp[i]) {index = i; break;}
    } 

    nh_core_freeList(&Commands, false);

    return index;
}

static TK_CORE_RESULT tk_core_getArguments(
    tk_core_Topbar *Topbar_p, nh_core_List *Arguments_p)
{
    *Arguments_p = nh_core_initList(8);
    nh_encoding_UTF32String *Argument_p = NULL;

    int offset = 0;
    for (offset = 0; offset < Topbar_p->Command.length && Topbar_p->Command.p[offset] != ' '; ++offset);
    offset++;

    for (int i = offset; i < Topbar_p->Command.length; ++i) {
        if (Argument_p && Topbar_p->Command.p[i] == ' ') {
            nh_core_appendToList(Arguments_p, Argument_p);
            Argument_p = NULL;
        }
        if (!Argument_p && Topbar_p->Command.p[i] != ' ') {
            Argument_p = (nh_encoding_UTF32String*)nh_core_allocate(sizeof(nh_encoding_UTF32String));
            TK_CHECK_MEM(Argument_p)
            *Argument_p = nh_encoding_initUTF32(64);
        }
        if (Argument_p) {
            nh_encoding_appendUTF32Codepoint(Argument_p, Topbar_p->Command.p[i]);
        }
    }

    if (Argument_p) {nh_core_appendToList(Arguments_p, Argument_p);}

    return TK_CORE_SUCCESS;
}

static TK_CORE_RESULT tk_core_executeProgramCommand(
    tk_core_Topbar *Topbar_p, tk_core_MicroWindow *Window_p)
{
    tk_core_Program *Program_p = tk_core_getCurrentProgram(Window_p);

    if (Program_p->Prototype_p->Callbacks.handleCommand_f != NULL) 
    {
        nh_core_List Arguments = nh_core_initList(8);
        TK_CHECK(tk_core_getArguments(Topbar_p, &Arguments))

        Program_p->command = Topbar_p->Suggestions.commandIndex;
        TK_CORE_RESULT result = Program_p->Prototype_p->Callbacks.handleCommand_f(Program_p);

        switch (result)
        {
            case TK_CORE_ERROR_UNKNOWN_COMMAND :
                TK_CHECK(tk_core_setDefaultMessage(Topbar_p, TK_CORE_MESSAGE_MISC_UNKNOWN_COMMAND))
                break;
             case TK_CORE_ERROR_INVALID_ARGUMENT :
                TK_CHECK(tk_core_setDefaultMessage(Topbar_p, TK_CORE_MESSAGE_MISC_INVALID_ARGUMENT))
                break;
        }

        for (int i = 0; i < Arguments.size; ++i) {
            nh_encoding_freeUTF32(Arguments.pp[i]);
        }
        nh_core_freeList(&Arguments, true);
    }
    else {
        TK_CHECK(tk_core_setDefaultMessage(Topbar_p, TK_CORE_MESSAGE_MISC_UNKNOWN_COMMAND))
    }

    TK_CHECK(tk_core_resetTopbar(Topbar_p))

    return TK_CORE_SUCCESS;
}

// INPUT ===========================================================================================

TK_CORE_RESULT tk_core_handleTopbarMouseInput(
    tk_core_Tile *Tile_p, nh_api_MouseEvent Event)
{
    if (Event.trigger == NH_API_TRIGGER_PRESS && Event.Position.x-1 < 9 && Event.Position.x > 0) {
        ((tk_core_MacroTile*)Tile_p->p)->current = Event.Position.x-1;
    }
//    if (Event.trigger == NH_API_TRIGGER_PRESS && Event.Position.x == TK_CORE_MACRO_TAB(Tile_p)->Topbar.quitPosition) {
//        TK_CORE_MICRO_TAB(TK_CORE_MACRO_TAB(Tile_p))->Tile_p->close = true;
//    }
//    if (Event.trigger == NH_API_TRIGGER_MOVE && Event.Position.x == TK_CORE_MACRO_TAB(Tile_p)->Topbar.quitPosition) {
//        TK_CORE_MACRO_TAB(Tile_p)->Topbar.quitHover = true;
//    }
//    if (Event.trigger == NH_API_TRIGGER_MOVE && Event.Position.x != TK_CORE_MACRO_TAB(Tile_p)->Topbar.quitPosition) {
//        TK_CORE_MACRO_TAB(Tile_p)->Topbar.quitHover = false;
//    }

    return TK_CORE_SUCCESS;
}

TK_CORE_RESULT tk_core_handleTopbarKeyboardInput(
    tk_core_Topbar *Topbar_p, tk_core_MicroWindow *Window_p, nh_api_KeyboardEvent Event)
{
    if (Event.trigger != NH_API_TRIGGER_PRESS) {return TK_CORE_SUCCESS;}

    NH_API_UTF32 c = Event.codepoint;

    switch (c)
    {
        case '\n' :
        case '\r' :
            if (Topbar_p->state == 0) {
                return tk_core_executeProgramCommand(Topbar_p, Window_p);
            }
            else {
//                TK_CHECK(tk_core_executeProgramSwitchCommand(Topbar_p, Window_p))
            }

        case '\x1b':
            return tk_core_resetTopbar(Topbar_p);

        case 127:
        case 8:
            if (Topbar_p->Command.length > 0) {
                nh_encoding_removeUTF32Tail(&Topbar_p->Command, 1);
                Topbar_p->cursorX -= 1;
            }
            return TK_CORE_SUCCESS;
    }
    
    TK_CHECK(tk_core_appendToCommand(Topbar_p, &c, 1))

    return TK_CORE_SUCCESS;
}

TK_CORE_RESULT tk_core_handleTopbarInput(
    tk_core_Tile *Tile_p, nh_api_WSIEvent Event)
{
    tk_core_MacroTile *MacroTile_p = Tile_p->p;
    tk_core_MacroTab *MacroTab_p = MacroTile_p->MacroTabs.pp[MacroTile_p->current];

    switch (Event.type) {
        case NH_API_WSI_EVENT_KEYBOARD :
            tk_core_handleTopbarKeyboardInput(&MacroTab_p->Topbar, &MacroTab_p->MicroWindow, Event.Keyboard);
            break;
        case NH_API_WSI_EVENT_MOUSE :
            tk_core_handleTopbarMouseInput(Tile_p, Event.Mouse);
            break;
    }

    return TK_CORE_SUCCESS;
}

// DRAW ============================================================================================

static tk_core_Glyph tk_core_getGlyphHelper(
    NH_API_UTF32 codepoint)
{
    tk_core_Glyph Glyph;
    memset(&Glyph, 0, sizeof(tk_core_Glyph));
    Glyph.Attributes.reverse = true;
    Glyph.codepoint = codepoint;
    Glyph.mark |= TK_CORE_MARK_LINE_HORIZONTAL;
    Glyph.mark |= TK_CORE_MARK_ACCENT;
    return Glyph;
}

static int tk_core_drawTopbarCommand(
    tk_core_Topbar *Topbar_p, tk_core_Glyph **Glyphs_pp, int *cols_p)
{
    tk_core_Glyph Glyph = tk_core_getGlyphHelper(' ');

    int length = Topbar_p->Command.length > *cols_p ? *cols_p : Topbar_p->Command.length;

    for (int i = 0; i < Topbar_p->Command.length && i < length; ++i) {
        Glyph.codepoint = Topbar_p->Command.p[i];
        (*Glyphs_pp)[i] = Glyph;
    }

    *cols_p = *cols_p - length;
    *Glyphs_pp = (*Glyphs_pp) + length;

    return TK_CORE_SUCCESS;
}

//static void tk_core_setNextCommandSuggestionIndex(
//    tk_core_Topbar *Topbar_p, tk_core_Interface *Prototype_p)
//{
//    if (Topbar_p->Suggestions.commandIndex == -1) {
//        Topbar_p->Suggestions.commandIndex = 0;
//        return;
//    }
//    if (Topbar_p->Suggestions.commandIndex >= Prototype_p->commands-1) {
//        Topbar_p->Suggestions.commandIndex = 0;
//        return;
//    }
//    Topbar_p->Suggestions.commandIndex++;
//}
//
//static void tk_core_setNextProgramSuggestionIndex(
//    tk_core_Topbar *Topbar_p, tk_core_MicroWindow *Window_p)
//{
//    if (Topbar_p->Suggestions.programIndex == -1) {
//        Topbar_p->Suggestions.programIndex = 0;
//        return;
//    }
//    if (Topbar_p->Suggestions.programIndex >= Window_p->Tabs_p->size-1) {
//	Topbar_p->Suggestions.programIndex = 0;
//        return;
//    }
//    Topbar_p->Suggestions.programIndex++;
//}
//
//static TK_CORE_RESULT tk_core_drawTopbarCommandSuggestion(
//    tk_core_Topbar *Topbar_p, tk_core_MicroWindow *Window_p, tk_core_Glyph *Glyphs_p, int cols)
//{
//    tk_core_Program *Program_p = tk_core_getCurrentProgram(Window_p);
//
//    Glyphs_p[2] = tk_core_getGlyphHelper(':');
//    Glyphs_p[3] = tk_core_getGlyphHelper(' ');
// 
//    if (Topbar_p->Suggestions.commandIndex == -1 || !Program_p || !Program_p->Prototype_p->commands) {
//        Glyphs_p[4] = tk_core_getGlyphHelper('n');
//        Glyphs_p[5] = tk_core_getGlyphHelper('o');
//        Glyphs_p[6] = tk_core_getGlyphHelper('n');
//        Glyphs_p[7] = tk_core_getGlyphHelper('e');
//        return TK_CORE_SUCCESS;
//    }
//
//    Glyphs_p += 4;
//    cols -= 4;
//
//    int length = Program_p->Prototype_p->CommandNames_p[Topbar_p->Suggestions.commandIndex].length;
//    for (int i = 0; i < length; ++i) {
//        Glyphs_p[i] = tk_core_getGlyphHelper(
//            Program_p->Prototype_p->CommandNames_p[Topbar_p->Suggestions.commandIndex].p[i]);
//    }
//
//    return TK_CORE_SUCCESS;
//}
//
//static TK_CORE_RESULT tk_core_drawTopbarProgramSuggestion(
//    tk_core_Topbar *Topbar_p, tk_core_MicroWindow *Window_p, tk_core_Glyph *Glyphs_p, int cols)
//{
//    Glyphs_p[2] = tk_core_getGlyphHelper(':');
//    Glyphs_p[3] = tk_core_getGlyphHelper(' ');
// 
//    if (Topbar_p->Suggestions.programIndex == -1 || !Window_p->Tabs_p) {
//        Glyphs_p[4] = tk_core_getGlyphHelper('n');
//        Glyphs_p[5] = tk_core_getGlyphHelper('o');
//        Glyphs_p[6] = tk_core_getGlyphHelper('n');
//        Glyphs_p[7] = tk_core_getGlyphHelper('e');
//        return TK_CORE_SUCCESS;
//    }
//
//    tk_core_Interface *Prototype_p = 
//        ((tk_core_MicroTab*)Window_p->Tabs_p->pp[Topbar_p->Suggestions.programIndex])->Prototype_p;
//    TK_CHECK_NULL(Prototype_p)
//
//    Glyphs_p += 4;
//    cols -= 4;
//
//    for (int i = 0; i < Prototype_p->Name.length && i < cols; ++i) {
//        Glyphs_p[i] = tk_core_getGlyphHelper(Prototype_p->Name.p[i]);
//    }
//
//    return TK_CORE_SUCCESS;
//}
//
//static TK_CORE_RESULT tk_core_setSuggestionIndex(
//    tk_core_Topbar *Topbar_p, tk_core_MicroWindow *Window_p, bool command)
//{
//    tk_core_Interface *Prototype_p = tk_core_getCurrentProgram(Window_p)->Prototype_p;
// 
//    // Check if current is perfect match.
//    bool match = Topbar_p->Command.length ? true : false;
//    if (command) {
//        if (Topbar_p->Suggestions.commandIndex == -1) {
//            match = false;
//        }
//        else if (!Prototype_p->CommandNames_p) {
//            match = false;
//        }
//        else if (Topbar_p->Command.length != 
//            Prototype_p->CommandNames_p[Topbar_p->Suggestions.commandIndex].length) {
//                match = false;
//        }
//        for (int i = 0; match && i < Topbar_p->Command.length; ++i) {
//            if (Topbar_p->Command.p[i] != 
//                Prototype_p->CommandNames_p[Topbar_p->Suggestions.commandIndex].p[i]) {
//                match = false;
//            }
//        }
//    } else {
//        if (Topbar_p->Suggestions.programIndex == -1) {
//            match = false;
//        }
//        else {
//            Prototype_p = 
//                ((tk_core_MicroTab*)Window_p->Tabs_p->pp[Topbar_p->Suggestions.programIndex])->Prototype_p;
//            TK_CHECK_NULL(Prototype_p)
//        }
//        if (match && Topbar_p->Command.length != Prototype_p->Name.length) {
//            match = false;
//        }
//        for (int i = 0; match && i < Topbar_p->Command.length && i < Prototype_p->Name.length; ++i) {
//            if (Topbar_p->Command.p[i] != Prototype_p->Name.p[i]) {
//                match = false;
//            }
//        }
//    }
//
//    // If perfect match, we don't need a new index.
//    if (match) {
//        return TK_CORE_SUCCESS;
//    }
//
//    // Find semi match or none.
//    bool ok = false;
//    int count = 0;
//    int previousCommandSuggestionIndex = Topbar_p->Suggestions.commandIndex; 
//    while (!ok) {
//        if (command) {
//            tk_core_setNextCommandSuggestionIndex(Topbar_p, Prototype_p);
//        } else {
//            tk_core_setNextProgramSuggestionIndex(Topbar_p, Window_p);
//        }
//        if (Topbar_p->Command.length == 0) {
//            ok = true;
//        }
//        match = true;
//        if (command) {
//            if (!Prototype_p->CommandNames_p) {
//                match = false;
//            }
//            else if (Topbar_p->Command.length >
//                Prototype_p->CommandNames_p[Topbar_p->Suggestions.commandIndex].length) {
//                    match = false;
//            }
//            for (int i = 0; match && i < Topbar_p->Command.length; ++i) {
//                if (Topbar_p->Command.p[i] != 
//                    Prototype_p->CommandNames_p[Topbar_p->Suggestions.commandIndex].p[i]) {
//                    match = false;
//                }
//            }
//            if (match) {ok = true;}
//            if (++count > Prototype_p->commands) {
//                Topbar_p->Suggestions.commandIndex = previousCommandSuggestionIndex;
//                break;
//            }
//        } else {
//            Prototype_p = 
//                ((tk_core_MicroTab*)Window_p->Tabs_p->pp[Topbar_p->Suggestions.programIndex])->Prototype_p;
//            TK_CHECK_NULL(Prototype_p)
//            for (int i = 0; i < Topbar_p->Command.length && i < Prototype_p->Name.length; ++i) {
//                if (Topbar_p->Command.p[i] != Prototype_p->Name.p[i]) {
//                    match = false;
//                }
//            }
//            if (match) {ok = true;}
//            if (++count > Window_p->Tabs_p->size) {
//                Topbar_p->Suggestions.programIndex = -1;
//                break;
//            }
//        }
//    }
//
//    return TK_CORE_SUCCESS;
//}
//
//static TK_CORE_RESULT tk_core_drawTopbarSuggestion(
//    tk_core_Topbar *Topbar_p, tk_core_MicroWindow *Window_p, tk_core_Glyph *Glyphs_p, int cols)
//{
//    if (Topbar_p->Suggestions.update) {
//        tk_core_setSuggestionIndex(Topbar_p, Window_p, true);
//        tk_core_setSuggestionIndex(Topbar_p, Window_p, false);
//        Topbar_p->Suggestions.update = false;
//    }
//
//    if (Topbar_p->state == 0) {
//	return tk_core_drawTopbarCommandSuggestion(Topbar_p, Window_p, Glyphs_p, cols);
//    }
//    if (Topbar_p->state == 1) {
//	return tk_core_drawTopbarProgramSuggestion(Topbar_p, Window_p, Glyphs_p, cols);
//    }
//    if (Topbar_p->state == 2) {
//	return tk_core_drawTopbarProgramSuggestion(Topbar_p, Window_p, Glyphs_p, cols);
//    }
//
//    return TK_CORE_ERROR_BAD_STATE;
//}

static TK_CORE_RESULT tk_core_drawTopbarMessage(
    tk_core_Config *Config_p, tk_core_Topbar *Topbar_p, tk_core_MicroWindow *Window_p, tk_core_Glyph *Glyphs_p, int cols)
{
    if (Config_p->Topbar.on && Topbar_p->Message.Text.length == 0) {
        tk_core_Program *Program_p = tk_core_getCurrentProgram(Window_p);
        if (!Program_p || !Program_p->Prototype_p) {return TK_CORE_SUCCESS;}
        if (Program_p->Prototype_p->Callbacks.drawTopbar_f) {
            TK_CHECK(Program_p->Prototype_p->Callbacks.drawTopbar_f(Program_p, Glyphs_p, cols))
        } else {
            for (int i = 0; i < cols && Program_p->Prototype_p->name_p[i] != 0; ++i) {
                Glyphs_p[i] = tk_core_getGlyphHelper(Program_p->Prototype_p->name_p[i]);
            }
        }
    } else {
        for (int i = 0, j = 0; i < cols && j < Topbar_p->Message.Text.length; ++i, ++j) {
            Glyphs_p[i] = tk_core_getGlyphHelper(Topbar_p->Message.Text.p[j]);
        }
    }

    return TK_CORE_SUCCESS;
}

static TK_CORE_RESULT tk_core_drawTopbarText(
    tk_core_Config *Config_p, tk_core_Tile *Tile_p, tk_core_Glyph *Glyphs_p, int cols)
{
    tk_core_Topbar *Topbar_p = &TK_CORE_MACRO_TAB(Tile_p)->Topbar;
    tk_core_MicroWindow *MicroWindow_p = &TK_CORE_MACRO_TAB(Tile_p)->MicroWindow;

    if (!Topbar_p->hasFocus) 
    {
        tk_core_TTY *TTY_p = nh_core_getWorkloadArg();

        NH_API_UTF32 t1_p[15] = {' ', '(', 'M', 'I', 'C', 'R', 'O', ' ', 'T', 'I', 'L', 'I', 'N', 'G', ')'};
        NH_API_UTF32 t2_p[15] = {' ', '(', 'M', 'A', 'C', 'R', 'O', ' ', 'T', 'I', 'L', 'I', 'N', 'G', ')'};

        tk_core_MacroWindow *MacroWindow_p = TTY_p->Window_p;
        if (MacroWindow_p->Tiling.stage == TK_CORE_TILING_STAGE_DONE) {
            tk_core_clearMessage(Topbar_p);
        }

        TK_CHECK(tk_core_drawTopbarMessage(Config_p, Topbar_p, MicroWindow_p, Glyphs_p, cols))

        if (Config_p->Topbar.on) {
            tk_core_Topbar *TopbarCompare_p = &TK_CORE_MACRO_TAB(MacroWindow_p->Tile_p)->Topbar;
            if (MacroWindow_p->Tiling.stage != TK_CORE_TILING_STAGE_DONE && TopbarCompare_p == Topbar_p) {
//                if (MacroWindow_p->Tiling.mode == TK_CORE_TILING_MODE_MICRO) {
//                    for (int i = cols-15, j = 0; j < 15; ++i, ++j) {
//                        Glyphs_p[i].codepoint = t1_p[j];
//                    }
//                } else if (MacroWindow_p->Tiling.mode == TK_CORE_TILING_MODE_MACRO) {
//                    for (int i = cols-15, j = 0; j < 15; ++i, ++j) {
//                        Glyphs_p[i].codepoint = t2_p[j];
//                    }
//                }
            } else {
//                for (int i = 0; i < Config.tabs; ++i) {
//                    Glyphs_p[i+1].codepoint = 0x25a1;
//                }
//                Topbar_p->quitPosition = cols-2;
//                Glyphs_p[TK_CORE_MACRO_TILE(Tile_p)->current+1].codepoint = 0x25a0;
//                Glyphs_p[Topbar_p->quitPosition].codepoint = Topbar_p->quitHover ? 0x25cf : 0x25cb;
            }
        }
        return TK_CORE_SUCCESS;
    }

//    if (Topbar_p->Command.length > 0) {
//        TK_CHECK(tk_core_drawTopbarCommand(Topbar_p, &Glyphs_p, &cols))
//    }
//
//    TK_CHECK(tk_core_drawTopbarSuggestion(Topbar_p, MicroWindow_p, Glyphs_p, cols))
//
//    NH_API_UTF32 cmd_p[14]  = {' ', '(', 'E', 'X', 'E', 'C', 'U', 'T', 'E', ' ', 'C', 'M', 'D', ')'};
//    NH_API_UTF32 prog_p[14] = {' ', '(', 'S', 'W', 'I', 'T', 'C', 'H', ' ', 'P', 'R', 'O', 'G', ')'};
//
//    if (Topbar_p->state == 0 && cols > 14) {
//        for (int i = cols-15, j = 0; j < 14; ++i, ++j) {
//            Glyphs_p[i].codepoint = cmd_p[j];
//        }
//    }
//    if (Topbar_p->state == 1 && cols > 14) {
//        for (int i = cols-15, j = 0; j < 14; ++i, ++j) {
//            Glyphs_p[i].codepoint = prog_p[j];
//        }
//    }

    return TK_CORE_SUCCESS;
}

TK_CORE_RESULT tk_core_drawTopbarRow(
    tk_core_Tile *Tile_p, tk_core_Glyph *Glyphs_p, int cols, int row, bool standardIO)
{
    tk_core_TTY *TTY_p = nh_core_getWorkloadArg();

    for (int i = 0; i < cols; ++i) {
        Glyphs_p[i] = tk_core_getGlyphHelper(' ');
        if (!Tile_p && !standardIO) {
            Glyphs_p[i].codepoint = 'q';
            Glyphs_p[i].Attributes.reverse = false;
            Glyphs_p[i].mark |= TK_CORE_MARK_LINE_GRAPHICS;
        }
    }

    if (Tile_p) {
        TK_CHECK(tk_core_drawTopbarText(&TTY_p->Config, Tile_p, Glyphs_p, cols))
    }

    for (int i = 0; i < cols && !(TTY_p->Config.Topbar.on); ++i) {
        if (Glyphs_p[i].codepoint == ' ' || Glyphs_p[i].codepoint == 0) {
            Glyphs_p[i] = tk_core_getGlyphHelper('q');
            Glyphs_p[i].Attributes.reverse = false;
            Glyphs_p[i].mark |= TK_CORE_MARK_LINE_GRAPHICS;
        }
    }

//    if (Tile_p && tk_core_getConfig().Topbar.on == false && tk_core_getConfig().Titlebar.on == true) {
//        for (int i = 1, j = 1; i <= tk_core_getConfig().tabs; ++i, j+=2) {
//            Glyphs_p[cols-(i+j)].codepoint = 'p';
//            Glyphs_p[cols-(i+j+1)].codepoint = 'p';
//            if (tk_core_getConfig().tabs - TK_CORE_MACRO_TILE(Tile_p)->current == i) {
//                Glyphs_p[cols-(i+j)].codepoint = 'z';
//                Glyphs_p[cols-(i+j+1)].codepoint = 'z';
//            }
//        }
//    }

    return TK_CORE_SUCCESS;
}

// UPDATE ==========================================================================================

TK_CORE_RESULT tk_core_updateTopbar(
    tk_core_Topbar *Topbar_p)
{
    nh_core_SystemTime Now = nh_core_getSystemTime();

    if (nh_core_getSystemTimeDiffInSeconds(Topbar_p->Suggestions.Time, Now) 
	>= Topbar_p->Suggestions.interval) 
    {
        Topbar_p->Suggestions.Time = Now;
        Topbar_p->Suggestions.update = true;
        Topbar_p->refresh = true;
    }

    return TK_CORE_SUCCESS;
}
