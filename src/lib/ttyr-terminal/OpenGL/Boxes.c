// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Background.h"

#include "../Terminal/Graphics.h"
#include "../Common/Macros.h"

#include "nh-gfx/Base/Viewport.h"
#include "nh-gfx/Common/Macros.h"

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>
#include <float.h>

// FUNCTIONS =======================================================================================

static TTYR_TERMINAL_RESULT ttyr_terminal_initOpenGLBoxesVertices(
    ttyr_terminal_OpenGLBoxes *Boxes_p, nh_gfx_OpenGLCommandBuffer *CommandBuffer_p)
{
    Boxes_p->VertexArray_p = nh_gfx_disableOpenGLDataAutoFree(nh_gfx_gluint(NULL, 0));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glGenVertexArrays", nh_gfx_gluint(NULL, 1),
        Boxes_p->VertexArray_p);
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBindVertexArray", Boxes_p->VertexArray_p);

    Boxes_p->VerticesBuffer_p = nh_gfx_disableOpenGLDataAutoFree(nh_gfx_gluint(NULL, 0));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glGenBuffers", nh_gfx_gluint(NULL, 1), 
        Boxes_p->VerticesBuffer_p);
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBindBuffer",
        nh_gfx_glenum(NULL, GL_ELEMENT_ARRAY_BUFFER), Boxes_p->VerticesBuffer_p);

    Boxes_p->ColorBuffer_p = nh_gfx_disableOpenGLDataAutoFree(nh_gfx_gluint(NULL, 0));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glGenBuffers", nh_gfx_gluint(NULL, 1),
        Boxes_p->ColorBuffer_p);

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_updateOpenGLBoxesVertices(
    ttyr_terminal_GraphicsBoxes *Boxes_p, nh_gfx_OpenGLCommandBuffer *CommandBuffer_p)
{
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBindVertexArray", Boxes_p->OpenGL.VertexArray_p);

    // Vertices.
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBindBuffer",
        nh_gfx_glenum(NULL, GL_ARRAY_BUFFER), Boxes_p->OpenGL.VerticesBuffer_p);
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBufferData", 
        nh_gfx_glenum(NULL, GL_ARRAY_BUFFER), 
        nh_gfx_glsizeiptr(NULL, Boxes_p->Vertices.length*sizeof(GLfloat)),
        nh_gfx_glubyte(NULL, Boxes_p->Vertices.p, Boxes_p->Vertices.length*sizeof(GLfloat)),
        nh_gfx_glenum(NULL, GL_STATIC_DRAW));

    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glEnableVertexAttribArray", nh_gfx_gluint(NULL, 0));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glVertexAttribPointer",
        nh_gfx_gluint(NULL, 0), nh_gfx_gluint(NULL, 3), nh_gfx_glenum(NULL, GL_FLOAT),
        nh_gfx_glboolean(NULL, GL_FALSE), nh_gfx_glsizei(NULL, sizeof(float)*3), 
        nh_gfx_glpointer(NULL, NULL));

    // Colors
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBindBuffer",
        nh_gfx_glenum(NULL, GL_ARRAY_BUFFER), Boxes_p->OpenGL.ColorBuffer_p);
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBufferData", 
        nh_gfx_glenum(NULL, GL_ARRAY_BUFFER), 
        nh_gfx_glsizeiptr(NULL, Boxes_p->Colors.length * sizeof(GLfloat)),
        nh_gfx_glubyte(NULL, Boxes_p->Colors.p, Boxes_p->Colors.length * sizeof(GLfloat)),
        nh_gfx_glenum(NULL, GL_DYNAMIC_DRAW));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glEnableVertexAttribArray", nh_gfx_gluint(NULL, 1));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glVertexAttribPointer",
        nh_gfx_gluint(NULL, 1), nh_gfx_gluint(NULL, 3), nh_gfx_glenum(NULL, GL_FLOAT),
        nh_gfx_glboolean(NULL, GL_FALSE), nh_gfx_glsizei(NULL, sizeof(float) * 3),
        nh_gfx_glpointer(NULL, NULL));

    return TTYR_TERMINAL_SUCCESS;
}

TTYR_TERMINAL_RESULT ttyr_terminal_updateOpenGLBoxes(
    void *state_p, void *data_p)
{
    ttyr_terminal_GraphicsData *Data_p = data_p;
    ttyr_terminal_GraphicsState *State_p = state_p;

    if (Data_p->Boxes.Action.init) {
        ttyr_terminal_initOpenGLBoxesVertices(
            &Data_p->Boxes.OpenGL, State_p->Viewport_p->OpenGL.CommandBuffer_p);
        Data_p->Boxes.Action.init = false;
    }

    ttyr_terminal_updateOpenGLBoxesVertices(
        &Data_p->Boxes, State_p->Viewport_p->OpenGL.CommandBuffer_p);

    return TTYR_TERMINAL_SUCCESS;
}

TTYR_TERMINAL_RESULT ttyr_terminal_initOpenGLBoxes(
    ttyr_terminal_OpenGLBoxes *Boxes_p)
{
    memset(Boxes_p, 0, sizeof(ttyr_terminal_OpenGLBoxes));
 
    return TTYR_TERMINAL_SUCCESS;
}

TTYR_TERMINAL_RESULT ttyr_terminal_freeOpenGLBoxes(
    ttyr_terminal_OpenGLBoxes *Boxes_p)
{
    nh_gfx_freeOpenGLData(Boxes_p->VertexArray_p);
    nh_gfx_freeOpenGLData(Boxes_p->VerticesBuffer_p);
    nh_gfx_freeOpenGLData(Boxes_p->ColorBuffer_p);
    nh_gfx_freeOpenGLCommand(Boxes_p->BufferData_p);
 
    return TTYR_TERMINAL_SUCCESS;
}
