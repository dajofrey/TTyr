#ifndef TTYR_CORE_TILING_H
#define TTYR_CORE_TILING_H

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

    typedef enum TTYR_CORE_TILE_TYPE_E { 
        TTYR_CORE_TILE_TYPE_MACRO,
        TTYR_CORE_TILE_TYPE_MICRO,
    } TTYR_CORE_TILE_TYPE_E;

    typedef enum TTYR_CORE_TILING_MODE_E { 
        TTYR_CORE_TILING_MODE_MICRO,
        TTYR_CORE_TILING_MODE_MACRO,
    } TTYR_CORE_TILING_MODE_E;

    typedef enum TTYR_CORE_TILING_STAGE_E { 
        TTYR_CORE_TILING_STAGE_DONE,
        TTYR_CORE_TILING_STAGE_OVERVIEW,
        TTYR_CORE_TILING_STAGE_INSERT,
    } TTYR_CORE_TILING_STAGE_E;

    typedef enum TTYR_CORE_TILE_ORIENTATION { 
        TTYR_CORE_TILE_ORIENTATION_VERTICAL,
        TTYR_CORE_TILE_ORIENTATION_HORIZONTAL,
    } TTYR_CORE_TILE_ORIENTATION;

/** @} */

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct tk_core_Tiling {
        TTYR_CORE_TILING_MODE_E mode;
        TTYR_CORE_TILING_STAGE_E stage;
    } tk_core_Tiling;

    typedef struct tk_core_Tile {
        TTYR_CORE_TILE_TYPE_E type;
        void *p;
        TTYR_CORE_TILE_ORIENTATION orientation;
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
        void *p, TTYR_CORE_TILE_TYPE_E type, tk_core_Tile *Parent_p, int index
    );

    TTYR_CORE_RESULT tk_core_closeTile(
        tk_core_Tile *Tile_p, void *MicroTab_p
    );

    tk_core_Tile *tk_core_switchTile(
        tk_core_MacroWindow *Window_p, tk_core_Tile *Tile_p, int direction
    );

    TTYR_CORE_RESULT tk_core_resetTiling(
        tk_core_MacroWindow *Tab_p
    );

    TTYR_CORE_RESULT tk_core_updateTiling(
        tk_core_Tile *RootTile_p, int viewRows, int viewCols
    );

    TTYR_CORE_RESULT tk_core_handleTilingInput(
        tk_core_MacroWindow *Tab_p, nh_api_KeyboardEvent Event 
    );

    nh_core_List tk_core_getTiles(
        tk_core_Tile *Root_p
    );
    
/** @} */

#endif 
