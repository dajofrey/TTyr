#ifndef TTYR_TERMINAL_GRID_H
#define TTYR_TERMINAL_GRID_H

#include "../Common/Includes.h"
#include "../Common/Config.h"
#include "nh-core/Util/Time.h"
#include "nh-gfx/Fonts/Text.h"

typedef struct ttyr_terminal_Box {
    bool accent;
    nh_api_PixelPosition UpperLeft;
    nh_api_PixelPosition LowerRight;
    float innerVertices_p[18];
    float outerVertices_p[18];
} ttyr_terminal_Box;

typedef struct ttyr_terminal_TileUpdate {
    ttyr_core_Glyph Glyph;
    int row;
    int col;
    bool cursor;
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
    ttyr_core_Glyph Glyph;
    bool dirty;
} ttyr_terminal_Tile;

typedef struct ttyr_terminal_Grid {
    nh_core_List Rows;
    nh_core_Array Boxes;
    nh_api_PixelSize TileSize;
    nh_api_PixelSize Size;
    int cols;
    int rows;
    int xOffset, yOffset;
    ttyr_terminal_Tile *Cursor_p;
    ttyr_terminal_TileUpdate **Updates_pp;
    bool **updates_pp;
    int borderPixel;
} ttyr_terminal_Grid;

ttyr_terminal_Tile *ttyr_terminal_getTile(
    ttyr_terminal_Grid *Grid_p, int row, int col
);

TTYR_TERMINAL_RESULT ttyr_terminal_initGrid(
    ttyr_terminal_Grid *Grid_p
);

TTYR_TERMINAL_RESULT ttyr_terminal_freeGrid(
    ttyr_terminal_Grid *Grid_p
);

TTYR_TERMINAL_RESULT ttyr_terminal_updateBorderGrid(
    ttyr_terminal_Config *Config_p, ttyr_terminal_Grid *Grid_p, void *state_p, nh_gfx_Text *Text_p
);

TTYR_TERMINAL_RESULT ttyr_terminal_updateGrid(
    ttyr_terminal_Config *Config_p, ttyr_terminal_Grid *Grid_p, void *state_p, nh_gfx_Text *Text_p
);

bool ttyr_terminal_compareBackgroundAttributes(
    ttyr_core_Glyph *Glyph1_p, ttyr_core_Glyph *Glyph2_p
);

bool ttyr_terminal_compareForegroundAttributes(
    ttyr_core_Glyph *Glyph1_p, ttyr_core_Glyph *Glyph2_p
);

TTYR_TERMINAL_RESULT ttyr_terminal_updateTile(
    ttyr_terminal_Grid *Grid_p, void *state_p, ttyr_terminal_TileUpdate *Update_p, bool *update_p, int fontSize
);

TTYR_TERMINAL_RESULT ttyr_terminal_updateBoxes(
    ttyr_terminal_Grid *Grid_p, void *state_p, nh_core_Array *Boxes_p, int fontSize
);

#endif 
