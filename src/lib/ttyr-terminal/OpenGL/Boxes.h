#ifndef TTYR_TERMINAL_OPENGL_BOXES_H
#define TTYR_TERMINAL_OPENGL_BOXES_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "../Common/Includes.h"

#include "nhgfx/OpenGL/CommandBuffer.h"
#include "nhgfx/OpenGL/Commands.h"

#endif

/** @addtogroup lib_nhterminal_structs
 *  @{
 */

    typedef struct ttyr_terminal_OpenGLBoxes {
        nh_opengl_Data *VertexArray_p;
        nh_opengl_Data *VerticesBuffer_p;
        nh_opengl_Command *BufferData_p;
    } ttyr_terminal_OpenGLBoxes;

/** @} */

/** @addtogroup lib_nhterminal_functions
 *  @{
 */

    TTYR_TERMINAL_RESULT ttyr_terminal_updateOpenGLBoxes(
        void *state_p, void *data_p
    );

    TTYR_TERMINAL_RESULT ttyr_terminal_initOpenGLBoxes(
        ttyr_terminal_OpenGLBoxes *Boxes_p
    );

    TTYR_TERMINAL_RESULT ttyr_terminal_freeOpenGLBoxes(
        ttyr_terminal_OpenGLBoxes *Boxes_p
    );

/** @} */

#endif 
