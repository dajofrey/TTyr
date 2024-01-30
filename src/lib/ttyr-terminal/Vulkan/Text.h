#ifndef TTYR_TERMINAL_VULKAN_TEXT_H
#define TTYR_TERMINAL_VULKAN_TEXT_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "../Common/Includes.h"

#include "nhgfx/Vulkan/GPU.h"
#include "nhgfx/Vulkan/Helper.h"
#include "nhgfx/Fonts/FontManager.h"

#include <stdint.h>

#endif

/** @addtogroup lib_nhterminal_structs
 *  @{
 */

    typedef struct ttyr_terminal_VulkanText {
        NH_BOOL destroy;
        nh_vk_Buffer VertexBuffer;
        nh_vk_Buffer IndexBuffer;
        nh_vk_Buffer VertShaderBuffer;
        nh_vk_Buffer FragShaderBuffer;
        nh_vk_DescriptorSet DescriptorSet;
    } ttyr_terminal_VulkanText;

/** @} */

/** @addtogroup lib_nhcss_functions
 *  @{
 */

    TTYR_TERMINAL_RESULT ttyr_terminal_initVulkanText(
        nh_vk_GPU *GPU_p, ttyr_terminal_VulkanText *Text_p
    );

    TTYR_TERMINAL_RESULT ttyr_terminal_updateVulkanText(
        nh_vk_GPU *GPU_p, nh_gfx_FontInstance *FontInstance_p, ttyr_terminal_VulkanText *Text_p, nh_Array *Vertices_p, 
        nh_Array *Indices_p
    );

/** @} */

#endif 
