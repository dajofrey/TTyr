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

ttyr_core_Color ttyr_terminal_getGlyphColor(
    ttyr_terminal_Config *Config_p, ttyr_terminal_GraphicsState *State_p, ttyr_core_Glyph *Glyph_p,
    bool foreground, int col, int row, ttyr_terminal_Grid *Grid_p
);

ttyr_core_Color ttyr_terminal_getGradientColor(
    ttyr_terminal_GraphicsGradient *Gradient_p, ttyr_core_Color *Colors_p, int colors
);

#endif 
