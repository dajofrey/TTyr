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

#include "../../../../external/Netzhaut/src/lib/nhgfx/Base/Viewport.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// BACKGROUND ======================================================================================

static TTYR_TERMINAL_RESULT ttyr_terminal_drawOpenGLBackground(
    ttyr_terminal_GraphicsState *State_p, ttyr_terminal_GraphicsData *Data_p, ttyr_terminal_Grid *Grid_p)
{
TTYR_TERMINAL_BEGIN()

    nh_opengl_CommandBuffer *CommandBuffer_p = State_p->Viewport_p->OpenGL.CommandBuffer_p;

    int offset = 0;
    for (int i = 0; i < Data_p->Background.Ranges.length; ++i) {
        ttyr_terminal_AttributeRange *Range_p = ((ttyr_terminal_AttributeRange*)Data_p->Background.Ranges.p)+i;
        nh_Color Color = ttyr_terminal_getGlyphColor(State_p, &Range_p->Glyph, NH_FALSE);
        nh_opengl_addCommand(CommandBuffer_p, "glUniform3f", &Data_p->Background.OpenGL.GetUniformLocation_p->Result, 
            nh_opengl_glfloat(NULL, Color.r), nh_opengl_glfloat(NULL, Color.g), nh_opengl_glfloat(NULL, Color.b));
        nh_opengl_addCommand(CommandBuffer_p, "glDrawElements", 
            nh_opengl_glenum(NULL, GL_TRIANGLES), nh_opengl_glsizei(NULL, Range_p->indices), 
            nh_opengl_glenum(NULL, GL_UNSIGNED_INT),
            nh_opengl_pointer(NULL, (void*)(sizeof(uint32_t)*offset)));
        offset += Range_p->indices;
    }

TTYR_TERMINAL_DIAGNOSTIC_END(TTYR_TERMINAL_SUCCESS)
}

static TTYR_TERMINAL_RESULT ttyr_terminal_prepareOpenGLBackgroundDraw(
    ttyr_terminal_GraphicsState *State_p, ttyr_terminal_GraphicsData *Data_p)
{
TTYR_TERMINAL_BEGIN()

    nh_opengl_CommandBuffer *CommandBuffer_p = State_p->Viewport_p->OpenGL.CommandBuffer_p;
    nh_opengl_addCommand(CommandBuffer_p, "glUseProgram", &Data_p->Background.OpenGL.Program_p->Result);

TTYR_TERMINAL_DIAGNOSTIC_END(TTYR_TERMINAL_SUCCESS)
}

// FOREGROUND ======================================================================================

static TTYR_TERMINAL_RESULT ttyr_terminal_drawOpenGLForeground(
    ttyr_terminal_GraphicsState *State_p, ttyr_terminal_GraphicsData *Data_p, ttyr_terminal_Grid *Grid_p)
{
TTYR_TERMINAL_BEGIN()

    nh_opengl_CommandBuffer *CommandBuffer_p = State_p->Viewport_p->OpenGL.CommandBuffer_p;

    nh_opengl_addCommand(CommandBuffer_p, "glBindVertexArray", Data_p->Foreground.OpenGL.VertexArray_p);
    nh_opengl_addCommand(CommandBuffer_p, "glUseProgram", &Data_p->Foreground.OpenGL.Program_p->Result);

    int offset = 0;
    for (int i = 0; i < Data_p->Foreground.Ranges.length; ++i) {
        ttyr_terminal_AttributeRange *Range_p = ((ttyr_terminal_AttributeRange*)Data_p->Foreground.Ranges.p)+i;
        nh_Color Color = ttyr_terminal_getGlyphColor(State_p, &Range_p->Glyph, NH_TRUE);
        nh_opengl_addCommand(CommandBuffer_p, "glUniform3f", &Data_p->Foreground.OpenGL.GetUniformLocationColor_p->Result, 
            nh_opengl_glfloat(NULL, Color.r), nh_opengl_glfloat(NULL, Color.g), nh_opengl_glfloat(NULL, Color.b));
        nh_opengl_addCommand(CommandBuffer_p, "glDrawElements", 
            nh_opengl_glenum(NULL, GL_TRIANGLES), nh_opengl_glsizei(NULL, Range_p->indices), 
            nh_opengl_glenum(NULL, GL_UNSIGNED_INT),
            nh_opengl_pointer(NULL, (void*)(sizeof(uint32_t)*offset)));
        offset += Range_p->indices;
    }

    nh_opengl_addCommand(CommandBuffer_p, "glUseProgram", &Data_p->Foreground.OpenGL.Program2_p->Result);
    nh_opengl_addCommand(CommandBuffer_p, "glBindVertexArray", Data_p->Foreground.OpenGL.VertexArray2_p);

    offset = 0;
    for (int i = 0; i < Data_p->Foreground.Ranges2.length; ++i) {
        ttyr_terminal_AttributeRange *Range_p = ((ttyr_terminal_AttributeRange*)Data_p->Foreground.Ranges2.p)+i;
        nh_Color Color = ttyr_terminal_getGlyphColor(State_p, &Range_p->Glyph, NH_TRUE);
        nh_opengl_addCommand(CommandBuffer_p, "glUniform3f", &Data_p->Foreground.OpenGL.GetUniformLocationColor2_p->Result, 
            nh_opengl_glfloat(NULL, Color.r), nh_opengl_glfloat(NULL, Color.g), nh_opengl_glfloat(NULL, Color.b));
        nh_opengl_addCommand(CommandBuffer_p, "glDrawElements", 
            nh_opengl_glenum(NULL, GL_TRIANGLES), nh_opengl_glsizei(NULL, Range_p->indices), 
            nh_opengl_glenum(NULL, GL_UNSIGNED_INT),
            nh_opengl_pointer(NULL, (void*)(sizeof(uint32_t)*offset)));
        offset += Range_p->indices;
    }

TTYR_TERMINAL_DIAGNOSTIC_END(TTYR_TERMINAL_SUCCESS)
}

