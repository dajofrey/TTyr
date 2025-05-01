// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES =======================================================================================

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

static TTYR_TERMINAL_RESULT tk_terminal_initOpenGLBackgroundProgram(
    tk_terminal_OpenGLBackground *Background_p, nh_gfx_OpenGLCommandBuffer *CommandBuffer_p)
{
    static const char* vsSource_p =
        "#version 450\n"
        "layout(location=0) in vec3 position;\n"
        "layout(location=1) in vec3 in_color;\n"
        "out vec3 color;\n"
        "void main() {\n"
        "    color = in_color;\n"
        "    gl_Position = vec4(position, 1.0);\n"
        "}\n";

    static const char* fsSource_p =
        "#version 450\n"
        "in vec3 color;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "    fragColor = vec4(color, 1.0);\n"
        "}\n";

    Background_p->VertexShader_p =
        nh_gfx_disableOpenGLCommandAutoFree(
            nh_gfx_addOpenGLCommand(CommandBuffer_p, "glCreateShader", nh_gfx_glint(NULL, GL_VERTEX_SHADER)));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glShaderSource",
        &Background_p->VertexShader_p->Result, nh_gfx_gluint(NULL, 1),
        nh_gfx_glchar(NULL, NULL, 0, (GLchar**)&vsSource_p),
        nh_gfx_glpointer(NULL, NULL));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glCompileShader", &Background_p->VertexShader_p->Result);

    Background_p->FragmentShader_p =
        nh_gfx_disableOpenGLCommandAutoFree(
            nh_gfx_addOpenGLCommand(CommandBuffer_p, "glCreateShader", nh_gfx_glint(NULL, GL_FRAGMENT_SHADER)));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glShaderSource",
        &Background_p->FragmentShader_p->Result, nh_gfx_gluint(NULL, 1),
        nh_gfx_glchar(NULL, NULL, 0, (GLchar**)&fsSource_p),
        nh_gfx_glpointer(NULL, NULL));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glCompileShader", &Background_p->FragmentShader_p->Result);

    Background_p->Program_p = 
        nh_gfx_disableOpenGLCommandAutoFree(nh_gfx_addOpenGLCommand(CommandBuffer_p, "glCreateProgram"));

    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glAttachShader", &Background_p->Program_p->Result,
        &Background_p->VertexShader_p->Result);
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glAttachShader", &Background_p->Program_p->Result,
        &Background_p->FragmentShader_p->Result);

    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glLinkProgram", &Background_p->Program_p->Result);

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT tk_terminal_initOpenGLBackgroundVertices(
    tk_terminal_OpenGLBackground *Background_p, nh_gfx_OpenGLCommandBuffer *CommandBuffer_p)
{
    Background_p->VertexArray_p = nh_gfx_disableOpenGLDataAutoFree(nh_gfx_gluint(NULL, 0));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glGenVertexArrays", nh_gfx_gluint(NULL, 1),
        Background_p->VertexArray_p);
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBindVertexArray", Background_p->VertexArray_p);

    Background_p->IndicesBuffer_p = nh_gfx_disableOpenGLDataAutoFree(nh_gfx_gluint(NULL, 0));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glGenBuffers", nh_gfx_gluint(NULL, 1), 
        Background_p->IndicesBuffer_p);

    Background_p->VerticesBuffer_p = nh_gfx_disableOpenGLDataAutoFree(nh_gfx_gluint(NULL, 0));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glGenBuffers", nh_gfx_gluint(NULL, 1), 
        Background_p->VerticesBuffer_p);

    Background_p->ColorBuffer_p = nh_gfx_disableOpenGLDataAutoFree(nh_gfx_gluint(NULL, 0));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glGenBuffers", nh_gfx_gluint(NULL, 1), 
        Background_p->ColorBuffer_p);

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT tk_terminal_updateOpenGLBackgroundVertices(
    tk_terminal_OpenGLBackground *Background_p, nh_gfx_OpenGLCommandBuffer *CommandBuffer_p, nh_core_Array *Vertices_p,
    nh_core_Array *Indices_p, nh_core_Array *Colors_p)
{
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBindVertexArray", Background_p->VertexArray_p);

    // Indices.
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBindBuffer",
        nh_gfx_glenum(NULL, GL_ELEMENT_ARRAY_BUFFER), Background_p->IndicesBuffer_p);
    Background_p->BufferData_p = nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBufferData",
        nh_gfx_glenum(NULL, GL_ELEMENT_ARRAY_BUFFER),
        nh_gfx_glsizeiptr(NULL, Indices_p->length*sizeof(uint32_t)),
        nh_gfx_glubyte(NULL, Indices_p->p, Indices_p->length*sizeof(uint32_t)),
        nh_gfx_glenum(NULL, GL_STATIC_DRAW));

    // Positions
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBindBuffer",
        nh_gfx_glenum(NULL, GL_ARRAY_BUFFER), Background_p->VerticesBuffer_p);
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBufferData", 
        nh_gfx_glenum(NULL, GL_ARRAY_BUFFER), 
        nh_gfx_glsizeiptr(NULL, Vertices_p->length * sizeof(GLfloat)),
        nh_gfx_glubyte(NULL, Vertices_p->p, Vertices_p->length * sizeof(GLfloat)),
        nh_gfx_glenum(NULL, GL_STATIC_DRAW));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glEnableVertexAttribArray", nh_gfx_gluint(NULL, 0));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glVertexAttribPointer",
        nh_gfx_gluint(NULL, 0), nh_gfx_gluint(NULL, 3), nh_gfx_glenum(NULL, GL_FLOAT),
        nh_gfx_glboolean(NULL, GL_FALSE), nh_gfx_glsizei(NULL, sizeof(float) * 3),
        nh_gfx_glpointer(NULL, NULL));
    
    // Colors
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBindBuffer",
        nh_gfx_glenum(NULL, GL_ARRAY_BUFFER), Background_p->ColorBuffer_p);
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBufferData", 
        nh_gfx_glenum(NULL, GL_ARRAY_BUFFER), 
        nh_gfx_glsizeiptr(NULL, Colors_p->length * sizeof(GLfloat)),
        nh_gfx_glubyte(NULL, Colors_p->p, Colors_p->length * sizeof(GLfloat)),
        nh_gfx_glenum(NULL, GL_DYNAMIC_DRAW));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glEnableVertexAttribArray", nh_gfx_gluint(NULL, 1));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glVertexAttribPointer",
        nh_gfx_gluint(NULL, 1), nh_gfx_gluint(NULL, 3), nh_gfx_glenum(NULL, GL_FLOAT),
        nh_gfx_glboolean(NULL, GL_FALSE), nh_gfx_glsizei(NULL, sizeof(float) * 3),
        nh_gfx_glpointer(NULL, NULL));

    return TTYR_TERMINAL_SUCCESS;
}

