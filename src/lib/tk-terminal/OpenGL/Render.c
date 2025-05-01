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

static TTYR_TERMINAL_RESULT tk_terminal_drawOpenGLDim(
    tk_terminal_GraphicsState *State_p, tk_terminal_GraphicsData *Data_p, tk_terminal_Grid *Grid_p)
{
    nh_gfx_OpenGLCommandBuffer *CommandBuffer_p = State_p->Viewport_p->OpenGL.CommandBuffer_p;

    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glUseProgram", &Data_p->Dim.OpenGL.Program_p->Result);
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBindVertexArray", Data_p->Dim.OpenGL.VertexArray_p);

    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glDrawArrays", 
        nh_gfx_glenum(NULL, GL_TRIANGLES), nh_gfx_glint(NULL, 0), nh_gfx_glsizei(NULL, 6));

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT tk_terminal_drawOpenGLBackground(
    tk_terminal_GraphicsState *State_p, tk_terminal_GraphicsData *Data_p)
{
    nh_gfx_OpenGLCommandBuffer *CommandBuffer_p = State_p->Viewport_p->OpenGL.CommandBuffer_p;

    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBindVertexArray", Data_p->Background.OpenGL.VertexArray_p);
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glUseProgram", &Data_p->Background.OpenGL.Program_p->Result);

    for (int i = 0, offset = 0; i < Data_p->Background.Ranges.length; ++i) {
        tk_terminal_AttributeRange *Range_p = ((tk_terminal_AttributeRange*)Data_p->Background.Ranges.p)+i;
        nh_gfx_addOpenGLCommand(CommandBuffer_p, "glDrawElements", 
            nh_gfx_glenum(NULL, GL_TRIANGLES), nh_gfx_glsizei(NULL, Range_p->indices), 
            nh_gfx_glenum(NULL, GL_UNSIGNED_INT),
            nh_gfx_glpointer(NULL, (void*)(sizeof(uint32_t)*offset)));
        offset += Range_p->indices;
    }

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT tk_terminal_drawOpenGLForeground(
    tk_terminal_GraphicsState *State_p, tk_terminal_GraphicsData *Data_p)
{
    nh_gfx_OpenGLCommandBuffer *CommandBuffer_p = State_p->Viewport_p->OpenGL.CommandBuffer_p;

    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBindVertexArray", Data_p->Foreground.OpenGL.VertexArray_p);
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glUseProgram", &Data_p->Foreground.OpenGL.Program_p->Result);
 
    for (int i = 0, offset = 0; i < Data_p->Foreground.Ranges.length; ++i) {
        tk_terminal_AttributeRange *Range_p = ((tk_terminal_AttributeRange*)Data_p->Foreground.Ranges.p)+i;
        nh_gfx_addOpenGLCommand(CommandBuffer_p, "glDrawElements", 
            nh_gfx_glenum(NULL, GL_TRIANGLES), nh_gfx_glsizei(NULL, Range_p->indices), 
            nh_gfx_glenum(NULL, GL_UNSIGNED_INT),
            nh_gfx_glpointer(NULL, (void*)(sizeof(uint32_t)*offset)));
        offset += Range_p->indices;
    }

    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glUseProgram", &Data_p->Foreground.OpenGL.Program2_p->Result);
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBindVertexArray", Data_p->Foreground.OpenGL.VertexArray2_p);

    for (int i = 0, offset = 0; i < Data_p->Foreground.Ranges2.length; ++i) {
        tk_terminal_AttributeRange *Range_p = ((tk_terminal_AttributeRange*)Data_p->Foreground.Ranges2.p)+i;
        nh_gfx_addOpenGLCommand(CommandBuffer_p, "glDrawElements", 
            nh_gfx_glenum(NULL, GL_TRIANGLES), nh_gfx_glsizei(NULL, Range_p->indices), 
            nh_gfx_glenum(NULL, GL_UNSIGNED_INT),
            nh_gfx_glpointer(NULL, (void*)(sizeof(uint32_t)*offset)));
        offset += Range_p->indices;
    }

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT tk_terminal_drawOpenGLMenuBackground(
    tk_terminal_GraphicsState *State_p, tk_terminal_GraphicsData *Data_p, tk_terminal_GraphicsData *Data1_p, 
    tk_terminal_Grid *Grid_p, tk_terminal_Grid *Grid2_p)
{
    for (int i = 0; i < Grid2_p->Boxes.length; ++i) {
        tk_terminal_Box *Box_p = ((tk_terminal_Box*)Grid2_p->Boxes.p)+i;
        if (Box_p->UpperLeft.x == Box_p->LowerRight.x) {continue;}
        nh_gfx_addOpenGLCommand(
            State_p->Viewport_p->OpenGL.CommandBuffer_p,
            "glScissor",
            nh_gfx_glint(NULL, ((Box_p->UpperLeft.x)*Grid_p->TileSize.width)+Grid2_p->borderPixel+Grid2_p->TileSize.width/2),
            nh_gfx_glint(NULL, ((Grid2_p->Size.height-((Box_p->LowerRight.y)*Grid2_p->TileSize.height))+Grid2_p->borderPixel)-Grid2_p->TileSize.height/2),
            nh_gfx_glsizei(NULL, ((((Box_p->LowerRight.x)-Box_p->UpperLeft.x)*Grid2_p->TileSize.width-Grid2_p->borderPixel)-Grid2_p->TileSize.width/2)),
            nh_gfx_glsizei(NULL, (((Box_p->LowerRight.y)-Box_p->UpperLeft.y)*Grid2_p->TileSize.height)+Grid2_p->TileSize.height));
        nh_gfx_addOpenGLCommand(
            State_p->Viewport_p->OpenGL.CommandBuffer_p,
            "glViewport",
            nh_gfx_glint(NULL, 0),
            nh_gfx_glint(NULL, 0),
            nh_gfx_glsizei(NULL, Grid_p->Size.width),
            nh_gfx_glsizei(NULL, Grid_p->Size.height));
        tk_terminal_drawOpenGLBackground(State_p, Data_p);
        tk_terminal_drawOpenGLDim(State_p, Data1_p, Grid2_p);
    }

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT tk_terminal_drawOpenGLInactiveCursor(
    tk_terminal_GraphicsState *State_p, tk_terminal_GraphicsData *Data_p, tk_terminal_Grid *Grid_p)
{
    nh_gfx_OpenGLCommandBuffer *CommandBuffer_p = State_p->Viewport_p->OpenGL.CommandBuffer_p;
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glUseProgram", &Data_p->Background.OpenGL.Program_p->Result);
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBindVertexArray", Data_p->Boxes.OpenGL.VertexArray_p);

    int offset = 0;
    for (int i = 0; i < Grid_p->Boxes.length; ++i) {
        tk_terminal_Box *Box_p = ((tk_terminal_Box*)Grid_p->Boxes.p)+i;
        if (Box_p->UpperLeft.x < 0 || Box_p->UpperLeft.y < 0 || Box_p->LowerRight.x < 0 || Box_p->LowerRight.y < 0) {continue;}
        if (Box_p->UpperLeft.x != Box_p->LowerRight.x) {continue;}
        // Render inner box.
        nh_gfx_addOpenGLCommand(CommandBuffer_p, "glDrawArrays", 
            nh_gfx_glenum(NULL, GL_TRIANGLES), nh_gfx_glint(NULL, offset), nh_gfx_glsizei(NULL, 6));
        offset += 6;
        // Render outer box.
        nh_gfx_addOpenGLCommand(CommandBuffer_p, "glDrawArrays", 
            nh_gfx_glenum(NULL, GL_TRIANGLES), nh_gfx_glint(NULL, offset), nh_gfx_glsizei(NULL, 6));
        offset += 6;
        nh_gfx_addOpenGLCommand(
            State_p->Viewport_p->OpenGL.CommandBuffer_p,
            "glScissor",
            nh_gfx_glint(NULL, ((Box_p->UpperLeft.x+1)*Grid_p->TileSize.width)+Grid_p->borderPixel*1.5f),
            nh_gfx_glint(NULL, Grid_p->Size.height-((Box_p->LowerRight.y+1)*Grid_p->TileSize.height)+Grid_p->borderPixel*1.5f),
            nh_gfx_glsizei(NULL, Grid_p->TileSize.width-Grid_p->borderPixel),
            nh_gfx_glsizei(NULL, Grid_p->TileSize.height-Grid_p->borderPixel));
        tk_terminal_drawOpenGLDim(State_p, Data_p, Grid_p);
    }

    return TTYR_TERMINAL_SUCCESS;
}

TTYR_TERMINAL_RESULT tk_terminal_renderUsingOpenGL(
    tk_terminal_Config *Config_p, tk_terminal_Graphics *Graphics_p, tk_terminal_Grid *Grid_p,
    tk_terminal_Grid *Grid2_p, tk_terminal_Grid *BorderGrid_p)
{
    bool blockUntilRender = Graphics_p->Data1.Background.Action.init || Graphics_p->Data1.Foreground.Action.init;

    nh_gfx_beginRecording(Graphics_p->State.Viewport_p);

 // first render border grid

   nh_gfx_addOpenGLCommand(
       Graphics_p->State.Viewport_p->OpenGL.CommandBuffer_p,
       "glViewport",
       nh_gfx_glint(NULL, 0),
       nh_gfx_glint(NULL, 0),
       nh_gfx_glsizei(NULL, BorderGrid_p->Size.width),
       nh_gfx_glsizei(NULL, BorderGrid_p->Size.height));

    TTYR_TERMINAL_CHECK(tk_terminal_updateOpenGLBackground(&Graphics_p->State, &Graphics_p->BorderData))
    TTYR_TERMINAL_CHECK(tk_terminal_drawOpenGLBackground(&Graphics_p->State, &Graphics_p->BorderData))

// clear except borders

    if (Config_p->style == 0) {
        nh_gfx_addOpenGLCommand(
            Graphics_p->State.Viewport_p->OpenGL.CommandBuffer_p,
            "glViewport",
            nh_gfx_glint(NULL, 0),
            nh_gfx_glint(NULL, 0),
            nh_gfx_glsizei(NULL, Grid_p->Size.width+Grid_p->borderPixel*2),
            nh_gfx_glsizei(NULL, Grid_p->Size.height+Grid_p->borderPixel*2));
 
        nh_gfx_addOpenGLCommand(
            Graphics_p->State.Viewport_p->OpenGL.CommandBuffer_p,
            "glScissor",
            nh_gfx_glint(NULL, Grid_p->borderPixel),
            nh_gfx_glint(NULL, Grid_p->borderPixel),
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
    }

// second render terminal

    nh_gfx_addOpenGLCommand(
        Graphics_p->State.Viewport_p->OpenGL.CommandBuffer_p,
        "glViewport",
        nh_gfx_glint(NULL, Grid_p->borderPixel),
        nh_gfx_glint(NULL, Grid_p->borderPixel),
        nh_gfx_glsizei(NULL, Grid_p->Size.width),
        nh_gfx_glsizei(NULL, Grid_p->Size.height));
 
    nh_gfx_OpenGLCommandBuffer *CommandBuffer_p = Graphics_p->State.Viewport_p->OpenGL.CommandBuffer_p;

    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glEnable", nh_gfx_glenum(NULL, GL_BLEND));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glEnable", nh_gfx_glenum(NULL, GL_DEPTH_TEST));
 
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBlendFunc", nh_gfx_glenum(NULL, GL_SRC_ALPHA),
        nh_gfx_glenum(NULL, GL_ONE_MINUS_SRC_ALPHA));

    TTYR_TERMINAL_CHECK(tk_terminal_updateOpenGLDim(&Graphics_p->State, &Graphics_p->Data1))
 
    if (Config_p->style > 0) {
        TTYR_TERMINAL_CHECK(tk_terminal_drawOpenGLDim(&Graphics_p->State, &Graphics_p->Data1, Grid_p))
    }

    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glDisable", nh_gfx_glenum(NULL, GL_DEPTH_TEST));
 
    TTYR_TERMINAL_CHECK(tk_terminal_updateOpenGLBackground(&Graphics_p->State, &Graphics_p->Data1))
    TTYR_TERMINAL_CHECK(tk_terminal_drawOpenGLBackground(&Graphics_p->State, &Graphics_p->Data1))

    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glDisable", nh_gfx_glenum(NULL, GL_DEPTH_TEST));
 
    TTYR_TERMINAL_CHECK(tk_terminal_updateOpenGLBoxes(&Graphics_p->State, &Graphics_p->Data1))
    TTYR_TERMINAL_CHECK(tk_terminal_drawOpenGLInactiveCursor(&Graphics_p->State, &Graphics_p->Data1, Grid_p))

    nh_gfx_addOpenGLCommand(
        Graphics_p->State.Viewport_p->OpenGL.CommandBuffer_p,
        "glScissor",
        nh_gfx_glint(NULL, 0),
        nh_gfx_glint(NULL, 0),
        nh_gfx_glsizei(NULL, Grid_p->Size.width),
        nh_gfx_glsizei(NULL, Grid_p->Size.height));
 
    nh_gfx_addOpenGLCommand(
        Graphics_p->State.Viewport_p->OpenGL.CommandBuffer_p,
        "glViewport",
        nh_gfx_glint(NULL, Grid_p->borderPixel),
        nh_gfx_glint(NULL, Grid_p->borderPixel),
        nh_gfx_glsizei(NULL, Grid_p->Size.width),
        nh_gfx_glsizei(NULL, Grid_p->Size.height));
 
    TTYR_TERMINAL_CHECK(tk_terminal_updateOpenGLForeground(Config_p, &Graphics_p->State, &Graphics_p->Data1))
    TTYR_TERMINAL_CHECK(tk_terminal_drawOpenGLForeground(&Graphics_p->State, &Graphics_p->Data1))

    TTYR_TERMINAL_CHECK(tk_terminal_drawOpenGLMenuBackground(&Graphics_p->State, &Graphics_p->BorderData, &Graphics_p->Data1, BorderGrid_p, Grid_p))

    nh_gfx_addOpenGLCommand(
        Graphics_p->State.Viewport_p->OpenGL.CommandBuffer_p,
        "glScissor",
        nh_gfx_glint(NULL, 0),
        nh_gfx_glint(NULL, 0),
        nh_gfx_glsizei(NULL, Grid_p->Size.width),
        nh_gfx_glsizei(NULL, Grid_p->Size.height));
 
    nh_gfx_addOpenGLCommand(
        Graphics_p->State.Viewport_p->OpenGL.CommandBuffer_p,
        "glViewport",
        nh_gfx_glint(NULL, Grid_p->borderPixel),
        nh_gfx_glint(NULL, Grid_p->borderPixel),
        nh_gfx_glsizei(NULL, Grid_p->Size.width),
        nh_gfx_glsizei(NULL, Grid_p->Size.height));
 
    TTYR_TERMINAL_CHECK(tk_terminal_updateOpenGLBackground(&Graphics_p->State, &Graphics_p->Data2))
    TTYR_TERMINAL_CHECK(tk_terminal_drawOpenGLBackground(&Graphics_p->State, &Graphics_p->Data2))

    TTYR_TERMINAL_CHECK(tk_terminal_updateOpenGLForeground(Config_p, &Graphics_p->State, &Graphics_p->Data2))
    TTYR_TERMINAL_CHECK(tk_terminal_drawOpenGLForeground(&Graphics_p->State, &Graphics_p->Data2))
 
    nh_gfx_endRecording(Graphics_p->State.Viewport_p, blockUntilRender);

    return TTYR_TERMINAL_SUCCESS;
}
