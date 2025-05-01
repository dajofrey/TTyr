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

typedef struct tk_terminal_Graphics tk_terminal_Graphics;

#endif

/** @addtogroup lib_nhterminal_structs
 *  @{
 */

    typedef struct tk_terminal_OpenGLDim {
        nh_gfx_OpenGLCommand *VertexShader_p;
        nh_gfx_OpenGLCommand *FragmentShader_p;
        nh_gfx_OpenGLCommand *Program_p;
        nh_gfx_OpenGLData *VertexArray_p;
        nh_gfx_OpenGLData *VerticesBuffer_p;
        nh_gfx_OpenGLData *ColorBuffer_p;
    } tk_terminal_OpenGLDim;

/** @} */

/** @addtogroup lib_nhterminal_functions
 *  @{
 */

    TTYR_TERMINAL_RESULT tk_terminal_updateOpenGLDim(
        void *state_p, void *data_p
    );

    TTYR_TERMINAL_RESULT tk_terminal_initOpenGLDim(
        tk_terminal_OpenGLDim *Dim_p
    );

    TTYR_TERMINAL_RESULT tk_terminal_freeOpenGLDim(
        tk_terminal_OpenGLDim *Dim_p
    );

/** @} */

#endif 
