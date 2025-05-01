#ifndef TTYR_TERMINAL_OPENGL_BOXES_H
#define TTYR_TERMINAL_OPENGL_BOXES_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "../Common/Includes.h"

#include "nh-gfx/OpenGL/CommandBuffer.h"
#include "nh-gfx/OpenGL/Commands.h"

#endif

/** @addtogroup lib_nhterminal_structs
 *  @{
 */

    typedef struct tk_terminal_OpenGLBoxes {
        nh_gfx_OpenGLData *VertexArray_p;
        nh_gfx_OpenGLData *VerticesBuffer_p;
        nh_gfx_OpenGLData *ColorBuffer_p;
        nh_gfx_OpenGLCommand *BufferData_p;
    } tk_terminal_OpenGLBoxes;

/** @} */

/** @addtogroup lib_nhterminal_functions
 *  @{
 */

    TTYR_TERMINAL_RESULT tk_terminal_updateOpenGLBoxes(
        void *state_p, void *data_p
    );

    TTYR_TERMINAL_RESULT tk_terminal_initOpenGLBoxes(
        tk_terminal_OpenGLBoxes *Boxes_p
    );

    TTYR_TERMINAL_RESULT tk_terminal_freeOpenGLBoxes(
        tk_terminal_OpenGLBoxes *Boxes_p
    );

/** @} */

#endif 
