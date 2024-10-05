// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES =======================================================================================

#include "Foreground.h"

#include "../Terminal/Graphics.h"

#include "../Common/Macros.h"
#include "../Common/Config.h"

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

// INIT HELPER =====================================================================================

static TTYR_TERMINAL_RESULT ttyr_terminal_initOpenGLForegroundPrograms(
    ttyr_terminal_OpenGLForeground *Foreground_p, nh_opengl_CommandBuffer *CommandBuffer_p)
{
TTYR_TERMINAL_BEGIN()

    static const char* vsSource_p =
        "#version 450\n"
        "layout(location=0) in vec3 position;\n"
        "layout(location=1) in vec2 uv;\n"
        "uniform vec3 in_color;\n"
        "out vec2 texcoord;\n"
        "out vec3 color;\n"
        "void main() {\n"
        "    texcoord = uv.xy;\n"
        "    color = in_color;\n"
        "    gl_Position = vec4(position, 1.0);\n"
        "}\n";

    static const char* fsSource_p =
        "#version 450\n"
        "#if __VERSION__ < 130\n"
        "#define TEXTURE2D texture2D\n"
        "#else\n"
        "#define TEXTURE2D texture\n"
        "#endif\n"
        "uniform sampler2D u_texture;\n"
        "in vec2 texcoord;\n"
        "in vec3 color;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "    float dist = TEXTURE2D(u_texture, texcoord.st).r;\n"
        "    float width = fwidth(dist);\n"
        "    float alpha = smoothstep(0.5-width, 0.5+width, dist);\n"
        "    fragColor = vec4(color, alpha);\n"
        "}\n";

    Foreground_p->VertexShader_p =
        nh_opengl_disableCommandAutoFree(
            nh_opengl_addCommand(CommandBuffer_p, "glCreateShader", nh_opengl_glint(NULL, GL_VERTEX_SHADER)));
    nh_opengl_addCommand(CommandBuffer_p, "glShaderSource",
        &Foreground_p->VertexShader_p->Result, nh_opengl_gluint(NULL, 1),
        nh_opengl_glchar(NULL, NULL, 0, (GLchar**)&vsSource_p),
        nh_opengl_pointer(NULL, NULL));
    nh_opengl_addCommand(CommandBuffer_p, "glCompileShader", &Foreground_p->VertexShader_p->Result);

    Foreground_p->FragmentShader_p =
        nh_opengl_disableCommandAutoFree(
            nh_opengl_addCommand(CommandBuffer_p, "glCreateShader", nh_opengl_glint(NULL, GL_FRAGMENT_SHADER)));
    nh_opengl_addCommand(CommandBuffer_p, "glShaderSource",
        &Foreground_p->FragmentShader_p->Result, nh_opengl_gluint(NULL, 1),
        nh_opengl_glchar(NULL, NULL, 0, (GLchar**)&fsSource_p),
        nh_opengl_pointer(NULL, NULL));
    nh_opengl_addCommand(CommandBuffer_p, "glCompileShader", &Foreground_p->FragmentShader_p->Result);

    Foreground_p->Program_p = 
        nh_opengl_disableCommandAutoFree(nh_opengl_addCommand(CommandBuffer_p, "glCreateProgram"));

    nh_opengl_addCommand(CommandBuffer_p, "glAttachShader", &Foreground_p->Program_p->Result,
        &Foreground_p->VertexShader_p->Result);
    nh_opengl_addCommand(CommandBuffer_p, "glAttachShader", &Foreground_p->Program_p->Result,
        &Foreground_p->FragmentShader_p->Result);

    nh_opengl_addCommand(CommandBuffer_p, "glLinkProgram", &Foreground_p->Program_p->Result);

    static GLchar *textureName_p = "u_texture";
    Foreground_p->GetUniformLocationTexture_p =
        nh_opengl_disableCommandAutoFree(nh_opengl_addCommand(CommandBuffer_p, "glGetUniformLocation",
            &Foreground_p->Program_p->Result, nh_opengl_glchar(NULL, NULL, 0, &textureName_p)));

    static GLchar *colorName_p = "in_color";
    Foreground_p->GetUniformLocationColor_p =
        nh_opengl_disableCommandAutoFree(nh_opengl_addCommand(CommandBuffer_p, "glGetUniformLocation", 
            &Foreground_p->Program_p->Result, nh_opengl_glchar(NULL, NULL, 0, &colorName_p)));

    static const char* vsSource2_p =
        "#version 450\n"
        "layout(location=0) in vec3 position;\n"
        "uniform vec3 in_color;\n"
        "out vec3 color;\n"
        "void main() {\n"
        "    color = in_color;\n"
        "    gl_Position = vec4(position, 1.0);\n"
        "}\n";

    static const char* fsSource2_p =
        "#version 450\n"
        "in vec3 color;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "    fragColor = vec4(color, 1.0);\n"
        "}\n";

    Foreground_p->VertexShader2_p =
        nh_opengl_disableCommandAutoFree(
            nh_opengl_addCommand(CommandBuffer_p, "glCreateShader", nh_opengl_glint(NULL, GL_VERTEX_SHADER)));
    nh_opengl_addCommand(CommandBuffer_p, "glShaderSource",
        &Foreground_p->VertexShader2_p->Result, nh_opengl_gluint(NULL, 1),
        nh_opengl_glchar(NULL, NULL, 0, (GLchar**)&vsSource2_p),
        nh_opengl_pointer(NULL, NULL));
    nh_opengl_addCommand(CommandBuffer_p, "glCompileShader", &Foreground_p->VertexShader2_p->Result);

    Foreground_p->FragmentShader2_p =
        nh_opengl_disableCommandAutoFree(
            nh_opengl_addCommand(CommandBuffer_p, "glCreateShader", nh_opengl_glint(NULL, GL_FRAGMENT_SHADER)));
    nh_opengl_addCommand(CommandBuffer_p, "glShaderSource",
        &Foreground_p->FragmentShader2_p->Result, nh_opengl_gluint(NULL, 1),
        nh_opengl_glchar(NULL, NULL, 0, (GLchar**)&fsSource2_p),
        nh_opengl_pointer(NULL, NULL));
    nh_opengl_addCommand(CommandBuffer_p, "glCompileShader", &Foreground_p->FragmentShader2_p->Result);

    Foreground_p->Program2_p = 
        nh_opengl_disableCommandAutoFree(nh_opengl_addCommand(CommandBuffer_p, "glCreateProgram"));

    nh_opengl_addCommand(CommandBuffer_p, "glAttachShader", &Foreground_p->Program2_p->Result,
        &Foreground_p->VertexShader2_p->Result);
    nh_opengl_addCommand(CommandBuffer_p, "glAttachShader", &Foreground_p->Program2_p->Result,
        &Foreground_p->FragmentShader2_p->Result);

    nh_opengl_addCommand(CommandBuffer_p, "glLinkProgram", &Foreground_p->Program2_p->Result);

    Foreground_p->GetUniformLocationColor2_p =
        nh_opengl_disableCommandAutoFree(nh_opengl_addCommand(CommandBuffer_p, "glGetUniformLocation",
            &Foreground_p->Program2_p->Result, nh_opengl_glchar(NULL, NULL, 0, &colorName_p)));

TTYR_TERMINAL_DIAGNOSTIC_END(TTYR_TERMINAL_SUCCESS)
}

