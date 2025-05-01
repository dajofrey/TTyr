// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Tiling.h"
#include "TTY.h"
#include "Macro.h"
#include "Micro.h"

#include "../Common/Macros.h"

#include "nh-core/System/Memory.h"
#include "nh-core/System/Thread.h"

#include "nh-encoding/Encodings/UTF32.h"
#include "nh-encoding/Encodings/UTF8.h"

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// INIT/FREE =======================================================================================

tk_core_Tile *tk_core_createTile(
    void *p, TTYR_CORE_TILE_TYPE_E type, tk_core_Tile *Parent_p, int index)
{
    tk_core_Tile *Tile_p = (tk_core_Tile*)nh_core_allocate(sizeof(tk_core_Tile));
    TTYR_CHECK_MEM_2(NULL, Tile_p)

    Tile_p->type = type;
    Tile_p->p = p;
    Tile_p->orientation = TTYR_CORE_TILE_ORIENTATION_VERTICAL;
    Tile_p->rightSeparator = false;
    Tile_p->refresh  = false;
    Tile_p->Children = nh_core_initLinkedList();
    Tile_p->Parent_p = Parent_p;
    Tile_p->Prev_p   = NULL;
    Tile_p->close    = false;

    if (Parent_p != NULL) {
        nh_core_insertIntoLinkedList(&Parent_p->Children, Tile_p, index);
    }

    return Tile_p;
}

// Helper function for destroying a tile without side effects.
static void tk_core_destroyTile(
    tk_core_Tile *Tile_p)
{
    if (Tile_p->p) {
        switch (Tile_p->type) {
            case TTYR_CORE_TILE_TYPE_MACRO : tk_core_destroyMacroTile(Tile_p->p); break;
            case TTYR_CORE_TILE_TYPE_MICRO : tk_core_destroyMicroTile(Tile_p->p); break;
        }
    }

    nh_core_destroyLinkedList(&Tile_p->Children, false); 
    nh_core_free(Tile_p);
}

static tk_core_Tile *tk_core_getNextFocusTile(
    tk_core_Tile *Tile_p)
{
    while (Tile_p->Children.count > 0) {
        Tile_p = nh_core_getFromLinkedList(&Tile_p->Children, 0);
    }
    return Tile_p;
}

