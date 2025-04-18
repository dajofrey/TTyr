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
                float time_now = (float)clock() / CLOCKS_PER_SEC;
                float intensity = get_color_component(*(((int*)Range_p->Cols.p)+k), *(((int*)Range_p->Rows.p)+k), time_now, Grid_p->cols, Grid_p->rows);
                Color.r = intensity;
                Color.g = intensity; 
                Color.b = 0.7;
                nh_gfx_addOpenGLCommand(CommandBuffer_p, "glUniform3f", &Data_p->Foreground.OpenGL.GetUniformLocationColor_p->Result, 
                    nh_gfx_glfloat(NULL, Color.r), nh_gfx_glfloat(NULL, Color.g), nh_gfx_glfloat(NULL, Color.b));
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

    int offset = 0;
    for (int i = 0; i < Data_p->Foreground.Ranges.length; ++i) {
        ttyr_terminal_AttributeRange *Range_p = ((ttyr_terminal_AttributeRange*)Data_p->Foreground.Ranges.p)+i;
        ttyr_core_Color Color = ttyr_terminal_getGlyphColor(State_p, &Range_p->Glyph, true);
//        if (Range_p->Glyph.mark & TTYR_CORE_MARK_ACCENT) {
//            for (int j = 0, k = 0; j < Range_p->indices; j+=6, k++) {
//                float time_now = (float)clock() / CLOCKS_PER_SEC;
//                float intensity = get_color_component(*(((int*)Range_p->Cols.p)+k), *(((int*)Range_p->Rows.p)+k), time_now, Grid_p->cols, Grid_p->rows);
//                Color.r = intensity;
//                Color.g = intensity; 
//                Color.b = 0.7;
//                nh_gfx_addOpenGLCommand(CommandBuffer_p, "glUniform3f", &Data_p->Foreground.OpenGL.GetUniformLocationColor_p->Result, 
//                    nh_gfx_glfloat(NULL, Color.r), nh_gfx_glfloat(NULL, Color.g), nh_gfx_glfloat(NULL, Color.b));
//                nh_gfx_addOpenGLCommand(CommandBuffer_p, "glDrawElements", 
//                    nh_gfx_glenum(NULL, GL_TRIANGLES), nh_gfx_glsizei(NULL, 6), 
//                    nh_gfx_glenum(NULL, GL_UNSIGNED_INT),
//                    nh_gfx_glpointer(NULL, (void*)(sizeof(uint32_t)*(offset+j))));
//            }
//        } else {
            nh_gfx_addOpenGLCommand(CommandBuffer_p, "glUniform3f", &Data_p->Foreground.OpenGL.GetUniformLocationColor_p->Result, 
                nh_gfx_glfloat(NULL, Color.r), nh_gfx_glfloat(NULL, Color.g), nh_gfx_glfloat(NULL, Color.b));
            nh_gfx_addOpenGLCommand(CommandBuffer_p, "glDrawElements", 
                nh_gfx_glenum(NULL, GL_TRIANGLES), nh_gfx_glsizei(NULL, Range_p->indices), 
                nh_gfx_glenum(NULL, GL_UNSIGNED_INT),
                nh_gfx_glpointer(NULL, (void*)(sizeof(uint32_t)*offset)));
//        }
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
                float time_now = (float)clock() / CLOCKS_PER_SEC;
                float intensity = get_color_component(*(((int*)Range_p->Cols.p)+k), *(((int*)Range_p->Rows.p)+k), time_now, Grid_p->cols, Grid_p->rows);
                Color.r = intensity;
                Color.g = intensity; 
                Color.b = 0.7;
                nh_gfx_addOpenGLCommand(CommandBuffer_p, "glUniform3f", &Data_p->Foreground.OpenGL.GetUniformLocationColor2_p->Result, 
                    nh_gfx_glfloat(NULL, Color.r), nh_gfx_glfloat(NULL, Color.g), nh_gfx_glfloat(NULL, Color.b));
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
    ttyr_terminal_Graphics *Graphics_p, ttyr_terminal_Grid *Grid_p, ttyr_terminal_Grid *Grid2_p)
{
    bool blockUntilRender = Graphics_p->Data1.Background.Action.init || Graphics_p->Data1.Foreground.Action.init;

    nh_gfx_beginRecording(Graphics_p->State.Viewport_p);

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
