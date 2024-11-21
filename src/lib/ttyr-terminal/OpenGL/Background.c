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

static TTYR_TERMINAL_RESULT ttyr_terminal_initOpenGLBackgroundProgram(
    ttyr_terminal_OpenGLBackground *Background_p, nh_gfx_OpenGLCommandBuffer *CommandBuffer_p)
{
    static const char* vsSource_p =
        "#version 450\n"
        "layout(location=0) in vec3 position;\n"
        "uniform vec3 in_color;\n"
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

    static GLchar *colorName_p = "in_color";
    Background_p->GetUniformLocation_p =
        nh_gfx_disableOpenGLCommandAutoFree(nh_gfx_addOpenGLCommand(CommandBuffer_p, "glGetUniformLocation", 
            &Background_p->Program_p->Result, nh_gfx_glchar(NULL, NULL, 0, &colorName_p)));
  
    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_initOpenGLBackgroundVertices(
    ttyr_terminal_OpenGLBackground *Background_p, nh_gfx_OpenGLCommandBuffer *CommandBuffer_p)
{
    Background_p->VertexArray_p = nh_gfx_disableOpenGLDataAutoFree(nh_gfx_gluint(NULL, 0));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glGenVertexArrays", nh_gfx_gluint(NULL, 1),
        Background_p->VertexArray_p);
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBindVertexArray", Background_p->VertexArray_p);

    Background_p->IndicesBuffer_p = nh_gfx_disableOpenGLDataAutoFree(nh_gfx_gluint(NULL, 0));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glGenBuffers", nh_gfx_gluint(NULL, 1), 
        Background_p->IndicesBuffer_p);
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBindBuffer",
        nh_gfx_glenum(NULL, GL_ELEMENT_ARRAY_BUFFER), Background_p->IndicesBuffer_p);

    Background_p->VerticesBuffer_p = nh_gfx_disableOpenGLDataAutoFree(nh_gfx_gluint(NULL, 0));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glGenBuffers", nh_gfx_gluint(NULL, 1), 
        Background_p->VerticesBuffer_p);
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBindBuffer",
        nh_gfx_glenum(NULL, GL_ELEMENT_ARRAY_BUFFER), Background_p->VerticesBuffer_p);

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_updateOpenGLBackgroundVertices(
    ttyr_terminal_OpenGLBackground *Background_p, nh_gfx_OpenGLCommandBuffer *CommandBuffer_p, nh_core_Array *Vertices_p,
    nh_core_Array *Indices_p)
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

    // Vertices.
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBindBuffer",
        nh_gfx_glenum(NULL, GL_ARRAY_BUFFER), Background_p->VerticesBuffer_p);
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glBufferData", 
        nh_gfx_glenum(NULL, GL_ARRAY_BUFFER), 
        nh_gfx_glsizeiptr(NULL, Vertices_p->length*sizeof(GLfloat)),
        nh_gfx_glubyte(NULL, Vertices_p->p, Vertices_p->length*sizeof(GLfloat)),
        nh_gfx_glenum(NULL, GL_STATIC_DRAW));

    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glEnableVertexAttribArray", nh_gfx_gluint(NULL, 0));
    nh_gfx_addOpenGLCommand(CommandBuffer_p, "glVertexAttribPointer",
        nh_gfx_gluint(NULL, 0), nh_gfx_gluint(NULL, 3), nh_gfx_glenum(NULL, GL_FLOAT),
        nh_gfx_glboolean(NULL, GL_FALSE), nh_gfx_glsizei(NULL, sizeof(float)*3), 
        nh_gfx_glpointer(NULL, NULL));

    return TTYR_TERMINAL_SUCCESS;
}

TTYR_TERMINAL_RESULT ttyr_terminal_updateOpenGLBackground(
    void *state_p, void *data_p)
{
    ttyr_terminal_GraphicsState *State_p = state_p;
    ttyr_terminal_GraphicsData *Data_p = data_p;

    if (Data_p->Background.Action.init) {
        ttyr_terminal_initOpenGLBackgroundProgram(&Data_p->Background.OpenGL, 
            State_p->Viewport_p->OpenGL.CommandBuffer_p);
        ttyr_terminal_initOpenGLBackgroundVertices(&Data_p->Background.OpenGL, 
            State_p->Viewport_p->OpenGL.CommandBuffer_p);
        Data_p->Background.Action.init = false;
    }

    ttyr_terminal_updateOpenGLBackgroundVertices(&Data_p->Background.OpenGL,
        State_p->Viewport_p->OpenGL.CommandBuffer_p, &Data_p->Background.Vertices,
        &Data_p->Background.Indices);

    return TTYR_TERMINAL_SUCCESS;
}

TTYR_TERMINAL_RESULT ttyr_terminal_initOpenGLBackground(
    ttyr_terminal_OpenGLBackground *Background_p)
{
    memset(Background_p, 0, sizeof(ttyr_terminal_OpenGLBackground));
 
    return TTYR_TERMINAL_SUCCESS;
}

TTYR_TERMINAL_RESULT ttyr_terminal_freeOpenGLBackground(
    ttyr_terminal_OpenGLBackground *Background_p)
{
    nh_gfx_freeOpenGLCommand(Background_p->VertexShader_p);
    nh_gfx_freeOpenGLCommand(Background_p->FragmentShader_p);
    nh_gfx_freeOpenGLCommand(Background_p->Program_p);
    nh_gfx_freeOpenGLCommand(Background_p->GetUniformLocation_p);
    
    nh_gfx_freeOpenGLData(Background_p->VertexArray_p);
    nh_gfx_freeOpenGLData(Background_p->IndicesBuffer_p);
    nh_gfx_freeOpenGLData(Background_p->VerticesBuffer_p);

    return TTYR_TERMINAL_SUCCESS;
}
