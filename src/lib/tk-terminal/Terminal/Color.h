#ifndef TTYR_TERMINAL_TERMINAL_COLOR_H
#define TTYR_TERMINAL_TERMINAL_COLOR_H

#include "Grid.h"
#include "Graphics.h"

#include "../Vulkan/Text.h"
#include "../OpenGL/Foreground.h"
#include "../OpenGL/Background.h"
#include "../OpenGL/Boxes.h"

#include "nh-core/Util/HashMap.h"
#include "nh-gfx/Base/Viewport.h"

tk_core_Color tk_terminal_getGlyphColor(
    tk_terminal_Config *Config_p, tk_terminal_GraphicsState *State_p, tk_core_Glyph *Glyph_p,
    bool foreground, int col, int row, tk_terminal_Grid *Grid_p
);

tk_core_Color tk_terminal_getGradientColor(
    tk_terminal_GraphicsGradient *Gradient_p, tk_core_Color *Colors_p, int colors
);

#endif 
