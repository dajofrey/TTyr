#ifndef TTYR_TERMINAL_OPENGL_RENDER_H
#define TTYR_TERMINAL_OPENGL_RENDER_H

#include "../Terminal/Graphics.h"
#include "../Common/Includes.h"

TTYR_TERMINAL_RESULT ttyr_terminal_renderUsingOpenGL(
    ttyr_terminal_Config *Config_p, ttyr_terminal_Graphics *Graphics_p, ttyr_terminal_Grid *Grid_p,
    ttyr_terminal_Grid *Grid2_p, ttyr_terminal_Grid *BorderGrid_p
);

#endif