static TTYR_TERMINAL_RESULT ttyr_terminal_prepareOpenGLForegroundDraw(
    ttyr_terminal_GraphicsState *State_p, ttyr_terminal_GraphicsData *Data_p)
{
TTYR_TERMINAL_BEGIN()

    nh_opengl_CommandBuffer *CommandBuffer_p = State_p->Viewport_p->OpenGL.CommandBuffer_p;

    nh_opengl_addCommand(CommandBuffer_p, "glUseProgram", &Data_p->Foreground.OpenGL.Program_p->Result);
    nh_opengl_addCommand(CommandBuffer_p, "glUniform1i", &Data_p->Foreground.OpenGL.GetUniformLocationTexture_p->Result, nh_opengl_glint(NULL, 0));

TTYR_TERMINAL_DIAGNOSTIC_END(TTYR_TERMINAL_SUCCESS)
}

// BOXES ===========================================================================================

static TTYR_TERMINAL_RESULT ttyr_terminal_drawOpenGLBoxes(
    ttyr_terminal_GraphicsState *State_p, ttyr_terminal_GraphicsData *Data_p, ttyr_terminal_Grid *Grid_p)
{
TTYR_TERMINAL_BEGIN()

    ttyr_terminal_Config Config = ttyr_terminal_getConfig();

    nh_opengl_CommandBuffer *CommandBuffer_p = State_p->Viewport_p->OpenGL.CommandBuffer_p;

    int offset = 0;
    for (int i = 0; i < Grid_p->Boxes.length; ++i) {
        ttyr_terminal_Box *Box_p = ((ttyr_terminal_Box*)Grid_p->Boxes.p)+i;
        // Render inner box.
        nh_opengl_addCommand(CommandBuffer_p, "glUniform3f", 
           &Data_p->Background.OpenGL.GetUniformLocation_p->Result, 
           nh_opengl_glfloat(NULL, Config.Background.r), 
           nh_opengl_glfloat(NULL, Config.Background.g), 
           nh_opengl_glfloat(NULL, Config.Background.b));
        nh_opengl_addCommand(CommandBuffer_p, "glDrawArrays", 
            nh_opengl_glenum(NULL, GL_TRIANGLES), nh_opengl_glint(NULL, offset), nh_opengl_glsizei(NULL, 6));
        offset += 6;
        // Render outer box.
        nh_opengl_addCommand(CommandBuffer_p, "glUniform3f", 
           &Data_p->Background.OpenGL.GetUniformLocation_p->Result, 
           nh_opengl_glfloat(NULL, Box_p->accent ? State_p->Gradient.Color.r : Config.Foreground.r), 
           nh_opengl_glfloat(NULL, Box_p->accent ? State_p->Gradient.Color.g : Config.Foreground.g), 
           nh_opengl_glfloat(NULL, Box_p->accent ? State_p->Gradient.Color.b : Config.Foreground.b));
        nh_opengl_addCommand(CommandBuffer_p, "glDrawArrays", 
            nh_opengl_glenum(NULL, GL_TRIANGLES), nh_opengl_glint(NULL, offset), nh_opengl_glsizei(NULL, 6));
        offset += 6;
    }

TTYR_TERMINAL_DIAGNOSTIC_END(TTYR_TERMINAL_SUCCESS)
}

static TTYR_TERMINAL_RESULT ttyr_terminal_prepareOpenGLBoxesDraw(
    ttyr_terminal_GraphicsState *State_p, ttyr_terminal_GraphicsData *Data_p)
{
TTYR_TERMINAL_BEGIN()

    nh_opengl_CommandBuffer *CommandBuffer_p = State_p->Viewport_p->OpenGL.CommandBuffer_p;
    nh_opengl_addCommand(CommandBuffer_p, "glUseProgram", &Data_p->Background.OpenGL.Program_p->Result);

TTYR_TERMINAL_DIAGNOSTIC_END(TTYR_TERMINAL_SUCCESS)
}

// RENDER ==========================================================================================

TTYR_TERMINAL_RESULT ttyr_terminal_renderUsingOpenGL(
    ttyr_terminal_Graphics *Graphics_p, ttyr_terminal_Grid *Grid_p, ttyr_terminal_Grid *Grid2_p)
{
TTYR_TERMINAL_BEGIN()

    NH_BOOL blockUntilRender = Graphics_p->Data1.Background.Action.init || Graphics_p->Data1.Foreground.Action.init;

    nh_gfx_beginRecording(Graphics_p->State.Viewport_p);

    nh_opengl_CommandBuffer *CommandBuffer_p = Graphics_p->State.Viewport_p->OpenGL.CommandBuffer_p;

    nh_opengl_addCommand(CommandBuffer_p, "glEnable", nh_opengl_glenum(NULL, GL_BLEND));
    nh_opengl_addCommand(CommandBuffer_p, "glEnable", nh_opengl_glenum(NULL, GL_DEPTH_TEST));
 
    nh_opengl_addCommand(CommandBuffer_p, "glBlendFunc", nh_opengl_glenum(NULL, GL_SRC_ALPHA),
        nh_opengl_glenum(NULL, GL_ONE_MINUS_SRC_ALPHA));

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

TTYR_TERMINAL_DIAGNOSTIC_END(TTYR_TERMINAL_SUCCESS)
}

