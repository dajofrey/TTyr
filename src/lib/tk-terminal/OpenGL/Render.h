#ifndef TTYR_TERMINAL_OPENGL_RENDER_H
#define TTYR_TERMINAL_OPENGL_RENDER_H

#include "../Terminal/Graphics.h"
#include "../Common/Includes.h"

TTYR_TERMINAL_RESULT tk_terminal_renderUsingOpenGL(
    tk_terminal_Config *Config_p, tk_terminal_Graphics *Graphics_p, tk_terminal_Grid *Grid_p,
    tk_terminal_Grid *Grid2_p, tk_terminal_Grid *BorderGrid_p
);

#endif