TTYR_CORE_RESULT tk_core_closeTile(
    tk_core_Tile *Tile_p, void *p)
{
    if (!Tile_p) {return TTYR_CORE_SUCCESS;}

    if (!Tile_p->Parent_p) {

        // Handle root tile.
        if (Tile_p->type == TTYR_CORE_TILE_TYPE_MACRO) {
            ((tk_core_MacroWindow*)p)->Tile_p = NULL;
            ((tk_core_MacroWindow*)p)->LastFocus_p = NULL;
            ((tk_core_MacroWindow*)p)->RootTile_p = NULL;
        } else if (Tile_p->type == TTYR_CORE_TILE_TYPE_MICRO) {
            ((tk_core_MicroTab*)p)->Tile_p = NULL;
            ((tk_core_MicroTab*)p)->LastFocus_p = NULL;
            ((tk_core_MicroTab*)p)->RootTile_p = NULL;
        }
        tk_core_destroyTile(Tile_p);

    } else if (Tile_p->Parent_p->Children.count == 2) {
        // Get neighbor tile.
        int index = nh_core_getLinkedListIndex(&Tile_p->Parent_p->Children, Tile_p);
        tk_core_Tile *Other_p = nh_core_getFromLinkedList(&Tile_p->Parent_p->Children, index == 0 ? 1 : 0);

        if (Tile_p->Parent_p->Parent_p) {
            for (int i = 0; i < Tile_p->Parent_p->Parent_p->Children.count; ++i) {
                if (nh_core_getFromLinkedList(&Tile_p->Parent_p->Parent_p->Children, i) == Tile_p->Parent_p) {
                    nh_core_setInLinkedList(&Tile_p->Parent_p->Parent_p->Children, i, Other_p);
                    Other_p->Parent_p = Tile_p->Parent_p->Parent_p;
                    Other_p->Prev_p = NULL;
                    break;
                }
            }

            tk_core_Tile *Focus_p = tk_core_getNextFocusTile(Other_p);

            // Set other tile as focused tile.
            if (Tile_p->type == TTYR_CORE_TILE_TYPE_MACRO) {
                ((tk_core_MacroWindow*)p)->Tile_p = Focus_p;
                ((tk_core_MacroWindow*)p)->LastFocus_p = Focus_p;
            } else if (Tile_p->type == TTYR_CORE_TILE_TYPE_MICRO) {
                ((tk_core_MicroTab*)p)->Tile_p = Focus_p;
                ((tk_core_MicroTab*)p)->LastFocus_p = Focus_p;
            }

            // Just destroy Tile_p and parent tile..
            tk_core_destroyTile(Tile_p->Parent_p);
            tk_core_destroyTile(Tile_p);
        } else {
            tk_core_Tile *Focus_p = tk_core_getNextFocusTile(Other_p);

            // Set other tile as new root tile and set focus.
            if (Tile_p->type == TTYR_CORE_TILE_TYPE_MACRO) {
		((tk_core_MacroWindow*)p)->Tile_p = Focus_p;
                ((tk_core_MacroWindow*)p)->LastFocus_p = Focus_p;
                ((tk_core_MacroWindow*)p)->RootTile_p = Other_p;
            } else if (Tile_p->type == TTYR_CORE_TILE_TYPE_MICRO) {
                ((tk_core_MicroTab*)p)->Tile_p = Focus_p;
                ((tk_core_MicroTab*)p)->LastFocus_p = Focus_p;
                ((tk_core_MicroTab*)p)->RootTile_p = Other_p;
            }

            // Destroy tiles and update Other_p.
            tk_core_destroyTile(Tile_p->Parent_p);
            tk_core_destroyTile(Tile_p);
            Other_p->Parent_p = NULL;
            Other_p->Prev_p = NULL;
        }
    } else {
        // Get neighbor tile index.
        int index = nh_core_getLinkedListIndex(&Tile_p->Parent_p->Children, Tile_p);
        index = index == 0 ? 1 : index - 1;

        // Get next focused tile.
        tk_core_Tile *Next_p = nh_core_getFromLinkedList(&Tile_p->Parent_p->Children, index);
        tk_core_Tile *Focus_p = tk_core_getNextFocusTile(Next_p);

        // Switch focus to neighbor tile.
        if (Tile_p->type == TTYR_CORE_TILE_TYPE_MACRO) {
            ((tk_core_MacroWindow*)p)->Tile_p = Focus_p;
            ((tk_core_MacroWindow*)p)->LastFocus_p = Focus_p;
        } else if (Tile_p->type == TTYR_CORE_TILE_TYPE_MICRO) {
            ((tk_core_MicroTab*)p)->Tile_p = Focus_p;
            ((tk_core_MicroTab*)p)->LastFocus_p = Focus_p;
        }

        // Remove and destroy Tile_p. 
        nh_core_removeFromLinkedList2(&Tile_p->Parent_p->Children, Tile_p, false);
        tk_core_destroyTile(Tile_p);
    }

    return TTYR_CORE_SUCCESS;
}

// GET =============================================================================================

static int tk_core_getTileNumber(
    tk_core_Tile *Current_p, tk_core_Tile *Stop_p, int *nr_p)
{
    if (Current_p == Stop_p) {return *nr_p;}

    for (int i = 0; i < Current_p->Children.count; ++i) {
        tk_core_Tile *Child_p = nh_core_getFromLinkedList(&Current_p->Children, i);
        if (Child_p->Children.count <= 0) {*nr_p += 1;}
        int result = tk_core_getTileNumber(Child_p, Stop_p, nr_p);
        if (result > -1) {return result;}
    }

    return -1;
}

// d -> right -> horizontal orientation -> 
// a -> left ->
// w -> up ->
static tk_core_Tile *tk_core_getTileFromNumber(
    tk_core_Tile *Current_p, int stop, int *nr_p)
{
    if (*nr_p == stop) {return Current_p;}

    for (int i = 0; i < Current_p->Children.count; ++i) {
        tk_core_Tile *Child_p = nh_core_getFromLinkedList(&Current_p->Children, i);
        if (Child_p->Children.count <= 0) {*nr_p += 1;}
        tk_core_Tile *Result_p = tk_core_getTileFromNumber(Child_p, stop, nr_p);
        if (Result_p != NULL) {return Result_p;}
    }

    return NULL;
}

