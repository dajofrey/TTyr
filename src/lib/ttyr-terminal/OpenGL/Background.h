#ifndef TTYR_TERMINAL_OPENGL_BACKGROUND_H
#define TTYR_TERMINAL_OPENGL_BACKGROUND_H

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

    typedef struct ttyr_terminal_OpenGLBackground {
        nh_gfx_OpenGLCommand *VertexShader_p;
        nh_gfx_OpenGLCommand *FragmentShader_p;
        nh_gfx_OpenGLCommand *Program_p;
        nh_gfx_OpenGLData *VertexArray_p;
        nh_gfx_OpenGLData *IndicesBuffer_p;
        nh_gfx_OpenGLData *VerticesBuffer_p;
        nh_gfx_OpenGLCommand *BufferData_p;
        nh_gfx_OpenGLCommand *GetUniformLocation_p;
    } ttyr_terminal_OpenGLBackground;

/** @} */

/** @addtogroup lib_nhterminal_functions
 *  @{
 */

    TTYR_TERMINAL_RESULT ttyr_terminal_updateOpenGLBackground(
        void *state_p, void *data_p
    );

    TTYR_TERMINAL_RESULT ttyr_terminal_initOpenGLBackground(
        ttyr_terminal_OpenGLBackground *Background_p
    );

    TTYR_TERMINAL_RESULT ttyr_terminal_freeOpenGLBackground(
        ttyr_terminal_OpenGLBackground *Background_p
    );

/** @} */

#endif 
