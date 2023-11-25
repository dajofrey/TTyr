#ifndef TTYR_TERMINAL_GRID_H
#define TTYR_TERMINAL_GRID_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "../Common/Includes.h"

#include "../../../../external/Netzhaut/src/lib/nhcore/Util/Time.h"
#include "../../../../external/Netzhaut/src/lib/nhgfx/Fonts/Text.h"

#endif

/** @addtogroup lib_nhterminal_structs
 *  @{
 */

    typedef struct ttyr_terminal_Box {
        NH_BOOL accent;
        nh_PixelPosition UpperLeft;
        nh_PixelPosition LowerRight;
        float innerVertices_p[18];
        float outerVertices_p[18];
    } ttyr_terminal_Box;

    typedef struct ttyr_terminal_TileUpdate {
        ttyr_tty_Glyph Glyph;
        int row;
        int col;
        NH_BOOL cursor;
    } ttyr_terminal_TileUpdate;

    typedef struct ttyr_terminal_TileForeground {
        float vertices_p[24];
    } ttyr_terminal_TileForeground;

    typedef struct ttyr_terminal_TileBackground {
        float vertices_p[12];
    } ttyr_terminal_TileBackground;

    typedef struct ttyr_terminal_Tile {
        ttyr_terminal_TileForeground Foreground;
        ttyr_terminal_TileBackground Background;
        ttyr_tty_Glyph Glyph;
        NH_BOOL dirty;
    } ttyr_terminal_Tile;

    typedef struct ttyr_terminal_Grid {
        nh_List Rows;
        nh_Array Boxes;
        nh_PixelSize TileSize;
        nh_PixelSize Size;
        int cols;
        int rows;
        ttyr_terminal_Tile *Cursor_p;
        ttyr_terminal_TileUpdate **Updates_pp;
        NH_BOOL **updates_pp;
    } ttyr_terminal_Grid;

/** @} */

/** @addtogroup lib_nhterminal_functions
 *  @{
 */

    ttyr_terminal_Tile *ttyr_terminal_getTile(
        ttyr_terminal_Grid *Grid_p, int row, int col
    );

    TTYR_TERMINAL_RESULT ttyr_terminal_initGrid(
        ttyr_terminal_Grid *Grid_p
    );

    TTYR_TERMINAL_RESULT ttyr_terminal_freeGrid(
        ttyr_terminal_Grid *Grid_p
    );

    TTYR_TERMINAL_RESULT ttyr_terminal_updateGrid(
        ttyr_terminal_Grid *Grid_p, void *state_p, nh_gfx_Text *Text_p
    );

    NH_BOOL ttyr_terminal_compareBackgroundAttributes(
        ttyr_tty_Glyph *Glyph1_p, ttyr_tty_Glyph *Glyph2_p
    );
    
    NH_BOOL ttyr_terminal_compareForegroundAttributes(
        ttyr_tty_Glyph *Glyph1_p, ttyr_tty_Glyph *Glyph2_p
    );

    TTYR_TERMINAL_RESULT ttyr_terminal_updateTile(
        ttyr_terminal_Grid *Grid_p, void *state_p, ttyr_terminal_TileUpdate *Update_p, NH_BOOL *update_p
    );

    TTYR_TERMINAL_RESULT ttyr_terminal_updateBoxes(
        ttyr_terminal_Grid *Grid_p, void *state_p, nh_Array *Boxes_p
    );

/** @} */

#endif 