nh_core_List tk_core_getTiles(
    tk_core_Tile *Root_p)
{
    nh_core_List List = nh_core_initList(4);

    for (int i = 0; Root_p != NULL;++i) {
        int start = 0;
        tk_core_Tile *Tile_p = tk_core_getTileFromNumber(Root_p, i, &start);
        if (Tile_p == NULL) {break;}
        nh_core_appendToList(&List, Tile_p);
    }

    return List;
}

static int tk_core_getTileIndex(
    tk_core_Tile *Tile_p)
{
    for (int i = 0; i < Tile_p->Parent_p->Children.count; ++i) {
        tk_core_Tile *Child_p = nh_core_getFromLinkedList(&Tile_p->Parent_p->Children, i);
        if (Child_p == Tile_p) {return i;}
    }
    return -1;
}

// SWITCH ==========================================================================================

static TTYR_CORE_RESULT tk_core_leaveTilingAndFocusTile(
    tk_core_MacroWindow *Window_p, tk_core_Tile *Focus_p)
{
    if (Focus_p->type == TTYR_CORE_TILE_TYPE_MICRO) {
        TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(Window_p->Tile_p))->Tile_p = Focus_p;
    } else {
        Window_p->Tile_p = Focus_p;
    }

    Window_p->Tiling.stage = TTYR_CORE_TILING_STAGE_DONE;
    Window_p->Tiling.mode  = TTYR_CORE_TILING_MODE_MICRO;

    tk_core_TTY *TTY_p = nh_core_getWorkloadArg();
    TTY_p->InsertTile_p = NULL;

    return TTYR_CORE_SUCCESS;
}

static tk_core_Tile *tk_core_getNextDirectionalTile(
    tk_core_Tile *Tile_p, int direction)
{
    if (!Tile_p->Parent_p) {
        return NULL;
    }

    switch (direction) {
        case 1 : // right
        case 3 : // left
            if (Tile_p->Parent_p->orientation != TTYR_CORE_TILE_ORIENTATION_VERTICAL) {
                return NULL;
            }
            for (int i = 0; i < Tile_p->Parent_p->Children.count; ++i) {
                tk_core_Tile *Child_p = nh_core_getFromLinkedList(&Tile_p->Parent_p->Children, i);
                if (Child_p == Tile_p) {
                    if (direction == 3 && i > 0) {
                        return nh_core_getFromLinkedList(&Tile_p->Parent_p->Children,i-1);
                    }
                    if (direction == 1 && (i+1 < Tile_p->Parent_p->Children.count)) {
                        return nh_core_getFromLinkedList(&Tile_p->Parent_p->Children,i+1);
                    }
                }
            }
            break;
        case 0 : // top
        case 2 : // bottom
            if (Tile_p->Parent_p->orientation != TTYR_CORE_TILE_ORIENTATION_HORIZONTAL) {
                return NULL;
            }
            for (int i = 0; i < Tile_p->Parent_p->Children.count; ++i) {
                tk_core_Tile *Child_p = nh_core_getFromLinkedList(&Tile_p->Parent_p->Children, i);
                if (Child_p == Tile_p) {
                    if (direction == 0 && i > 0) {
                        return nh_core_getFromLinkedList(&Tile_p->Parent_p->Children,i-1);
                    }
                    if (direction == 2 && (i+1 < Tile_p->Parent_p->Children.count)) {
                        return nh_core_getFromLinkedList(&Tile_p->Parent_p->Children,i+1);
                    }
                }
            }
            break;
    }

    return NULL;
}

tk_core_Tile *tk_core_switchTile(
    tk_core_MacroWindow *Window_p, tk_core_Tile *MacroTile_p, int direction)
{
    tk_core_Tile *MicroTile_p = TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(MacroTile_p))->Tile_p;

    tk_core_Tile *Result_p = NULL;
    while (!Result_p && MicroTile_p) {
        Result_p = tk_core_getNextDirectionalTile(MicroTile_p, direction);
        MicroTile_p = MicroTile_p->Parent_p;
    }

    while (!Result_p && MacroTile_p) {
        Result_p = tk_core_getNextDirectionalTile(MacroTile_p, direction);
        MacroTile_p = MacroTile_p->Parent_p;
    }

    if (Result_p) {
        Result_p = tk_core_getNextFocusTile(Result_p);
        tk_core_leaveTilingAndFocusTile(Window_p, Result_p);
    }

    return Result_p;
}

