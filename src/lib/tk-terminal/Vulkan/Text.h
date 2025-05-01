#ifndef TTYR_TERMINAL_VULKAN_TEXT_H
#define TTYR_TERMINAL_VULKAN_TEXT_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "../Common/Includes.h"

#include "nh-gfx/Vulkan/GPU.h"
#include "nh-gfx/Vulkan/Helper.h"
#include "nh-gfx/Fonts/FontManager.h"

#include <stdint.h>

#endif

/** @addtogroup lib_nhterminal_structs
 *  @{
 */

    typedef struct tk_terminal_VulkanText {
        bool destroy;
        nh_gfx_VulkanBuffer VertexBuffer;
        nh_gfx_VulkanBuffer IndexBuffer;
        nh_gfx_VulkanBuffer VertShaderBuffer;
        nh_gfx_VulkanBuffer FragShaderBuffer;
        nh_gfx_VulkanDescriptorSet DescriptorSet;
    } tk_terminal_VulkanText;

/** @} */

/** @addtogroup lib_nh-css_functions
 *  @{
 */

    TTYR_TERMINAL_RESULT tk_terminal_initVulkanText(
        nh_gfx_VulkanGPU *GPU_p, tk_terminal_VulkanText *Text_p
    );

    TTYR_TERMINAL_RESULT tk_terminal_updateVulkanText(
        nh_gfx_VulkanGPU *GPU_p, nh_gfx_FontInstance *FontInstance_p, tk_terminal_VulkanText *Text_p, nh_core_Array *Vertices_p, 
        nh_core_Array *Indices_p
    );

/** @} */

#endif 
