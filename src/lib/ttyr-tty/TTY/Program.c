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

ttyr_tty_Program *ttyr_tty_createProgramInstance(
    ttyr_tty_Interface *Interface_p, bool once)
{
    ttyr_tty_Program *Program_p = nh_core_allocate(sizeof(ttyr_tty_Program));
    TTYR_CHECK_MEM_2(NULL, Program_p)

    Program_p->once        = once;
    Program_p->refresh     = false;
    Program_p->close       = false;
    Program_p->Prototype_p = Interface_p;
    Program_p->handle_p    = Interface_p->Callbacks.init_f ? 
        Interface_p->Callbacks.init_f(Interface_p->initArg_p) : NULL; 
 
    return Program_p;
}

TTYR_TTY_RESULT ttyr_tty_destroyProgramInstance(
    ttyr_tty_Program *Program_p)
{
    if (Program_p->once) {
        Program_p->Prototype_p->Callbacks.destroyPrototype_f(Program_p->Prototype_p);
    }

    if (Program_p && Program_p->Prototype_p && Program_p->Prototype_p->Callbacks.destroy_f) {
        Program_p->Prototype_p->Callbacks.destroy_f(Program_p->handle_p);
    }

    nh_core_free(Program_p);
 
    return TTYR_TTY_SUCCESS;
}

// INTERFACE =======================================================================================

TTYR_TTY_RESULT ttyr_tty_addProgram(
    ttyr_tty_TTY *TTY_p, ttyr_tty_Interface *Prototype_p, bool once)
{
//    if (once) {
//        ttyr_tty_MacroTile *Tile_p = TTY_p->Window_p->Tile_p;
//
//        // Remove program with same name on tile if necessary.
//        for (int i = 0; Tile_p->Canvas.Programs_p && i < Tile_p->Canvas.Programs_p->length; ++i) {
//            ttyr_tty_Program *Program_p = ((ttyr_tty_Program*)Tile_p->Canvas.Programs_p->p)+i;
//            if (ttyr_encoding_compareUTF32(Program_p->Prototype_p->Name.p, Prototype_p->Name.p)) {
//                TTYR_CHECK(ttyr_tty_destroyProgramInstance(((ttyr_tty_Program*)Tile_p->Canvas.Programs_p->p)+i))
//                nh_core_removeFromArray(Tile_p->Canvas.Programs_p, i, 1);
//                if (Program_p->once) {
//                    Program_p->Prototype_p->Callbacks.destroyPrototype_f(Program_p->Prototype_p);
//                }
//            }
//        }
//
//        // Add canvas tile to tile.
//        TTYR_CHECK(ttyr_tty_appendCanvasTile(&Tile_p->Canvas, Prototype_p, &TTY_p->Views, once))
//
//        // Switch to new program.
//        Tile_p->Canvas.currentProgram = Tile_p->Canvas.Programs_p->length-1; 
//
//        // Prevent black screen.
//        TTY_p->Window_p->refreshScreen = true;
//
//        TTYR_TTY_END(TTYR_TTY_SUCCESS)
//    }

    // Check if there already exists a program with this name.
    for (int i = 0; i < TTY_p->Prototypes.size; ++i) {
        if (nh_encoding_compareUTF32(((ttyr_tty_Interface*)TTY_p->Prototypes.pp[i])->name_p, Prototype_p->name_p)) {
            return TTYR_TTY_ERROR_BAD_STATE;
        }
    }

    // Add program prototype to other program prototypes.
    nh_core_appendToList(&TTY_p->Prototypes, Prototype_p);

    // If necessary, Add program prototype instance to all current tiles.
    nh_core_List Tiles = ttyr_tty_getTiles(TTY_p->Window_p->RootTile_p);
    for (int i = 0; i < Tiles.size; ++i) {
        ttyr_tty_Tile *Tile_p = Tiles.pp[i];
        bool add = true;
        for (int j = 0; TTYR_TTY_MACRO_TAB(Tile_p)->MicroWindow.Tabs_p && j < TTYR_TTY_MACRO_TAB(Tile_p)->MicroWindow.Tabs_p->size; ++j) {
            ttyr_tty_Interface *MicroTileProgramPrototype_p = 
                ((ttyr_tty_MicroTab*)TTYR_TTY_MACRO_TAB(Tile_p)->MicroWindow.Tabs_p->pp[j])->Prototype_p;
            if (!MicroTileProgramPrototype_p) {continue;}
            if (nh_encoding_compareUTF32(MicroTileProgramPrototype_p->name_p, Prototype_p->name_p)) {
                add = false;
            }
        }
        if (!add) {continue;}
        for (int j = 0; j < 9; ++j) {
            TTYR_CHECK(ttyr_tty_appendMicroTab(
                &TTYR_TTY_MACRO_TAB_2(Tile_p, j)->MicroWindow, TTY_p->Prototypes.pp[TTY_p->Prototypes.size - 1], once
            ))
        }
    }
    nh_core_freeList(&Tiles, false);

    // Prevent black screen. 
    TTY_p->Window_p->refreshGrid1 = true;

    return TTYR_TTY_SUCCESS;
}