// UPDATE ==========================================================================================

static TTYR_CORE_RESULT tk_core_computeTileSizeRecursively(
    tk_core_Tile *Tile_p, int viewRows, int viewCols)
{
    Tile_p->rightSeparator = false;

    if (Tile_p->Parent_p == NULL) {
        Tile_p->rowPosition = 0;
        Tile_p->colPosition = 0;
        Tile_p->rowSize = viewRows;
        Tile_p->colSize = viewCols;
    }
    else {
        Tile_p->rowPosition = Tile_p->rowPosition;
        Tile_p->colPosition = Tile_p->colPosition;
        Tile_p->rowSize = Tile_p->rowSize;
        Tile_p->colSize = Tile_p->colSize;
    }

    // Compute vertical borders.
    Tile_p->rightSeparator = Tile_p->colPosition + Tile_p->colSize < viewCols;

    for (int i = 0; i < Tile_p->Children.count; ++i) 
    {
        tk_core_Tile *Child_p = nh_core_getFromLinkedList(&Tile_p->Children, i);

        switch (Tile_p->orientation)
        {
            case TTYR_CORE_TILE_ORIENTATION_VERTICAL :
                Child_p->colSize     = Tile_p->colSize / Tile_p->Children.count;
                Child_p->colPosition = Tile_p->colPosition + Child_p->colSize * i;
                Child_p->rowSize     = Tile_p->rowSize;
                Child_p->rowPosition = Tile_p->rowPosition;
                if (Tile_p->colSize % Tile_p->Children.count > 0 && i == Tile_p->Children.count - 1) {
                    Child_p->colSize += Tile_p->colSize % Tile_p->Children.count;
                }
                break;
            case TTYR_CORE_TILE_ORIENTATION_HORIZONTAL :
                Child_p->colSize     = Tile_p->colSize;
                Child_p->colPosition = Tile_p->colPosition;
                Child_p->rowSize     = Tile_p->rowSize / Tile_p->Children.count;
                Child_p->rowPosition = Tile_p->rowPosition + Child_p->rowSize * i;
                if (Tile_p->rowSize % Tile_p->Children.count > 0 && i == Tile_p->Children.count - 1) {
                    Child_p->rowSize += Tile_p->rowSize % Tile_p->Children.count;
                }
                break;
        }

        TTYR_CHECK(tk_core_computeTileSizeRecursively(Child_p, viewRows, viewCols))
    }

    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT tk_core_updateTiling(
    tk_core_Tile *RootTile_p, int viewRows, int viewCols)
{
    return tk_core_computeTileSizeRecursively(RootTile_p, viewRows, viewCols);
}

// INSERT ==========================================================================================

static void tk_core_moveInsertTile(
    tk_core_Tile *Tile_p, NH_API_UTF32 c)
{
    int index = tk_core_getTileIndex(Tile_p);
    if (index == -1) {return;}

    switch (c) 
    {
        case TTYR_CORE_INSERT_TILE_TOP_KEY :
            Tile_p->Parent_p->orientation = TTYR_CORE_TILE_ORIENTATION_HORIZONTAL;
            if (index > 0) {
                nh_core_setInLinkedList(&Tile_p->Parent_p->Children, index, nh_core_getFromLinkedList(&Tile_p->Parent_p->Children, index - 1));
                nh_core_setInLinkedList(&Tile_p->Parent_p->Children, index - 1, Tile_p);
            }
            break;

        case TTYR_CORE_INSERT_TILE_BOTTOM_KEY :
            Tile_p->Parent_p->orientation = TTYR_CORE_TILE_ORIENTATION_HORIZONTAL;
            if (index < Tile_p->Parent_p->Children.count - 1) {
                nh_core_setInLinkedList(&Tile_p->Parent_p->Children, index, nh_core_getFromLinkedList(&Tile_p->Parent_p->Children, index + 1));
                nh_core_setInLinkedList(&Tile_p->Parent_p->Children, index + 1, Tile_p);
            }
            break;

        case TTYR_CORE_INSERT_TILE_LEFT_KEY :
            Tile_p->Parent_p->orientation = TTYR_CORE_TILE_ORIENTATION_VERTICAL;
            if (index > 0) {
                nh_core_setInLinkedList(&Tile_p->Parent_p->Children, index, nh_core_getFromLinkedList(&Tile_p->Parent_p->Children, index - 1));
                nh_core_setInLinkedList(&Tile_p->Parent_p->Children, index - 1, Tile_p);
            }
            break;

        case TTYR_CORE_INSERT_TILE_RIGHT_KEY :
            Tile_p->Parent_p->orientation = TTYR_CORE_TILE_ORIENTATION_VERTICAL;
            if (index < Tile_p->Parent_p->Children.count - 1) {
                nh_core_setInLinkedList(&Tile_p->Parent_p->Children, index, nh_core_getFromLinkedList(&Tile_p->Parent_p->Children, index + 1));
                nh_core_setInLinkedList(&Tile_p->Parent_p->Children, index + 1, Tile_p);
            }
            break;
    }
}

static TTYR_CORE_RESULT tk_core_insertEmptyTile(
    tk_core_Tile *Parent_p, NH_API_UTF32 c)
{
    tk_core_TTY *TTY_p = nh_core_getWorkloadArg();

    int orientation = 0;

    switch (c) 
    {
        case TTYR_CORE_INSERT_TILE_TOP_KEY    :
        case TTYR_CORE_INSERT_TILE_BOTTOM_KEY :
            orientation = TTYR_CORE_TILE_ORIENTATION_HORIZONTAL;
            break;
        case TTYR_CORE_INSERT_TILE_LEFT_KEY  :
        case TTYR_CORE_INSERT_TILE_RIGHT_KEY :
            orientation = TTYR_CORE_TILE_ORIENTATION_VERTICAL;
            break;
    }

    if (Parent_p->Children.count > 0 && Parent_p->orientation != orientation) {
        // Changing orientation is not allowed because it's confusing.
        return TTYR_CORE_ERROR_BAD_STATE;
    }

    // Configure orientation.
    Parent_p->orientation = orientation;

    // Insert tile. The inserted tile is created without data.
    switch (c)
    {
        case TTYR_CORE_INSERT_TILE_TOP_KEY  :
        case TTYR_CORE_INSERT_TILE_LEFT_KEY :
            if (Parent_p->type == TTYR_CORE_TILE_TYPE_MACRO) {
                if (Parent_p->Children.count == 0) {
                    nh_core_List *List_pp[9] = {};
                    for (int i = 0; i < 9; ++i) {
                        List_pp[i] = TTYR_CORE_MACRO_TAB_2(Parent_p, i)->MicroWindow.Tabs_p;
                    }
                    TTY_p->InsertTile_p     = tk_core_createMacroTile(Parent_p, NULL, 0);
                    TTY_p->Window_p->Tile_p = tk_core_createMacroTile(Parent_p, List_pp, 1);
                    // Remove data references from parent tile.
                    nh_core_freeList(&TTYR_CORE_MACRO_TILE(Parent_p)->MacroTabs, true);
                    TTYR_CORE_MACRO_TILE(Parent_p)->current = -1;
                } else {
                    TTY_p->InsertTile_p = tk_core_createMacroTile(Parent_p, NULL, 0);
                }
            } else {
                if (Parent_p->Children.count == 0) {
                    TTY_p->InsertTile_p = tk_core_createMicroTile(Parent_p, NULL, 0);
                    tk_core_createMicroTile(Parent_p, TTYR_CORE_MICRO_TILE(Parent_p)->Program_p, 1);
                    // Remove data references from parent tile.
                    TTYR_CORE_MICRO_TILE(Parent_p)->Program_p = NULL;
                } else {
                    TTY_p->InsertTile_p = tk_core_createMicroTile(Parent_p, NULL, 0);
                }
            }
            break;
        case TTYR_CORE_INSERT_TILE_BOTTOM_KEY :
        case TTYR_CORE_INSERT_TILE_RIGHT_KEY  :
            if (Parent_p->type == TTYR_CORE_TILE_TYPE_MACRO) {
                if (Parent_p->Children.count == 0) {
                    nh_core_List *List_pp[9] = {};
                    for (int i = 0; i < 9; ++i) {
                        List_pp[i] = TTYR_CORE_MACRO_TAB_2(Parent_p, i)->MicroWindow.Tabs_p;
                    }
                    TTY_p->Window_p->Tile_p = tk_core_createMacroTile(Parent_p, List_pp, 0);
                    TTY_p->InsertTile_p     = tk_core_createMacroTile(Parent_p, NULL, 1);
                    // Remove data references from parent tile.
                    nh_core_freeList(&TTYR_CORE_MACRO_TILE(Parent_p)->MacroTabs, true);
                    TTYR_CORE_MACRO_TILE(Parent_p)->current = -1;
                } else {
                    TTY_p->InsertTile_p = tk_core_createMacroTile(Parent_p, NULL, Parent_p->Children.count);
                }
            } else {
                if (Parent_p->Children.count == 0) {
                    tk_core_createMicroTile(Parent_p, TTYR_CORE_MICRO_TILE(Parent_p)->Program_p, 0);
                    TTY_p->InsertTile_p = tk_core_createMicroTile(Parent_p, NULL, 1);
                    // Remove data references from parent tile.
                    TTYR_CORE_MICRO_TILE(Parent_p)->Program_p = NULL;
                } else {
                    TTY_p->InsertTile_p = tk_core_createMicroTile(Parent_p, NULL, Parent_p->Children.count);
                }
            }
            break;
    }

    return TTYR_CORE_SUCCESS;
}

static TTYR_CORE_RESULT tk_core_addTile(
    tk_core_Tile *Tile_p, NH_API_UTF32 c)
{
    if (Tile_p->Parent_p == NULL || Tile_p->Parent_p->Children.count == 0) {return TTYR_CORE_ERROR_BAD_STATE;}
    return tk_core_insertEmptyTile(Tile_p->Parent_p, c);
}

static TTYR_CORE_RESULT tk_core_splitTile(
    tk_core_Tile *Tile_p, NH_API_UTF32 c)
{
    if (Tile_p->Children.count != 0) {return TTYR_CORE_ERROR_BAD_STATE;}
    return tk_core_insertEmptyTile(Tile_p, c);
}

// RESET ===========================================================================================

TTYR_CORE_RESULT tk_core_resetTiling(
    tk_core_MacroWindow *Window_p)
{
    tk_core_TTY *TTY_p = nh_core_getWorkloadArg();

    if (Window_p->Tiling.mode == TTYR_CORE_TILING_MODE_MICRO) {
        switch (Window_p->Tiling.stage)
        {
            case TTYR_CORE_TILING_STAGE_OVERVIEW :
                TTYR_CHECK(tk_core_leaveTilingAndFocusTile(Window_p, TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(Window_p->Tile_p))->LastFocus_p))
                break;

            case TTYR_CORE_TILING_STAGE_INSERT :
                tk_core_closeTile(TTY_p->InsertTile_p, TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(Window_p->Tile_p)));
                TTYR_CHECK(tk_core_leaveTilingAndFocusTile(Window_p, TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(Window_p->Tile_p))->LastFocus_p))
                break;
        }
    } else {
        switch (Window_p->Tiling.stage)
        {
            case TTYR_CORE_TILING_STAGE_OVERVIEW :
                TTYR_CHECK(tk_core_leaveTilingAndFocusTile(Window_p, Window_p->LastFocus_p))
                break;

            case TTYR_CORE_TILING_STAGE_INSERT :
                tk_core_closeTile(TTY_p->InsertTile_p, Window_p);
                TTYR_CHECK(tk_core_leaveTilingAndFocusTile(Window_p, Window_p->LastFocus_p))
                break;
        }
    }

    return TTYR_CORE_SUCCESS;
}

