#ifndef TTYR_TTY_TILING_H
#define TTYR_TTY_TILING_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "../Common/Includes.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/Util/LinkedList.h"

typedef struct ttyr_tty_MacroWindow ttyr_tty_MacroWindow;

#endif

/** @addtogroup lib_nhtty_enums
 *  @{
 */

    typedef enum TTYR_TTY_TILE_TYPE_E { 
        TTYR_TTY_TILE_TYPE_MACRO,
        TTYR_TTY_TILE_TYPE_MICRO,
    } TTYR_TTY_TILE_TYPE_E;

    typedef enum TTYR_TTY_TILING_MODE_E { 
        TTYR_TTY_TILING_MODE_MICRO,
        TTYR_TTY_TILING_MODE_MACRO,
    } TTYR_TTY_TILING_MODE_E;

    typedef enum TTYR_TTY_TILING_STAGE_E { 
        TTYR_TTY_TILING_STAGE_DONE,
        TTYR_TTY_TILING_STAGE_OVERVIEW,
        TTYR_TTY_TILING_STAGE_INSERT,
    } TTYR_TTY_TILING_STAGE_E;

    typedef enum TTYR_TTY_TILE_ORIENTATION { 
        TTYR_TTY_TILE_ORIENTATION_VERTICAL,
        TTYR_TTY_TILE_ORIENTATION_HORIZONTAL,
    } TTYR_TTY_TILE_ORIENTATION;

/** @} */

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct ttyr_tty_Tiling {
        TTYR_TTY_TILING_MODE_E mode;
        TTYR_TTY_TILING_STAGE_E stage;
    } ttyr_tty_Tiling;

    typedef struct ttyr_tty_Tile {
        TTYR_TTY_TILE_TYPE_E type;
        void *p;
        TTYR_TTY_TILE_ORIENTATION orientation;
        NH_BOOL rightSeparator; 
        NH_BOOL refresh;
        NH_BOOL close;
        int rowPosition, colPosition;
        int rowSize, colSize;
        nh_LinkedList Children;
        struct ttyr_tty_Tile *Parent_p;
        struct ttyr_tty_Tile *Prev_p;
    } ttyr_tty_Tile;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    ttyr_tty_Tile *ttyr_tty_createTile(
        void *p, TTYR_TTY_TILE_TYPE_E type, ttyr_tty_Tile *Parent_p, int index
    );

    TTYR_TTY_RESULT ttyr_tty_closeTile(
        ttyr_tty_Tile *Tile_p, void *MicroTab_p
    );

    ttyr_tty_Tile *ttyr_tty_switchTile(
        ttyr_tty_MacroWindow *Window_p, ttyr_tty_Tile *Tile_p, int direction
    );

    TTYR_TTY_RESULT ttyr_tty_resetTiling(
        ttyr_tty_MacroWindow *Tab_p
    );

    TTYR_TTY_RESULT ttyr_tty_updateTiling(
        ttyr_tty_Tile *RootTile_p, int viewRows, int viewCols
    );

    TTYR_TTY_RESULT ttyr_tty_handleTilingInput(
        ttyr_tty_MacroWindow *Tab_p, nh_wsi_KeyboardEvent Event 
    );

    nh_List ttyr_tty_getTiles(
        ttyr_tty_Tile *Root_p
    );
    
/** @} */

#endif 
