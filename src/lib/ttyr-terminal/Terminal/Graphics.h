#ifndef TTYR_TERMINAL_TERMINAL_GRAPHICS_H
#define TTYR_TERMINAL_TERMINAL_GRAPHICS_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "Grid.h"

#include "../Vulkan/Text.h"
#include "../OpenGL/Foreground.h"
#include "../OpenGL/Background.h"
#include "../OpenGL/Boxes.h"

#include "nh-core/Util/HashMap.h"
#include "nh-gfx/Base/Viewport.h"

#endif

/** @addtogroup lib_nhterminal_structs
 *  @{
 */

    typedef struct ttyr_terminal_AttributeRange {
        ttyr_tty_Glyph Glyph;
        int indices;
    } ttyr_terminal_AttributeRange;

    typedef struct ttyr_terminal_GraphicsAction {
        bool init;
    } ttyr_terminal_GraphicsAction;

    /**
     * Text that is drawn on top of the background. Can be either normal or elevated,
     * which basically is the same but with different depth values.
     */
    typedef struct ttyr_terminal_GraphicsForeground {
        ttyr_terminal_GraphicsAction Action;
        ttyr_terminal_VulkanText Vulkan;
        ttyr_terminal_OpenGLForeground OpenGL;
        nh_core_Array Vertices;
        nh_core_Array Indices;
        nh_core_Array Vertices2;
        nh_core_Array Indices2;
        nh_core_Array Ranges;
        nh_core_Array Ranges2;
    } ttyr_terminal_GraphicsForeground;

    /**
     * The background behind normal text, boxes and elevated text.
     */
    typedef struct ttyr_terminal_GraphicsBackground {
        ttyr_terminal_GraphicsAction Action;
        ttyr_terminal_OpenGLBackground OpenGL;
        nh_core_Array Vertices;
        nh_core_Array Indices;
        nh_core_Array Ranges;
    } ttyr_terminal_GraphicsBackground;

    typedef struct ttyr_terminal_GraphicsBoxes {
        ttyr_terminal_GraphicsAction Action;
        ttyr_terminal_OpenGLBoxes OpenGL;
        nh_core_Array Vertices;
    } ttyr_terminal_GraphicsBoxes;
 
    typedef struct ttyr_terminal_GraphicsGradient {
        nh_Color Color;
        float ratio;
        int index;
        double interval;
        nh_core_SystemTime LastChange;
    } ttyr_terminal_GraphicsGradient;

    typedef struct ttyr_terminal_GraphicsData {
        ttyr_terminal_GraphicsForeground Foreground;
        ttyr_terminal_GraphicsBackground Background;
        ttyr_terminal_GraphicsBoxes Boxes;
    } ttyr_terminal_GraphicsData;

    /**
     * Blinking state.
     */
    typedef struct ttyr_terminal_GraphicsBlink {
        bool on;              /**<If true, cursor is in visible period.*/
        nh_core_SystemTime LastBlink; /**<System time of last blink change.*/
    } ttyr_terminal_GraphicsBlink;

    typedef struct ttyr_terminal_GraphicsState {
        nh_gfx_Viewport *Viewport_p;
        ttyr_terminal_GraphicsBlink Blink;
        ttyr_terminal_GraphicsGradient Gradient;
        nh_HashMap Map;
        nh_gfx_FontInstance *FontInstance_p;
        nh_core_List Fonts;
        int font;
        nh_core_List Glyphs;
        nh_core_List Codepoints;
    } ttyr_terminal_GraphicsState;

    typedef struct ttyr_terminal_Graphics {
        ttyr_terminal_GraphicsState State;
        ttyr_terminal_GraphicsData Data1;
        ttyr_terminal_GraphicsData Data2;
    } ttyr_terminal_Graphics;

/** @} */

/** @addtogroup lib_nhterminal_functions
 *  @{
 */

    TTYR_TERMINAL_RESULT ttyr_terminal_initGraphics(
        ttyr_terminal_Graphics *Graphics_p
    );

    TTYR_TERMINAL_RESULT ttyr_terminal_freeGraphics(
        ttyr_terminal_Graphics *Graphics_p
    );

    TTYR_TERMINAL_RESULT ttyr_terminal_handleViewportChange(
        ttyr_terminal_Graphics *Graphics_p, nh_gfx_Viewport *Viewport_p
    );

    TTYR_TERMINAL_RESULT ttyr_terminal_updateGraphicsData(
        ttyr_terminal_GraphicsData *Data_p, ttyr_terminal_Grid *Grid_p
    );

    TTYR_TERMINAL_RESULT ttyr_terminal_renderGraphics(
        ttyr_terminal_Graphics *Graphics_p, ttyr_terminal_Grid *Grid_p, ttyr_terminal_Grid *Grid2_p
    );

    nh_Color ttyr_terminal_getGlyphColor(
        ttyr_terminal_GraphicsState *State_p, ttyr_tty_Glyph *Glyph_p, bool foreground
    );

    bool ttyr_terminal_updateBlinkOrGradient(
        ttyr_terminal_GraphicsState *State_p
    );

/** @} */

#endif 