// MESSAGES ========================================================================================

static TTYR_CORE_RESULT tk_core_updateTilingMessages(
    tk_core_MacroWindow *Window_p)
{
    if (Window_p->Tiling.mode == TTYR_CORE_TILING_MODE_MACRO) {
        if (Window_p->Tile_p->Parent_p == NULL) {
            TTYR_CHECK(tk_core_setDefaultMessage(&TTYR_CORE_MACRO_TAB(Window_p->Tile_p)->Topbar, TTYR_CORE_MESSAGE_MACRO_TILING_WASD))
        } else {
            TTYR_CHECK(tk_core_setDefaultMessage(&TTYR_CORE_MACRO_TAB(Window_p->Tile_p)->Topbar, TTYR_CORE_MESSAGE_MACRO_TILING_WASDF))
        }
    } else if (Window_p->Tiling.mode == TTYR_CORE_TILING_MODE_MICRO) {
        if (TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(Window_p->Tile_p))->Tile_p->Parent_p == NULL) {
            TTYR_CHECK(tk_core_setDefaultMessage(&TTYR_CORE_MACRO_TAB(Window_p->Tile_p)->Topbar, TTYR_CORE_MESSAGE_MICRO_TILING_WASD))
        } else {
            TTYR_CHECK(tk_core_setDefaultMessage(&TTYR_CORE_MACRO_TAB(Window_p->Tile_p)->Topbar, TTYR_CORE_MESSAGE_MICRO_TILING_WASDF))
        }
    }

    return TTYR_CORE_SUCCESS;
}

