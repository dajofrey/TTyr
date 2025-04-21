#ifndef TTYR_TERMINAL_OPENGL_DIM_H
#define TTYR_TERMINAL_OPENGL_DIM_H

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

    typedef struct ttyr_terminal_OpenGLDim {
        nh_gfx_OpenGLCommand *VertexShader_p;
        nh_gfx_OpenGLCommand *FragmentShader_p;
        nh_gfx_OpenGLCommand *Program_p;
        nh_gfx_OpenGLData *VertexArray_p;
        nh_gfx_OpenGLData *VerticesBuffer_p;
        nh_gfx_OpenGLData *ColorBuffer_p;
    } ttyr_terminal_OpenGLDim;

/** @} */

/** @addtogroup lib_nhterminal_functions
 *  @{
 */

    TTYR_TERMINAL_RESULT ttyr_terminal_updateOpenGLDim(
        void *state_p, void *data_p
    );

    TTYR_TERMINAL_RESULT ttyr_terminal_initOpenGLDim(
        ttyr_terminal_OpenGLDim *Dim_p
    );

    TTYR_TERMINAL_RESULT ttyr_terminal_freeOpenGLDim(
        ttyr_terminal_OpenGLDim *Dim_p
    );

/** @} */

#endif 
