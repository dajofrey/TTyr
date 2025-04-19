// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Render.h"

#include "../Common/Macros.h"
#include "../Common/Config.h"

#include "nh-gfx/Base/Viewport.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// FUNCTIONS =======================================================================================

float get_color_component(int col, int row, float time, int total_cols, int total_rows) {
	float wave_length = 100.0f; // How many tiles one full wave spans (same in both axes)

	// Convert to frequency (2π / wavelength)
	float freq_x = (2.0f * M_PI) / wave_length;

	// Adjust Y frequency to match physical scale
	float aspect_ratio = (float)total_rows / (float)total_cols;
	float freq_y = freq_x * ((float)total_cols / (float)total_rows); // or freq_x / aspect_ratio

	float speed = 10.0f;

	float t = (sinf(col * freq_x + row * freq_y + time * speed) + 1.0f) / 2.0f;
	return t;
}

#include <math.h>


#include <math.h>

// Pseudo-random function based on col, row and time
float randf(int x, int y, float time) {
    return fmodf(sinf(x * 12.9898f + y * 78.233f + time * 43758.5453f) * 43758.5453f, 1.0f);
}

ttyr_core_Color get_strobe_color(int col, int row, float time, ttyr_core_Color base) {
    float flicker = randf(col, row, floorf(time * 15.0f));  // Change every 1/15th second
    ttyr_core_Color random_color;
    float min_brightness = 0.5f;
    random_color.r = min_brightness + randf(col, row, time + 0.1f) * (1.0f - min_brightness);
    random_color.g = min_brightness + randf(col, row, time + 0.5f) * (1.0f - min_brightness);
    random_color.b = min_brightness + randf(col, row, time + 1.0f) * (1.0f - min_brightness);
    return random_color;
}

ttyr_core_Color get_wave_color(int col, int row, float time, int total_cols, int total_rows, ttyr_core_Color base) {
    float wave_length = 200.0f;
    float freq_x = (2.0f * M_PI) / wave_length;
    float aspect_ratio = (float)total_rows / (float)total_cols;
    float freq_y = freq_x / aspect_ratio;
    float speed = 5.0f;

    float t = (sinf(col * freq_x + row * freq_y + time * speed) + 1.0f) / 2.0f;

    // Inverted color
    ttyr_core_Color inverted;
    inverted.r = 1.0f - base.r;
    inverted.g = 1.0f - base.g;
    inverted.b = 1.0f - base.b;

    // Interpolate between base and inverted
    ttyr_core_Color result;
    result.r = base.r + (inverted.r - base.r) * t;
    result.g = base.g + (inverted.g - base.g) * t;
    result.b = base.b + (inverted.b - base.b) * t;

    return result;
}

