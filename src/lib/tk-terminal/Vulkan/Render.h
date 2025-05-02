#ifndef TK_TERMINAL_VULKAN_RENDER_H
#define TK_TERMINAL_VULKAN_RENDER_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "../Terminal/Graphics.h"
#include "../Common/Includes.h"

#endif

/** @addtogroup lib_nh-css_functions 
 *  @{
 */

    TK_TERMINAL_RESULT tk_terminal_renderUsingVulkan(
        tk_terminal_Graphics *Graphics_p
    );

/** @} */

#endif
