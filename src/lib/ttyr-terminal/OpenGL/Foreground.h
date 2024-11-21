#ifndef TTYR_TERMINAL_OPENGL_FOREGROUND_H
#define TTYR_TERMINAL_OPENGL_FOREGROUND_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "../Common/Includes.h"

#include "nh-gfx/OpenGL/CommandBuffer.h"
#include "nh-gfx/OpenGL/Commands.h"

#include "nh-gfx/Fonts/FontManager.h"

#include <stdint.h>

typedef struct ttyr_terminal_Graphics ttyr_terminal_Graphics;

#endif

/** @addtogroup lib_nhterminal_structs
 *  @{
 */

    typedef struct ttyr_terminal_OpenGLForeground {
        nh_gfx_OpenGLCommand *VertexShader_p;
        nh_gfx_OpenGLCommand *FragmentShader_p;
        nh_gfx_OpenGLCommand *Program_p;
        nh_gfx_OpenGLCommand *VertexShader2_p;
        nh_gfx_OpenGLCommand *FragmentShader2_p;
        nh_gfx_OpenGLCommand *Program2_p;
        nh_gfx_OpenGLData *Texture_p;
        nh_gfx_OpenGLData *VertexArray_p;
        nh_gfx_OpenGLData *VertexArray2_p;
        nh_gfx_OpenGLData *IndicesBuffer_p;
        nh_gfx_OpenGLData *IndicesBuffer2_p;
        nh_gfx_OpenGLData *VerticesBuffer_p;
        nh_gfx_OpenGLData *VerticesBuffer2_p;
        nh_gfx_OpenGLCommand *BufferData_p;
        nh_gfx_OpenGLCommand *BufferData2_p;
        nh_gfx_OpenGLCommand *GetUniformLocationTexture_p;
        nh_gfx_OpenGLCommand *GetUniformLocationColor_p;
        nh_gfx_OpenGLCommand *GetUniformLocationColor2_p;
    } ttyr_terminal_OpenGLForeground;

/** @} */

/** @addtogroup lib_nhterminal_functions
 *  @{
 */

    TTYR_TERMINAL_RESULT ttyr_terminal_updateOpenGLForeground(
        void *state_p, void *data_p
    );

    TTYR_TERMINAL_RESULT ttyr_terminal_initOpenGLForeground(
        ttyr_terminal_OpenGLForeground *Text_p
    );

    TTYR_TERMINAL_RESULT ttyr_terminal_freeOpenGLForeground(
        ttyr_terminal_OpenGLForeground *Foreground_p
    );

/** @} */

#endif 