// INPUT ===========================================================================================

static TTYR_CORE_RESULT tk_core_handlePotentialMacroTileInsertion(
    tk_core_MacroWindow *Window_p, NH_API_UTF32 c)
{
    tk_core_TTY *TTY_p = nh_core_getWorkloadArg();

    if (c == 13) 
    {
        // Insert data into new macro tile.
        for (int i = 0; i < 9; ++i) {
            TTYR_CORE_MACRO_TAB_2(TTY_p->InsertTile_p, i)->MicroWindow.Tabs_p = tk_core_createMicroTabs(TTY_p);
            TTYR_CORE_MACRO_TAB_2(TTY_p->InsertTile_p, i)->MicroWindow.current = 0; 
        }

        return tk_core_leaveTilingAndFocusTile(Window_p, TTY_p->InsertTile_p);
    }

    switch (c) 
    {
        case TTYR_CORE_INSERT_TILE_TOP_KEY    :
        case TTYR_CORE_INSERT_TILE_BOTTOM_KEY :
        case TTYR_CORE_INSERT_TILE_LEFT_KEY   :
        case TTYR_CORE_INSERT_TILE_RIGHT_KEY  :

            if (TTY_p->InsertTile_p) { 
                tk_core_moveInsertTile(TTY_p->InsertTile_p, c);
                break;
            }
            // Fall through.

        default :
            TTYR_CHECK(tk_core_resetTiling(Window_p))
    }

    return tk_core_updateTilingMessages(Window_p);
}