TTYR_TERMINAL_RESULT tk_terminal_updateOpenGLBackground(
    void *state_p, void *data_p)
{
    tk_terminal_GraphicsState *State_p = state_p;
    tk_terminal_GraphicsData *Data_p = data_p;

    if (Data_p->Background.Action.init) {
        tk_terminal_initOpenGLBackgroundProgram(&Data_p->Background.OpenGL, 
            State_p->Viewport_p->OpenGL.CommandBuffer_p);
        tk_terminal_initOpenGLBackgroundVertices(&Data_p->Background.OpenGL, 
            State_p->Viewport_p->OpenGL.CommandBuffer_p);
        Data_p->Background.Action.init = false;
    }

    tk_terminal_updateOpenGLBackgroundVertices(&Data_p->Background.OpenGL,
        State_p->Viewport_p->OpenGL.CommandBuffer_p, &Data_p->Background.Vertices,
        &Data_p->Background.Indices, &Data_p->Background.Colors);

    return TTYR_TERMINAL_SUCCESS;
}

TTYR_TERMINAL_RESULT tk_terminal_initOpenGLBackground(
    tk_terminal_OpenGLBackground *Background_p)
{
    memset(Background_p, 0, sizeof(tk_terminal_OpenGLBackground));
 
    return TTYR_TERMINAL_SUCCESS;
}

TTYR_TERMINAL_RESULT tk_terminal_freeOpenGLBackground(
    tk_terminal_OpenGLBackground *Background_p)
{
    nh_gfx_freeOpenGLCommand(Background_p->VertexShader_p);
    nh_gfx_freeOpenGLCommand(Background_p->FragmentShader_p);
    nh_gfx_freeOpenGLCommand(Background_p->Program_p);
    
    nh_gfx_freeOpenGLData(Background_p->VertexArray_p);
    nh_gfx_freeOpenGLData(Background_p->IndicesBuffer_p);
    nh_gfx_freeOpenGLData(Background_p->ColorBuffer_p);
    nh_gfx_freeOpenGLData(Background_p->VerticesBuffer_p);

    return TTYR_TERMINAL_SUCCESS;
}
