#ifndef TK_CORE_TILING_H
#define TK_CORE_TILING_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "../Common/Includes.h"

#include "nh-core/Util/LinkedList.h"
#include "nh-core/Util/List.h"

typedef struct tk_core_MacroWindow tk_core_MacroWindow;

#endif

/** @addtogroup lib_nhtty_enums
 *  @{
 */

    typedef enum TK_CORE_TILE_TYPE_E { 
        TK_CORE_TILE_TYPE_MACRO,
        TK_CORE_TILE_TYPE_MICRO,
    } TK_CORE_TILE_TYPE_E;

    typedef enum TK_CORE_TILING_MODE_E { 
        TK_CORE_TILING_MODE_MICRO,
        TK_CORE_TILING_MODE_MACRO,
    } TK_CORE_TILING_MODE_E;

    typedef enum TK_CORE_TILING_STAGE_E { 
        TK_CORE_TILING_STAGE_DONE,
        TK_CORE_TILING_STAGE_OVERVIEW,
        TK_CORE_TILING_STAGE_INSERT,
    } TK_CORE_TILING_STAGE_E;

    typedef enum TK_CORE_TILE_ORIENTATION { 
        TK_CORE_TILE_ORIENTATION_VERTICAL,
        TK_CORE_TILE_ORIENTATION_HORIZONTAL,
    } TK_CORE_TILE_ORIENTATION;

/** @} */

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct tk_core_Tiling {
        TK_CORE_TILING_MODE_E mode;
        TK_CORE_TILING_STAGE_E stage;
    } tk_core_Tiling;

    typedef struct tk_core_Tile {
        TK_CORE_TILE_TYPE_E type;
        void *p;
        TK_CORE_TILE_ORIENTATION orientation;
        bool rightSeparator; 
        bool refresh;
        bool close;
        int rowPosition, colPosition;
        int rowSize, colSize;
        nh_LinkedList Children;
        struct tk_core_Tile *Parent_p;
        struct tk_core_Tile *Prev_p;
    } tk_core_Tile;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    tk_core_Tile *tk_core_createTile(
        void *p, TK_CORE_TILE_TYPE_E type, tk_core_Tile *Parent_p, int index
    );

    TK_CORE_RESULT tk_core_closeTile(
        tk_core_Tile *Tile_p, void *MicroTab_p
    );

    tk_core_Tile *tk_core_switchTile(
        tk_core_MacroWindow *Window_p, tk_core_Tile *Tile_p, int direction
    );

    TK_CORE_RESULT tk_core_resetTiling(
        tk_core_MacroWindow *Tab_p
    );

    TK_CORE_RESULT tk_core_updateTiling(
        tk_core_Tile *RootTile_p, int viewRows, int viewCols
    );

    TK_CORE_RESULT tk_core_handleTilingInput(
        tk_core_MacroWindow *Tab_p, nh_api_KeyboardEvent Event 
    );

    nh_core_List tk_core_getTiles(
        tk_core_Tile *Root_p
    );
    
/** @} */

#endif 