static TTYR_TERMINAL_RESULT ttyr_terminal_initOpenGLFontTexture(
    ttyr_terminal_OpenGLForeground *Foreground_p, nh_opengl_CommandBuffer *CommandBuffer_p)
{
TTYR_TERMINAL_BEGIN()

    Foreground_p->Texture_p = nh_opengl_disableDataAutoFree(nh_opengl_gluint(NULL, 0));

    nh_opengl_addCommand(CommandBuffer_p, "glGenTextures", nh_opengl_glsizei(NULL, 1), Foreground_p->Texture_p);
    nh_opengl_addCommand(CommandBuffer_p, "glActiveTexture", nh_opengl_glenum(NULL, GL_TEXTURE0));
    nh_opengl_addCommand(CommandBuffer_p, "glBindTexture",
        nh_opengl_glenum(NULL, GL_TEXTURE_2D), Foreground_p->Texture_p);

    nh_opengl_addCommand(CommandBuffer_p, "glTexParameteri",
        nh_opengl_glenum(NULL, GL_TEXTURE_2D), nh_opengl_glenum(NULL, GL_TEXTURE_WRAP_S),
        nh_opengl_glenum(NULL, GL_CLAMP_TO_EDGE));
    nh_opengl_addCommand(CommandBuffer_p, "glTexParameteri",
        nh_opengl_glenum(NULL, GL_TEXTURE_2D), nh_opengl_glenum(NULL, GL_TEXTURE_WRAP_T),
        nh_opengl_glenum(NULL, GL_CLAMP_TO_EDGE));
    nh_opengl_addCommand(CommandBuffer_p, "glTexParameteri",
        nh_opengl_glenum(NULL, GL_TEXTURE_2D), nh_opengl_glenum(NULL, GL_TEXTURE_MAG_FILTER),
        nh_opengl_glenum(NULL, GL_LINEAR));
    nh_opengl_addCommand(CommandBuffer_p, "glTexParameteri",
        nh_opengl_glenum(NULL, GL_TEXTURE_2D), nh_opengl_glenum(NULL, GL_TEXTURE_MIN_FILTER),
        nh_opengl_glenum(NULL, GL_LINEAR));

TTYR_TERMINAL_DIAGNOSTIC_END(TTYR_TERMINAL_SUCCESS)
}

