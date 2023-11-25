#ifndef TTYR_TERMINAL_OPENGL_FOREGROUND_H
#define TTYR_TERMINAL_OPENGL_FOREGROUND_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "../Common/Includes.h"

#include "../../../../external/Netzhaut/src/lib/nhgfx/OpenGL/CommandBuffer.h"
#include "../../../../external/Netzhaut/src/lib/nhgfx/OpenGL/Commands.h"

#include "../../../../external/Netzhaut/src/lib/nhgfx/Fonts/FontManager.h"

#include <stdint.h>

typedef struct ttyr_terminal_Graphics ttyr_terminal_Graphics;

#endif

/** @addtogroup lib_nhterminal_structs
 *  @{
 */

    typedef struct ttyr_terminal_OpenGLForeground {
        nh_opengl_Command *VertexShader_p;
        nh_opengl_Command *FragmentShader_p;
        nh_opengl_Command *Program_p;
        nh_opengl_Command *VertexShader2_p;
        nh_opengl_Command *FragmentShader2_p;
        nh_opengl_Command *Program2_p;
        nh_opengl_Data *Texture_p;
        nh_opengl_Data *VertexArray_p;
        nh_opengl_Data *VertexArray2_p;
        nh_opengl_Data *IndicesBuffer_p;
        nh_opengl_Data *IndicesBuffer2_p;
        nh_opengl_Data *VerticesBuffer_p;
        nh_opengl_Data *VerticesBuffer2_p;
        nh_opengl_Command *BufferData_p;
        nh_opengl_Command *BufferData2_p;
        nh_opengl_Command *GetUniformLocationTexture_p;
        nh_opengl_Command *GetUniformLocationColor_p;
        nh_opengl_Command *GetUniformLocationColor2_p;
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
