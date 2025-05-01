#ifndef TTYR_TERMINAL_GRID_H
#define TTYR_TERMINAL_GRID_H

#include "../Common/Includes.h"
#include "../Common/Config.h"
#include "nh-core/Util/Time.h"
#include "nh-gfx/Fonts/Text.h"

typedef struct tk_terminal_Box {
    bool accent;
    nh_api_PixelPosition UpperLeft;
    nh_api_PixelPosition LowerRight;
    float innerVertices_p[18];
    float outerVertices_p[18];
} tk_terminal_Box;

typedef struct tk_terminal_TileUpdate {
    tk_core_Glyph Glyph;
    int row;
    int col;
    bool cursor;
} tk_terminal_TileUpdate;

typedef struct tk_terminal_TileForeground {
    float vertices_p[24];
} tk_terminal_TileForeground;

typedef struct tk_terminal_TileBackground {
    float vertices_p[12];
} tk_terminal_TileBackground;

typedef struct tk_terminal_Tile {
    tk_terminal_TileForeground Foreground;
    tk_terminal_TileBackground Background;
    tk_core_Glyph Glyph;
    bool dirty;
} tk_terminal_Tile;

typedef struct tk_terminal_Grid {
    nh_core_List Rows;
    nh_core_Array Boxes;
    nh_api_PixelSize TileSize;
    nh_api_PixelSize Size;
    int cols;
    int rows;
    int xOffset, yOffset;
    tk_terminal_Tile *Cursor_p;
    tk_terminal_TileUpdate **Updates_pp;
    bool **updates_pp;
    int borderPixel;
} tk_terminal_Grid;

tk_terminal_Tile *tk_terminal_getTile(
    tk_terminal_Grid *Grid_p, int row, int col
);

TTYR_TERMINAL_RESULT tk_terminal_initGrid(
    tk_terminal_Grid *Grid_p
);

TTYR_TERMINAL_RESULT tk_terminal_freeGrid(
    tk_terminal_Grid *Grid_p
);

TTYR_TERMINAL_RESULT tk_terminal_updateBorderGrid(
    tk_terminal_Config *Config_p, tk_terminal_Grid *Grid_p, void *state_p, nh_gfx_Text *Text_p
);

TTYR_TERMINAL_RESULT tk_terminal_updateGrid(
    tk_terminal_Config *Config_p, tk_terminal_Grid *Grid_p, void *state_p, nh_gfx_Text *Text_p
);

bool tk_terminal_compareBackgroundAttributes(
    tk_core_Glyph *Glyph1_p, tk_core_Glyph *Glyph2_p
);

bool tk_terminal_compareForegroundAttributes(
    tk_core_Glyph *Glyph1_p, tk_core_Glyph *Glyph2_p
);

TTYR_TERMINAL_RESULT tk_terminal_updateTile(
    tk_terminal_Grid *Grid_p, void *state_p, tk_terminal_TileUpdate *Update_p, bool *update_p, int fontSize
);

TTYR_TERMINAL_RESULT tk_terminal_updateBoxes(
    tk_terminal_Grid *Grid_p, void *state_p, nh_core_Array *Boxes_p, int fontSize
);

#endif 