static TTYR_TERMINAL_RESULT ttyr_terminal_initOpenGLForegroundVertices(
    ttyr_terminal_OpenGLForeground *Foreground_p, nh_opengl_CommandBuffer *CommandBuffer_p)
{
TTYR_TERMINAL_BEGIN()

    Foreground_p->VertexArray_p = nh_opengl_disableDataAutoFree(nh_opengl_gluint(NULL, 0));
    nh_opengl_addCommand(CommandBuffer_p, "glGenVertexArrays", nh_opengl_gluint(NULL, 1),
        Foreground_p->VertexArray_p);
    nh_opengl_addCommand(CommandBuffer_p, "glBindVertexArray", Foreground_p->VertexArray_p);

    Foreground_p->IndicesBuffer_p = nh_opengl_disableDataAutoFree(nh_opengl_gluint(NULL, 0));
    nh_opengl_addCommand(CommandBuffer_p, "glGenBuffers", nh_opengl_gluint(NULL, 1), 
        Foreground_p->IndicesBuffer_p);
    nh_opengl_addCommand(CommandBuffer_p, "glBindBuffer",
        nh_opengl_glenum(NULL, GL_ELEMENT_ARRAY_BUFFER), Foreground_p->IndicesBuffer_p);

    Foreground_p->VerticesBuffer_p = nh_opengl_disableDataAutoFree(nh_opengl_gluint(NULL, 0));
    nh_opengl_addCommand(CommandBuffer_p, "glGenBuffers", nh_opengl_gluint(NULL, 1), 
        Foreground_p->VerticesBuffer_p);
    nh_opengl_addCommand(CommandBuffer_p, "glBindBuffer",
        nh_opengl_glenum(NULL, GL_ELEMENT_ARRAY_BUFFER), Foreground_p->VerticesBuffer_p);

    Foreground_p->VertexArray2_p = nh_opengl_disableDataAutoFree(nh_opengl_gluint(NULL, 0));
    nh_opengl_addCommand(CommandBuffer_p, "glGenVertexArrays", nh_opengl_gluint(NULL, 1),
        Foreground_p->VertexArray2_p);
    nh_opengl_addCommand(CommandBuffer_p, "glBindVertexArray", Foreground_p->VertexArray2_p);

    Foreground_p->IndicesBuffer2_p = nh_opengl_disableDataAutoFree(nh_opengl_gluint(NULL, 0));
    nh_opengl_addCommand(CommandBuffer_p, "glGenBuffers", nh_opengl_gluint(NULL, 1), 
        Foreground_p->IndicesBuffer2_p);
    nh_opengl_addCommand(CommandBuffer_p, "glBindBuffer",
        nh_opengl_glenum(NULL, GL_ELEMENT_ARRAY_BUFFER), Foreground_p->IndicesBuffer2_p);

    Foreground_p->VerticesBuffer2_p = nh_opengl_disableDataAutoFree(nh_opengl_gluint(NULL, 0));
    nh_opengl_addCommand(CommandBuffer_p, "glGenBuffers", nh_opengl_gluint(NULL, 1), 
        Foreground_p->VerticesBuffer2_p);
    nh_opengl_addCommand(CommandBuffer_p, "glBindBuffer",
        nh_opengl_glenum(NULL, GL_ELEMENT_ARRAY_BUFFER), Foreground_p->VerticesBuffer2_p);


TTYR_TERMINAL_DIAGNOSTIC_END(TTYR_TERMINAL_SUCCESS)
}

