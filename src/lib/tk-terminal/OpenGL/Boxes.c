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

static TK_TERMINAL_RESULT tk_terminal_initOpenGLBoxesVertices(
    tk_terminal_OpenGLBoxes *Boxes_p, nh_gfx_OpenGLCommandBuffer *CommandBuffer_p)
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

    return TK_TERMINAL_SUCCESS;
}

static TK_TERMINAL_RESULT tk_terminal_updateOpenGLBoxesVertices(
    tk_terminal_Boxes *Boxes_p, nh_gfx_OpenGLCommandBuffer *CommandBuffer_p)
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

    return TK_TERMINAL_SUCCESS;
}

TK_TERMINAL_RESULT tk_terminal_updateOpenGLBoxes(
    void *state_p, void *data_p)
{
    tk_terminal_Boxes *Boxes_p = data_p;
    tk_terminal_GraphicsState *State_p = state_p;

    if (Boxes_p->Action.init) {
        tk_terminal_initOpenGLBoxesVertices(
            &Boxes_p->OpenGL, State_p->Viewport_p->OpenGL.CommandBuffer_p);
        Boxes_p->Action.init = false;
    }

    tk_terminal_updateOpenGLBoxesVertices(Boxes_p, State_p->Viewport_p->OpenGL.CommandBuffer_p);

    return TK_TERMINAL_SUCCESS;
}

TK_TERMINAL_RESULT tk_terminal_initOpenGLBoxes(
    tk_terminal_OpenGLBoxes *Boxes_p)
{
    memset(Boxes_p, 0, sizeof(tk_terminal_OpenGLBoxes));
 
    return TK_TERMINAL_SUCCESS;
}

TK_TERMINAL_RESULT tk_terminal_freeOpenGLBoxes(
    tk_terminal_OpenGLBoxes *Boxes_p)
{
    nh_gfx_freeOpenGLData(Boxes_p->VertexArray_p);
    nh_gfx_freeOpenGLData(Boxes_p->VerticesBuffer_p);
    nh_gfx_freeOpenGLData(Boxes_p->ColorBuffer_p);
    nh_gfx_freeOpenGLCommand(Boxes_p->BufferData_p);
 
    return TK_TERMINAL_SUCCESS;
}