static TTYR_CORE_RESULT tk_core_handlePotentialMicroTileInsertion(
    tk_core_MacroWindow *Window_p, NH_API_UTF32 c)
{
    tk_core_TTY *TTY_p = nh_core_getWorkloadArg();

    if (c == 13)
    {
        // Insert data into new micro tile.
        TTYR_CORE_MICRO_TILE(TTY_p->InsertTile_p)->Program_p = tk_core_createProgramInstance(
            TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(Window_p->Tile_p))->Prototype_p, false);

        return tk_core_leaveTilingAndFocusTile(Window_p, TTY_p->InsertTile_p);
    }

    switch (c)
    {
        case TTYR_CORE_INSERT_TILE_TOP_KEY    :
        case TTYR_CORE_INSERT_TILE_BOTTOM_KEY :
        case TTYR_CORE_INSERT_TILE_LEFT_KEY   :
        case TTYR_CORE_INSERT_TILE_RIGHT_KEY  :

            tk_core_moveInsertTile(TTY_p->InsertTile_p, c);
            break;

        default :
            TTYR_CHECK(tk_core_resetTiling(Window_p))
    }

    return tk_core_updateTilingMessages(Window_p);
}

static TTYR_CORE_RESULT tk_core_handleMacroTilingInput(
    tk_core_MacroWindow *Window_p, nh_api_KeyboardEvent Event)
{
    NH_API_UTF32 c = Event.codepoint;

    switch (Window_p->Tiling.stage)
    {
        case TTYR_CORE_TILING_STAGE_OVERVIEW :

            if (c == TTYR_CORE_SPLIT_KEY) {
                // Split tile.
                TTYR_CHECK(tk_core_splitTile(Window_p->Tile_p, TTYR_CORE_INSERT_TILE_RIGHT_KEY))
                Window_p->Tiling.stage = TTYR_CORE_TILING_STAGE_INSERT;
                TTYR_CHECK(tk_core_updateTilingMessages(Window_p))
            }
            else if (c == TTYR_CORE_INSERT_TILE_LEFT_KEY || c == TTYR_CORE_INSERT_TILE_RIGHT_KEY || c == TTYR_CORE_INSERT_TILE_TOP_KEY || c == TTYR_CORE_INSERT_TILE_BOTTOM_KEY) {
                // Append or split tile.
                if (Window_p->Tile_p->Parent_p == NULL) {
                    TTYR_CHECK(tk_core_splitTile(Window_p->Tile_p, c))
                } else {
                    if (tk_core_addTile(Window_p->Tile_p, c)) {
                        return tk_core_resetTiling(Window_p);
                    }
                }
                Window_p->Tiling.stage = TTYR_CORE_TILING_STAGE_INSERT;
                TTYR_CHECK(tk_core_updateTilingMessages(Window_p))
            } else {
                TTYR_CHECK(tk_core_resetTiling(Window_p))
            }
            break;

        case TTYR_CORE_TILING_STAGE_INSERT :

            TTYR_CHECK(tk_core_handlePotentialMacroTileInsertion(Window_p, c))
            break;
    }

    return TTYR_CORE_SUCCESS;
}