// UPDATE ==========================================================================================

static TTYR_TERMINAL_RESULT ttyr_terminal_updateOpenGLForegroundFont(
    ttyr_terminal_GraphicsState *State_p, ttyr_terminal_GraphicsForeground *Foreground_p, 
    nh_opengl_CommandBuffer *CommandBuffer_p)
{
TTYR_TERMINAL_BEGIN()

    ttyr_terminal_Config Config = ttyr_terminal_getConfig();
 
    nh_gfx_FontInstance *FontInstance_p = 
        nh_gfx_claimFontInstance(State_p->Fonts.pp[State_p->font], Config.fontSize);

    nh_opengl_addCommand(CommandBuffer_p, "glBindTexture",
        nh_opengl_glenum(NULL, GL_TEXTURE_2D), Foreground_p->OpenGL.Texture_p);

    nh_opengl_addCommand(CommandBuffer_p, "glTexImage2D",
        nh_opengl_glenum(NULL, GL_TEXTURE_2D), nh_opengl_glint(NULL, 0), nh_opengl_glint(NULL, GL_RED),
        nh_opengl_glsizei(NULL, FontInstance_p->Font_p->Atlas.width),
        nh_opengl_glsizei(NULL, FontInstance_p->Font_p->Atlas.height),
        nh_opengl_glint(NULL, 0), nh_opengl_glenum(NULL, GL_RED), nh_opengl_glenum(NULL, GL_UNSIGNED_BYTE),
        nh_opengl_glchar(NULL, NULL, 0, (GLchar**)&FontInstance_p->Font_p->Atlas.data_p));

TTYR_TERMINAL_END(TTYR_TERMINAL_SUCCESS)
}

