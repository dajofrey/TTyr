#ifndef TTYR_TERMINAL_TERMINAL_VERTICES_H
#define TTYR_TERMINAL_TERMINAL_VERTICES_H

#include "Grid.h"
#include "Graphics.h"

TTYR_TERMINAL_RESULT ttyr_terminal_getBackgroundVertices(
    ttyr_terminal_GraphicsState *State_p, ttyr_terminal_Grid *Grid_p, ttyr_core_Glyph *Glyph_p, int col,
    int row, float vertices_p[12], int colOffset, int rowOffset, int fontSize
);

TTYR_TERMINAL_RESULT ttyr_terminal_getBoxVertices(
    ttyr_terminal_GraphicsState *State_p, ttyr_terminal_Grid *Grid_p, ttyr_terminal_Box *Box_p,
    bool inner, int fontSize
);

TTYR_TERMINAL_RESULT ttyr_terminal_getOutlineVertices(
    ttyr_terminal_GraphicsState *State_p, ttyr_terminal_Grid *Grid_p, ttyr_terminal_Box *Box_p,
    bool inner, int fontSize
);

TTYR_TERMINAL_RESULT ttyr_terminal_getForegroundVertices(
    ttyr_terminal_GraphicsState *State_p, ttyr_terminal_Grid *Grid_p, ttyr_core_Glyph *Glyph_p, int col,
    int row, float *vertices_p, int fontSize
);

#endif // TTYR_TERMINAL_TERMINAL_VERTICES_H 
