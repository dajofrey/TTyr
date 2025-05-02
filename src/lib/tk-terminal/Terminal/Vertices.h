#ifndef TTYR_TERMINAL_TERMINAL_VERTICES_H
#define TTYR_TERMINAL_TERMINAL_VERTICES_H

#include "Grid.h"
#include "Graphics.h"

TTYR_TERMINAL_RESULT tk_terminal_getBackgroundVertices(
    tk_terminal_GraphicsState *State_p, tk_terminal_Grid *Grid_p, tk_core_Glyph *Glyph_p, int col,
    int row, float vertices_p[12], int colOffset, int rowOffset, int fontSize
);

TTYR_TERMINAL_RESULT tk_terminal_getBoxVertices(
    tk_terminal_GraphicsState *State_p, tk_terminal_Grid *Grid_p, tk_terminal_Box *Box_p,
    bool inner, int fontSize
);

TTYR_TERMINAL_RESULT tk_terminal_getOutlineVertices(
    tk_terminal_GraphicsState *State_p, tk_terminal_Grid *Grid_p, tk_terminal_Box *Box_p,
    bool inner, int fontSize
);

TTYR_TERMINAL_RESULT tk_terminal_getForegroundVertices(
    tk_terminal_GraphicsState *State_p, tk_terminal_Grid *Grid_p, tk_core_Glyph *Glyph_p, int col,
    int row, float *vertices_p, int fontSize
);

TTYR_TERMINAL_RESULT tk_terminal_getForegroundVerticesForLineGraphics(
    tk_terminal_GraphicsState *State_p, tk_terminal_Grid *Grid_p, NH_API_UTF32 codepoint, int col,
    int row, float depth, float vertices_p[24], int fontSize, int colOffset, int rowOffset
);

#endif // TTYR_TERMINAL_TERMINAL_VERTICES_H 