static TTYR_TERMINAL_RESULT ttyr_terminal_updateOpenGLForegroundVertices(
    ttyr_terminal_GraphicsForeground *Foreground_p, nh_opengl_CommandBuffer *CommandBuffer_p)
{
TTYR_TERMINAL_BEGIN()

    nh_opengl_addCommand(CommandBuffer_p, "glBindVertexArray", Foreground_p->OpenGL.VertexArray_p);

    // Indices.
    nh_opengl_addCommand(CommandBuffer_p, "glBindBuffer",
        nh_opengl_glenum(NULL, GL_ELEMENT_ARRAY_BUFFER), Foreground_p->OpenGL.IndicesBuffer_p);
    Foreground_p->OpenGL.BufferData_p = nh_opengl_addCommand(CommandBuffer_p, "glBufferData",
        nh_opengl_glenum(NULL, GL_ELEMENT_ARRAY_BUFFER),
        nh_opengl_glsizeiptr(NULL, Foreground_p->Indices.length*sizeof(uint32_t)),
        nh_opengl_glubyte(NULL, Foreground_p->Indices.p, Foreground_p->Indices.length*sizeof(uint32_t)),
        nh_opengl_glenum(NULL, GL_STATIC_DRAW));

    // Vertices.
    nh_opengl_addCommand(CommandBuffer_p, "glBindBuffer",
        nh_opengl_glenum(NULL, GL_ARRAY_BUFFER), Foreground_p->OpenGL.VerticesBuffer_p);
    nh_opengl_addCommand(CommandBuffer_p, "glBufferData", 
        nh_opengl_glenum(NULL, GL_ARRAY_BUFFER), 
        nh_opengl_glsizeiptr(NULL, Foreground_p->Vertices.length*sizeof(GLfloat)),
        nh_opengl_glubyte(NULL, Foreground_p->Vertices.p, Foreground_p->Vertices.length*sizeof(GLfloat)),
        nh_opengl_glenum(NULL, GL_STATIC_DRAW));

    nh_opengl_addCommand(CommandBuffer_p, "glEnableVertexAttribArray", nh_opengl_gluint(NULL, 0));
    nh_opengl_addCommand(CommandBuffer_p, "glVertexAttribPointer",
        nh_opengl_gluint(NULL, 0), nh_opengl_gluint(NULL, 3), nh_opengl_glenum(NULL, GL_FLOAT),
        nh_opengl_glboolean(NULL, GL_FALSE), nh_opengl_glsizei(NULL, sizeof(float)*5), 
        nh_opengl_pointer(NULL, NULL));

    nh_opengl_addCommand(CommandBuffer_p, "glEnableVertexAttribArray", nh_opengl_gluint(NULL, 1));
    nh_opengl_addCommand(CommandBuffer_p, "glVertexAttribPointer",
        nh_opengl_gluint(NULL, 1), nh_opengl_gluint(NULL, 2), nh_opengl_glenum(NULL, GL_FLOAT),
        nh_opengl_glboolean(NULL, GL_FALSE), nh_opengl_glsizei(NULL, sizeof(float)*5),
        nh_opengl_glsizei(NULL, sizeof(float)*3));

    nh_opengl_addCommand(CommandBuffer_p, "glBindVertexArray", Foreground_p->OpenGL.VertexArray2_p);

    // Indices.
    nh_opengl_addCommand(CommandBuffer_p, "glBindBuffer",
        nh_opengl_glenum(NULL, GL_ELEMENT_ARRAY_BUFFER), Foreground_p->OpenGL.IndicesBuffer2_p);
    Foreground_p->OpenGL.BufferData2_p = nh_opengl_addCommand(CommandBuffer_p, "glBufferData",
        nh_opengl_glenum(NULL, GL_ELEMENT_ARRAY_BUFFER),
        nh_opengl_glsizeiptr(NULL, Foreground_p->Indices2.length*sizeof(uint32_t)),
        nh_opengl_glubyte(NULL, Foreground_p->Indices2.p, Foreground_p->Indices2.length*sizeof(uint32_t)),
        nh_opengl_glenum(NULL, GL_STATIC_DRAW));
 
    // Vertices.
    nh_opengl_addCommand(CommandBuffer_p, "glBindBuffer",
        nh_opengl_glenum(NULL, GL_ARRAY_BUFFER), Foreground_p->OpenGL.VerticesBuffer2_p);
    nh_opengl_addCommand(CommandBuffer_p, "glBufferData", 
        nh_opengl_glenum(NULL, GL_ARRAY_BUFFER), 
        nh_opengl_glsizeiptr(NULL, Foreground_p->Vertices2.length*sizeof(GLfloat)),
        nh_opengl_glubyte(NULL, Foreground_p->Vertices2.p, Foreground_p->Vertices2.length*sizeof(GLfloat)),
        nh_opengl_glenum(NULL, GL_STATIC_DRAW));

    nh_opengl_addCommand(CommandBuffer_p, "glEnableVertexAttribArray", nh_opengl_gluint(NULL, 0));
    nh_opengl_addCommand(CommandBuffer_p, "glVertexAttribPointer",
        nh_opengl_gluint(NULL, 0), nh_opengl_gluint(NULL, 3), nh_opengl_glenum(NULL, GL_FLOAT),
        nh_opengl_glboolean(NULL, GL_FALSE), nh_opengl_glsizei(NULL, sizeof(float)*3), 
        nh_opengl_pointer(NULL, NULL));

TTYR_TERMINAL_END(TTYR_TERMINAL_SUCCESS)
}

