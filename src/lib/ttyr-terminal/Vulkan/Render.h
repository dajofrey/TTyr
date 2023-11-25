#ifndef TTYR_TERMINAL_VULKAN_RENDER_H
#define TTYR_TERMINAL_VULKAN_RENDER_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "../Terminal/Graphics.h"
#include "../Common/Includes.h"

#endif

/** @addtogroup lib_nhcss_functions 
 *  @{
 */

    TTYR_TERMINAL_RESULT ttyr_terminal_renderUsingVulkan(
        ttyr_terminal_Graphics *Graphics_p
    );

/** @} */

#endif