ttyr_core_Color get_fun_color(int col, int row, int total_cols, int total_rows, ttyr_core_Color base)
{
    ttyr_terminal_Config Config = ttyr_terminal_getConfig();
//    float time_now = (float)clock() / CLOCKS_PER_SEC;
float time_now = 0;
    switch (Config.style) {
        case 1 : return get_wave_color(col, row, time_now, total_cols, total_rows, base);
        case 2 : return get_strobe_color(col, row, time_now, base);
    }
    return base;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_drawOpenGLBackground(
    ttyr_terminal_GraphicsState *State_p, ttyr_terminal_GraphicsData *Data_p, ttyr_terminal_Grid *Grid_p)
{
    nh_gfx_OpenGLCommandBuffer *CommandBuffer_p = State_p->Viewport_p->OpenGL.CommandBuffer_p;
    int offset = 0;
    for (int i = 0; i < Data_p->Background.Ranges.length; ++i) {
        ttyr_terminal_AttributeRange *Range_p = ((ttyr_terminal_AttributeRange*)Data_p->Background.Ranges.p)+i;
        ttyr_core_Color Color = ttyr_terminal_getGlyphColor(State_p, &Range_p->Glyph, false);
        if (Range_p->Glyph.mark & TTYR_CORE_MARK_ACCENT) {
            for (int j = 0, k = 0; j < Range_p->indices; j+=6, k++) {
                ttyr_core_Color NewColor = get_fun_color(*(((int*)Range_p->Cols.p)+k), *(((int*)Range_p->Rows.p)+k), Grid_p->cols, Grid_p->rows, Color);
                nh_gfx_addOpenGLCommand(CommandBuffer_p, "glUniform3f", &Data_p->Background.OpenGL.GetUniformLocation_p->Result, 
                    nh_gfx_glfloat(NULL, NewColor.r), nh_gfx_glfloat(NULL, NewColor.g), nh_gfx_glfloat(NULL, NewColor.b));
                nh_gfx_addOpenGLCommand(CommandBuffer_p, "glDrawElements", 
                    nh_gfx_glenum(NULL, GL_TRIANGLES), nh_gfx_glsizei(NULL, 6), 
                    nh_gfx_glenum(NULL, GL_UNSIGNED_INT),
                    nh_gfx_glpointer(NULL, (void*)(sizeof(uint32_t)*(offset+j))));
            }
        } else {
            nh_gfx_addOpenGLCommand(CommandBuffer_p, "glUniform3f", &Data_p->Background.OpenGL.GetUniformLocation_p->Result, 
                nh_gfx_glfloat(NULL, Color.r), nh_gfx_glfloat(NULL, Color.g), nh_gfx_glfloat(NULL, Color.b));
            nh_gfx_addOpenGLCommand(CommandBuffer_p, "glDrawElements", 
                nh_gfx_glenum(NULL, GL_TRIANGLES), nh_gfx_glsizei(NULL, Range_p->indices), 
                nh_gfx_glenum(NULL, GL_UNSIGNED_INT),
                nh_gfx_glpointer(NULL, (void*)(sizeof(uint32_t)*offset)));
        }
        offset += Range_p->indices;
    }

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_prepareOpenGLBackgroundDraw(
    ttyr_terminal_GraphicsState *State_p, ttyr_terminal_GraphicsData *Data_p)
{
    nh_gfx_OpenGLCommandBuffer *CommandBuffer_p = State_p->Viewport_p->OpenGL.CommandBuffer_p;
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glUseProgram", &Data_p->Background.OpenGL.Program_p->Result);

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_drawOpenGLForeground(
    ttyr_terminal_GraphicsState *State_p, ttyr_terminal_GraphicsData *Data_p, ttyr_terminal_Grid *Grid_p)
{
    nh_gfx_OpenGLCommandBuffer *CommandBuffer_p = State_p->Viewport_p->OpenGL.CommandBuffer_p;

    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBindVertexArray", Data_p->Foreground.OpenGL.VertexArray_p);
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glUseProgram", &Data_p->Foreground.OpenGL.Program_p->Result);

    ttyr_terminal_Config Config = ttyr_terminal_getConfig();

    int offset = 0;
    for (int i = 0; i < Data_p->Foreground.Ranges.length; ++i) {
        ttyr_terminal_AttributeRange *Range_p = ((ttyr_terminal_AttributeRange*)Data_p->Foreground.Ranges.p)+i;
        ttyr_core_Color Color = ttyr_terminal_getGlyphColor(State_p, &Range_p->Glyph, true);
        if (Range_p->Glyph.mark & TTYR_CORE_MARK_ACCENT) {
            for (int j = 0, k = 0; j < Range_p->indices; j+=6, k++) {
                ttyr_core_Color NewColor = get_fun_color(*(((int*)Range_p->Cols.p)+k), *(((int*)Range_p->Rows.p)+k), Grid_p->cols, Grid_p->rows, Color);
                nh_gfx_addOpenGLCommand(CommandBuffer_p, "glUniform3f", &Data_p->Foreground.OpenGL.GetUniformLocationColor_p->Result, 
                    nh_gfx_glfloat(NULL, NewColor.r), nh_gfx_glfloat(NULL, NewColor.g), nh_gfx_glfloat(NULL, NewColor.b));
                nh_gfx_addOpenGLCommand(CommandBuffer_p, "glDrawElements", 
                    nh_gfx_glenum(NULL, GL_TRIANGLES), nh_gfx_glsizei(NULL, 6), 
                    nh_gfx_glenum(NULL, GL_UNSIGNED_INT),
                    nh_gfx_glpointer(NULL, (void*)(sizeof(uint32_t)*(offset+j))));
            }
        } else {
            nh_gfx_addOpenGLCommand(CommandBuffer_p, "glUniform3f", &Data_p->Foreground.OpenGL.GetUniformLocationColor_p->Result, 
                nh_gfx_glfloat(NULL, Color.r), nh_gfx_glfloat(NULL, Color.g), nh_gfx_glfloat(NULL, Color.b));
            nh_gfx_addOpenGLCommand(CommandBuffer_p, "glDrawElements", 
                nh_gfx_glenum(NULL, GL_TRIANGLES), nh_gfx_glsizei(NULL, Range_p->indices), 
                nh_gfx_glenum(NULL, GL_UNSIGNED_INT),
                nh_gfx_glpointer(NULL, (void*)(sizeof(uint32_t)*offset)));
        }
        offset += Range_p->indices;
    }

    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glUseProgram", &Data_p->Foreground.OpenGL.Program2_p->Result);
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBindVertexArray", Data_p->Foreground.OpenGL.VertexArray2_p);

    offset = 0;
    for (int i = 0; i < Data_p->Foreground.Ranges2.length; ++i) {
        ttyr_terminal_AttributeRange *Range_p = ((ttyr_terminal_AttributeRange*)Data_p->Foreground.Ranges2.p)+i;
        ttyr_core_Color Color = ttyr_terminal_getGlyphColor(State_p, &Range_p->Glyph, true);
        if (Range_p->Glyph.mark & TTYR_CORE_MARK_ACCENT) {
            for (int j = 0, k = 0; j < Range_p->indices; j+=12, k++) {
                ttyr_core_Color NewColor = get_fun_color(*(((int*)Range_p->Cols.p)+k), *(((int*)Range_p->Rows.p)+k), Grid_p->cols, Grid_p->rows, Color);
                nh_gfx_addOpenGLCommand(CommandBuffer_p, "glUniform3f", &Data_p->Foreground.OpenGL.GetUniformLocationColor2_p->Result, 
                    nh_gfx_glfloat(NULL, NewColor.r), nh_gfx_glfloat(NULL, NewColor.g), nh_gfx_glfloat(NULL, NewColor.b));
                nh_gfx_addOpenGLCommand(CommandBuffer_p, "glDrawElements", 
                    nh_gfx_glenum(NULL, GL_TRIANGLES), nh_gfx_glsizei(NULL, 12), 
                    nh_gfx_glenum(NULL, GL_UNSIGNED_INT),
                    nh_gfx_glpointer(NULL, (void*)(sizeof(uint32_t)*(offset+j))));
            }
        } else {
            nh_gfx_addOpenGLCommand(CommandBuffer_p, "glUniform3f", &Data_p->Foreground.OpenGL.GetUniformLocationColor2_p->Result, 
                nh_gfx_glfloat(NULL, Color.r), nh_gfx_glfloat(NULL, Color.g), nh_gfx_glfloat(NULL, Color.b));
            nh_gfx_addOpenGLCommand(CommandBuffer_p, "glDrawElements", 
                nh_gfx_glenum(NULL, GL_TRIANGLES), nh_gfx_glsizei(NULL, Range_p->indices), 
                nh_gfx_glenum(NULL, GL_UNSIGNED_INT),
                nh_gfx_glpointer(NULL, (void*)(sizeof(uint32_t)*offset)));
        }
        offset += Range_p->indices;
    }

return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_prepareOpenGLForegroundDraw(
    ttyr_terminal_GraphicsState *State_p, ttyr_terminal_GraphicsData *Data_p)
{
    nh_gfx_OpenGLCommandBuffer *CommandBuffer_p = State_p->Viewport_p->OpenGL.CommandBuffer_p;

    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glUseProgram", &Data_p->Foreground.OpenGL.Program_p->Result);
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glUniform1i", &Data_p->Foreground.OpenGL.GetUniformLocationTexture_p->Result, nh_gfx_glint(NULL, 0));

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_drawOpenGLBoxes(
    ttyr_terminal_GraphicsState *State_p, ttyr_terminal_GraphicsData *Data_p, ttyr_terminal_Grid *Grid_p)
{
    ttyr_terminal_Config Config = ttyr_terminal_getConfig();

    nh_gfx_OpenGLCommandBuffer *CommandBuffer_p = State_p->Viewport_p->OpenGL.CommandBuffer_p;

    int offset = 0;
    for (int i = 0; i < Grid_p->Boxes.length; ++i) {
        ttyr_terminal_Box *Box_p = ((ttyr_terminal_Box*)Grid_p->Boxes.p)+i;
        // Render inner box.
        nh_gfx_addOpenGLCommand(CommandBuffer_p, "glUniform3f", 
           &Data_p->Background.OpenGL.GetUniformLocation_p->Result, 
           nh_gfx_glfloat(NULL, State_p->BackgroundGradient.Color.r), 
           nh_gfx_glfloat(NULL, State_p->BackgroundGradient.Color.g), 
           nh_gfx_glfloat(NULL, State_p->BackgroundGradient.Color.b));
        nh_gfx_addOpenGLCommand(CommandBuffer_p, "glDrawArrays", 
            nh_gfx_glenum(NULL, GL_TRIANGLES), nh_gfx_glint(NULL, offset), nh_gfx_glsizei(NULL, 6));
        offset += 6;
        // Render outer box.
        nh_gfx_addOpenGLCommand(CommandBuffer_p, "glUniform3f", 
           &Data_p->Background.OpenGL.GetUniformLocation_p->Result, 
           nh_gfx_glfloat(NULL, Box_p->accent ? State_p->AccentGradient.Color.r : Config.Foreground.r), 
           nh_gfx_glfloat(NULL, Box_p->accent ? State_p->AccentGradient.Color.g : Config.Foreground.g), 
           nh_gfx_glfloat(NULL, Box_p->accent ? State_p->AccentGradient.Color.b : Config.Foreground.b));
        nh_gfx_addOpenGLCommand(CommandBuffer_p, "glDrawArrays", 
            nh_gfx_glenum(NULL, GL_TRIANGLES), nh_gfx_glint(NULL, offset), nh_gfx_glsizei(NULL, 6));
        offset += 6;
    }

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_prepareOpenGLBoxesDraw(
    ttyr_terminal_GraphicsState *State_p, ttyr_terminal_GraphicsData *Data_p)
{
    nh_gfx_OpenGLCommandBuffer *CommandBuffer_p = State_p->Viewport_p->OpenGL.CommandBuffer_p;
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glUseProgram", &Data_p->Background.OpenGL.Program_p->Result);

    return TTYR_TERMINAL_SUCCESS;
}

TTYR_TERMINAL_RESULT ttyr_terminal_renderUsingOpenGL(
    ttyr_terminal_Graphics *Graphics_p, ttyr_terminal_Grid *Grid_p, ttyr_terminal_Grid *Grid2_p, ttyr_terminal_Grid *BorderGrid_p)
{
    bool blockUntilRender = Graphics_p->Data1.Background.Action.init || Graphics_p->Data1.Foreground.Action.init;
    ttyr_terminal_Config Config = ttyr_terminal_getConfig();

    nh_gfx_beginRecording(Graphics_p->State.Viewport_p);

 // first render border grid

   nh_gfx_addOpenGLCommand(
       Graphics_p->State.Viewport_p->OpenGL.CommandBuffer_p,
       "glViewport",
       nh_gfx_glint(NULL, -BorderGrid_p->xOffset),
       nh_gfx_glint(NULL, -BorderGrid_p->yOffset),
       nh_gfx_glsizei(NULL, BorderGrid_p->Size.width),
       nh_gfx_glsizei(NULL, BorderGrid_p->Size.height));

    TTYR_TERMINAL_CHECK(ttyr_terminal_updateOpenGLBackground(&Graphics_p->State, &Graphics_p->BorderData))
    TTYR_TERMINAL_CHECK(ttyr_terminal_prepareOpenGLBackgroundDraw(&Graphics_p->State, &Graphics_p->BorderData))
    TTYR_TERMINAL_CHECK(ttyr_terminal_drawOpenGLBackground(&Graphics_p->State, &Graphics_p->BorderData, BorderGrid_p))

// clear except borders

    nh_gfx_addOpenGLCommand(
        Graphics_p->State.Viewport_p->OpenGL.CommandBuffer_p,
        "glViewport",
        nh_gfx_glint(NULL, 0),
        nh_gfx_glint(NULL, 0),
        nh_gfx_glsizei(NULL, Grid_p->Size.width+Config.border*2),
        nh_gfx_glsizei(NULL, Grid_p->Size.height+Config.border*2));
 
    nh_gfx_addOpenGLCommand(
        Graphics_p->State.Viewport_p->OpenGL.CommandBuffer_p,
        "glScissor",
        nh_gfx_glint(NULL, Config.border),
        nh_gfx_glint(NULL, Config.border),
        nh_gfx_glsizei(NULL, Grid_p->Size.width),
        nh_gfx_glsizei(NULL, Grid_p->Size.height));
    
    nh_gfx_addOpenGLCommand(
        Graphics_p->State.Viewport_p->OpenGL.CommandBuffer_p, 
        "glClearColor",
        nh_gfx_glfloat(NULL, Graphics_p->State.Viewport_p->Settings.ClearColor.r),
        nh_gfx_glfloat(NULL, Graphics_p->State.Viewport_p->Settings.ClearColor.g),
        nh_gfx_glfloat(NULL, Graphics_p->State.Viewport_p->Settings.ClearColor.b),
        nh_gfx_glfloat(NULL, Graphics_p->State.Viewport_p->Settings.ClearColor.a));

    nh_gfx_addOpenGLCommand(
        Graphics_p->State.Viewport_p->OpenGL.CommandBuffer_p, 
        "glClear", 
        nh_gfx_glenum(NULL, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

// second render terminal

    nh_gfx_addOpenGLCommand(
        Graphics_p->State.Viewport_p->OpenGL.CommandBuffer_p,
        "glViewport",
        nh_gfx_glint(NULL, Config.border),
        nh_gfx_glint(NULL, Config.border),
        nh_gfx_glsizei(NULL, Grid_p->Size.width),
        nh_gfx_glsizei(NULL, Grid_p->Size.height));
 
    nh_gfx_OpenGLCommandBuffer *CommandBuffer_p = Graphics_p->State.Viewport_p->OpenGL.CommandBuffer_p;

    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glEnable", nh_gfx_glenum(NULL, GL_BLEND));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glEnable", nh_gfx_glenum(NULL, GL_DEPTH_TEST));
 
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBlendFunc", nh_gfx_glenum(NULL, GL_SRC_ALPHA),
        nh_gfx_glenum(NULL, GL_ONE_MINUS_SRC_ALPHA));

    TTYR_TERMINAL_CHECK(ttyr_terminal_updateOpenGLBackground(&Graphics_p->State, &Graphics_p->Data1))
    TTYR_TERMINAL_CHECK(ttyr_terminal_prepareOpenGLBackgroundDraw(&Graphics_p->State, &Graphics_p->Data1))
    TTYR_TERMINAL_CHECK(ttyr_terminal_drawOpenGLBackground(&Graphics_p->State, &Graphics_p->Data1, Grid_p))

    TTYR_TERMINAL_CHECK(ttyr_terminal_updateOpenGLBoxes(&Graphics_p->State, &Graphics_p->Data1))
    TTYR_TERMINAL_CHECK(ttyr_terminal_prepareOpenGLBoxesDraw(&Graphics_p->State, &Graphics_p->Data1))
    TTYR_TERMINAL_CHECK(ttyr_terminal_drawOpenGLBoxes(&Graphics_p->State, &Graphics_p->Data1, Grid_p))

    TTYR_TERMINAL_CHECK(ttyr_terminal_updateOpenGLForeground(&Graphics_p->State, &Graphics_p->Data1))
    TTYR_TERMINAL_CHECK(ttyr_terminal_prepareOpenGLForegroundDraw(&Graphics_p->State, &Graphics_p->Data1))
    TTYR_TERMINAL_CHECK(ttyr_terminal_drawOpenGLForeground(&Graphics_p->State, &Graphics_p->Data1, Grid_p))

    TTYR_TERMINAL_CHECK(ttyr_terminal_updateOpenGLBackground(&Graphics_p->State, &Graphics_p->Data2))
    TTYR_TERMINAL_CHECK(ttyr_terminal_prepareOpenGLBackgroundDraw(&Graphics_p->State, &Graphics_p->Data2))
    TTYR_TERMINAL_CHECK(ttyr_terminal_drawOpenGLBackground(&Graphics_p->State, &Graphics_p->Data2, Grid2_p))

    TTYR_TERMINAL_CHECK(ttyr_terminal_updateOpenGLForeground(&Graphics_p->State, &Graphics_p->Data2))
    TTYR_TERMINAL_CHECK(ttyr_terminal_prepareOpenGLForegroundDraw(&Graphics_p->State, &Graphics_p->Data2))
    TTYR_TERMINAL_CHECK(ttyr_terminal_drawOpenGLForeground(&Graphics_p->State, &Graphics_p->Data2, Grid2_p))
 
    nh_gfx_endRecording(Graphics_p->State.Viewport_p, blockUntilRender);

    return TTYR_TERMINAL_SUCCESS;
}
