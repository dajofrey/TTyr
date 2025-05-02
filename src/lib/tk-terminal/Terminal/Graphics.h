#ifndef TK_TERMINAL_TERMINAL_GRAPHICS_H
#define TK_TERMINAL_TERMINAL_GRAPHICS_H

// INCLUDES ========================================================================================

#include "Grid.h"

#include "../Vulkan/Text.h"
#include "../OpenGL/Foreground.h"
#include "../OpenGL/Background.h"
#include "../OpenGL/Boxes.h"
#include "../OpenGL/Dim.h"

#include "nh-core/Util/HashMap.h"
#include "nh-gfx/Base/Viewport.h"

// STRUCTS =========================================================================================

typedef struct tk_terminal_AttributeRange {
    tk_core_Glyph Glyph;
    int indices;
} tk_terminal_AttributeRange;

typedef struct tk_terminal_GraphicsAction {
    bool init;
} tk_terminal_GraphicsAction;

typedef struct tk_terminal_Dim {
    tk_terminal_GraphicsAction Action;
    tk_terminal_OpenGLDim OpenGL;
    nh_core_Array Vertices;
    nh_core_Array Colors;
} tk_terminal_Dim;

/**
 * Text that is drawn on top of the background. Can be either normal or elevated,
 * which basically is the same but with different depth values.
 */
typedef struct tk_terminal_GraphicsForeground {
    tk_terminal_GraphicsAction Action;
    tk_terminal_VulkanText Vulkan;
    tk_terminal_OpenGLForeground OpenGL;
    nh_core_Array Vertices;
    nh_core_Array Indices;
    nh_core_Array Vertices2;
    nh_core_Array Indices2;
    nh_core_Array Ranges;
    nh_core_Array Ranges2;
    nh_core_Array Colors;
    nh_core_Array Colors2;
} tk_terminal_GraphicsForeground;

/**
 * The background behind normal text, boxes and elevated text.
 */
typedef struct tk_terminal_GraphicsBackground {
    tk_terminal_GraphicsAction Action;
    tk_terminal_OpenGLBackground OpenGL;
    nh_core_Array Vertices;
    nh_core_Array Indices;
    nh_core_Array Ranges;
    nh_core_Array Colors;
} tk_terminal_GraphicsBackground;

typedef struct tk_terminal_Boxes {
    tk_terminal_GraphicsAction Action;
    tk_terminal_OpenGLBoxes OpenGL;
    nh_core_Array Vertices;
    nh_core_Array Colors;
    nh_core_Array Data;
} tk_terminal_Boxes;

typedef struct tk_terminal_GraphicsGradient {
    tk_core_Color Color;
    float ratio;
    int index;
    double interval;
    nh_core_SystemTime LastChange;
} tk_terminal_GraphicsGradient;

typedef struct tk_terminal_GraphicsData {
    tk_terminal_GraphicsForeground Foreground;
    tk_terminal_GraphicsBackground Background;
} tk_terminal_GraphicsData;

/**
 * Blinking state.
 */
typedef struct tk_terminal_GraphicsBlink {
    bool on;              /**<If true, cursor is in visible period.*/
    nh_core_SystemTime LastBlink; /**<System time of last blink change.*/
} tk_terminal_GraphicsBlink;

typedef struct tk_terminal_GraphicsState {
    nh_gfx_Viewport *Viewport_p;
    tk_terminal_GraphicsBlink Blink;
    tk_terminal_GraphicsGradient AccentGradient;
    tk_terminal_GraphicsGradient BackgroundGradient;
    nh_core_HashMap Map;
    nh_gfx_FontInstance *FontInstance_p;
    nh_core_List Fonts;
    int font;
    nh_core_List Glyphs;
    nh_core_List Codepoints;
} tk_terminal_GraphicsState;

typedef struct tk_terminal_Graphics {
    tk_terminal_GraphicsState State;
    tk_terminal_GraphicsData MainData;
    tk_terminal_GraphicsData ElevatedData;
    tk_terminal_GraphicsData BackdropData;
    tk_terminal_Dim Dim;
    tk_terminal_Boxes Boxes;
} tk_terminal_Graphics;

// FUNCTIONS =======================================================================================

TK_TERMINAL_RESULT tk_terminal_initGraphics(
    tk_terminal_Config *Config_p, tk_terminal_Graphics *Graphics_p
);

TK_TERMINAL_RESULT tk_terminal_freeGraphics(
    tk_terminal_Graphics *Graphics_p
);

TK_TERMINAL_RESULT tk_terminal_handleViewportChange(
    tk_terminal_Graphics *Graphics_p, nh_gfx_Viewport *Viewport_p
);

TK_TERMINAL_RESULT tk_terminal_updateGraphics( 
    tk_terminal_Config *Config_p, tk_terminal_Graphics *Graphics_p, tk_terminal_Grid *Grid_p,
    tk_terminal_Grid *BackdropGrid_p, tk_terminal_Grid *ElevatedGrid_p, bool titlebarOn
); 

TK_TERMINAL_RESULT tk_terminal_renderGraphics(
    tk_terminal_Config *Config_p, tk_terminal_Graphics *Graphics_p, tk_terminal_Grid *Grid_p,
    tk_terminal_Grid *ElevatedGrid_p, tk_terminal_Grid *BackdropGrid_p
);

bool tk_terminal_updateBlinkOrGradient(
    tk_terminal_Config *Config_p, tk_terminal_GraphicsState *State_p
);

#endif 