static TTYR_CORE_RESULT tk_core_handleMicroTilingInput(
    tk_core_MacroWindow *Window_p, nh_api_KeyboardEvent Event)
{
    NH_API_UTF32 c = Event.codepoint;

    switch (Window_p->Tiling.stage)
    {
        case TTYR_CORE_TILING_STAGE_DONE :

            if (c == TTYR_CORE_TILING_KEY) {
                Window_p->Tiling.stage = TTYR_CORE_TILING_STAGE_OVERVIEW; 
                Window_p->LastFocus_p = Window_p->Tile_p;
                TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(Window_p->Tile_p))->LastFocus_p = 
                    TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(Window_p->Tile_p))->Tile_p;
                TTYR_CHECK(tk_core_updateTilingMessages(Window_p))
            }
            break;

        case TTYR_CORE_TILING_STAGE_OVERVIEW :

            if (c == TTYR_CORE_TILING_KEY) {
                Window_p->Tiling.mode = TTYR_CORE_TILING_MODE_MACRO;
                TTYR_CHECK(tk_core_updateTilingMessages(Window_p))
            } 
            else if (c == TTYR_CORE_SPLIT_KEY) {
                // Split tile.
                TTYR_CHECK(tk_core_splitTile(TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(Window_p->Tile_p))->Tile_p, TTYR_CORE_INSERT_TILE_RIGHT_KEY))
                Window_p->Tiling.stage = TTYR_CORE_TILING_STAGE_INSERT;
                TTYR_CHECK(tk_core_updateTilingMessages(Window_p))
            }
            else if (c == TTYR_CORE_INSERT_TILE_LEFT_KEY || c == TTYR_CORE_INSERT_TILE_RIGHT_KEY || c == TTYR_CORE_INSERT_TILE_TOP_KEY || c == TTYR_CORE_INSERT_TILE_BOTTOM_KEY) {
                // Append or split tile.
                if (TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(Window_p->Tile_p))->Tile_p->Parent_p == NULL) {
                    TTYR_CHECK(tk_core_splitTile(TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(Window_p->Tile_p))->Tile_p, c))
                } else {
                    if (tk_core_addTile(TTYR_CORE_MICRO_TAB(TTYR_CORE_MACRO_TAB(Window_p->Tile_p))->Tile_p, c)) {
                        return tk_core_resetTiling(Window_p);
                    }
                }
                Window_p->Tiling.stage = TTYR_CORE_TILING_STAGE_INSERT;
                TTYR_CHECK(tk_core_updateTilingMessages(Window_p))
            }
            else {
                TTYR_CHECK(tk_core_resetTiling(Window_p))
            }
            break;

        case TTYR_CORE_TILING_STAGE_INSERT :

            TTYR_CHECK(tk_core_handlePotentialMicroTileInsertion(Window_p, c))
            break;
    }

    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT tk_core_handleTilingInput(
    tk_core_MacroWindow *Window_p, nh_api_KeyboardEvent Event)
{
    if (Event.trigger != NH_API_TRIGGER_PRESS) {return TTYR_CORE_SUCCESS;}

    if (Window_p->Tiling.mode == TTYR_CORE_TILING_MODE_MICRO) {
        TTYR_CHECK(tk_core_handleMicroTilingInput(Window_p, Event))
    } else if (Window_p->Tiling.mode == TTYR_CORE_TILING_MODE_MACRO) {
        TTYR_CHECK(tk_core_handleMacroTilingInput(Window_p, Event))
    }

    return TTYR_CORE_SUCCESS;
}
