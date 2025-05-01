// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Program.h"
#include "TTY.h"

#include "../Editor/Editor.h"
#include "../Shell/Shell.h"
#include "../Common/Macros.h"

#include "nh-core/Loader/Loader.h"
#include "nh-core/System/Memory.h"
#include "nh-core/System/Thread.h"

#include "nh-encoding/Encodings/UTF32.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// PROGRAM INSTANCE ================================================================================

tk_core_Program *tk_core_createProgramInstance(
    tk_core_Interface *Interface_p, bool once)
{
    tk_core_Program *Program_p = (tk_core_Program*)nh_core_allocate(sizeof(tk_core_Program));
    TTYR_CHECK_MEM_2(NULL, Program_p)

    Program_p->once        = once;
    Program_p->refresh     = false;
    Program_p->close       = false;
    Program_p->Prototype_p = Interface_p;
    Program_p->handle_p    = Interface_p->Callbacks.init_f ? 
        Interface_p->Callbacks.init_f(Interface_p->initArg_p) : NULL; 
 
    return Program_p;
}

TTYR_CORE_RESULT tk_core_destroyProgramInstance(
    tk_core_Program *Program_p)
{
    if (Program_p->once) {
        Program_p->Prototype_p->Callbacks.destroyPrototype_f(Program_p->Prototype_p);
    }

    if (Program_p && Program_p->Prototype_p && Program_p->Prototype_p->Callbacks.destroy_f) {
        Program_p->Prototype_p->Callbacks.destroy_f(Program_p->handle_p);
    }

    nh_core_free(Program_p);
 
    return TTYR_CORE_SUCCESS;
}

// INTERFACE =======================================================================================

TTYR_CORE_RESULT tk_core_addProgram(
    tk_core_TTY *TTY_p, tk_core_Interface *Prototype_p, bool once)
{
//    if (once) {
//        tk_core_MacroTile *Tile_p = TTY_p->Window_p->Tile_p;
//
//        // Remove program with same name on tile if necessary.
//        for (int i = 0; Tile_p->Canvas.Programs_p && i < Tile_p->Canvas.Programs_p->length; ++i) {
//            tk_core_Program *Program_p = ((tk_core_Program*)Tile_p->Canvas.Programs_p->p)+i;
//            if (ttyr_encoding_compareUTF32(Program_p->Prototype_p->Name.p, Prototype_p->Name.p)) {
//                TTYR_CHECK(tk_core_destroyProgramInstance(((tk_core_Program*)Tile_p->Canvas.Programs_p->p)+i))
//                nh_core_removeFromArray(Tile_p->Canvas.Programs_p, i, 1);
//                if (Program_p->once) {
//                    Program_p->Prototype_p->Callbacks.destroyPrototype_f(Program_p->Prototype_p);
//                }
//            }
//        }
//
//        // Add canvas tile to tile.
//        TTYR_CHECK(tk_core_appendCanvasTile(&Tile_p->Canvas, Prototype_p, &TTY_p->Views, once))
//
//        // Switch to new program.
//        Tile_p->Canvas.currentProgram = Tile_p->Canvas.Programs_p->length-1; 
//
//        // Prevent black screen.
//        TTY_p->Window_p->refreshScreen = true;
//
//        TTYR_CORE_END(TTYR_CORE_SUCCESS)
//    }

    // Check if there already exists a program with this name.
    for (int i = 0; i < TTY_p->Prototypes.size; ++i) {
        if (nh_encoding_compareUTF32(((tk_core_Interface*)TTY_p->Prototypes.pp[i])->name_p, Prototype_p->name_p)) {
            return TTYR_CORE_ERROR_BAD_STATE;
        }
    }

    // Add program prototype to other program prototypes.
    nh_core_appendToList(&TTY_p->Prototypes, Prototype_p);

    // If necessary, Add program prototype instance to all current tiles.
    nh_core_List Tiles = tk_core_getTiles(TTY_p->Window_p->RootTile_p);
    for (int i = 0; i < Tiles.size; ++i) {
        tk_core_Tile *Tile_p = Tiles.pp[i];
        bool add = true;
        for (int j = 0; TTYR_CORE_MACRO_TAB(Tile_p)->MicroWindow.Tabs_p && j < TTYR_CORE_MACRO_TAB(Tile_p)->MicroWindow.Tabs_p->size; ++j) {
            tk_core_Interface *MicroTileProgramPrototype_p = 
                ((tk_core_MicroTab*)TTYR_CORE_MACRO_TAB(Tile_p)->MicroWindow.Tabs_p->pp[j])->Prototype_p;
            if (!MicroTileProgramPrototype_p) {continue;}
            if (nh_encoding_compareUTF32(MicroTileProgramPrototype_p->name_p, Prototype_p->name_p)) {
                add = false;
            }
        }
        if (!add) {continue;}
        for (int j = 0; j < 9; ++j) {
            TTYR_CHECK(tk_core_appendMicroTab(
                &TTYR_CORE_MACRO_TAB_2(Tile_p, j)->MicroWindow, TTY_p->Prototypes.pp[TTY_p->Prototypes.size - 1], once
            ))
        }
    }
    nh_core_freeList(&Tiles, false);

    // Prevent black screen. 
    TTY_p->Window_p->refreshGrid1 = true;

    return TTYR_CORE_SUCCESS;
}