TTYR_TERMINAL_RESULT ttyr_terminal_updateOpenGLForeground(
    void *state_p, void *data_p)
{
TTYR_TERMINAL_BEGIN()

    ttyr_terminal_GraphicsState *State_p = state_p;
    ttyr_terminal_GraphicsData *Data_p = data_p;

    if (Data_p->Foreground.Action.init) {
        ttyr_terminal_initOpenGLFontTexture(&Data_p->Foreground.OpenGL, 
            State_p->Viewport_p->OpenGL.CommandBuffer_p);
        ttyr_terminal_initOpenGLForegroundPrograms(&Data_p->Foreground.OpenGL, 
            State_p->Viewport_p->OpenGL.CommandBuffer_p);
        ttyr_terminal_initOpenGLForegroundVertices(&Data_p->Foreground.OpenGL, 
            State_p->Viewport_p->OpenGL.CommandBuffer_p);
        Data_p->Foreground.Action.init = false;
    }

    ttyr_terminal_updateOpenGLForegroundFont(
        State_p, &Data_p->Foreground, State_p->Viewport_p->OpenGL.CommandBuffer_p);

    ttyr_terminal_updateOpenGLForegroundVertices(
        &Data_p->Foreground, State_p->Viewport_p->OpenGL.CommandBuffer_p);

TTYR_TERMINAL_DIAGNOSTIC_END(TTYR_TERMINAL_SUCCESS)
}

TTYR_TERMINAL_RESULT ttyr_terminal_initOpenGLForeground(
    ttyr_terminal_OpenGLForeground *Foreground_p)
{
TTYR_TERMINAL_BEGIN()

    memset(Foreground_p, 0, sizeof(ttyr_terminal_OpenGLForeground));
 
TTYR_TERMINAL_DIAGNOSTIC_END(TTYR_TERMINAL_SUCCESS)
}

TTYR_TERMINAL_RESULT ttyr_terminal_freeOpenGLForeground(
    ttyr_terminal_OpenGLForeground *Foreground_p)
{
TTYR_TERMINAL_BEGIN()

    nh_opengl_freeCommand(Foreground_p->VertexShader_p);
    nh_opengl_freeCommand(Foreground_p->VertexShader2_p);
    nh_opengl_freeCommand(Foreground_p->FragmentShader_p);
    nh_opengl_freeCommand(Foreground_p->FragmentShader2_p);
    nh_opengl_freeCommand(Foreground_p->Program_p);
    nh_opengl_freeCommand(Foreground_p->Program2_p);
    nh_opengl_freeCommand(Foreground_p->GetUniformLocationTexture_p);
    nh_opengl_freeCommand(Foreground_p->GetUniformLocationColor_p);
    nh_opengl_freeCommand(Foreground_p->GetUniformLocationColor2_p);
 
    nh_opengl_freeData(Foreground_p->Texture_p);
    nh_opengl_freeData(Foreground_p->VertexArray_p);
    nh_opengl_freeData(Foreground_p->VertexArray2_p);
    nh_opengl_freeData(Foreground_p->IndicesBuffer_p);
    nh_opengl_freeData(Foreground_p->IndicesBuffer2_p);
    nh_opengl_freeData(Foreground_p->VerticesBuffer_p);
    nh_opengl_freeData(Foreground_p->VerticesBuffer2_p);

TTYR_TERMINAL_DIAGNOSTIC_END(TTYR_TERMINAL_SUCCESS)
}

