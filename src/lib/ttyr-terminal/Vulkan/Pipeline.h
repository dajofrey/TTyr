#ifndef TTYR_TERMINAL_VULKAN_PIPELINE_H
#define TTYR_TERMINAL_VULKAN_PIPELINE_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "../Common/Includes.h"

#include "nhgfx/Vulkan/Driver.h"
#include "nhgfx/Vulkan/Helper.h"

#endif

/** @addtogroup lib_nhcss_enums
 *  @{
 */

    typedef enum TTYR_TERMINAL_VULKAN_PIPELINE {
        TTYR_TERMINAL_VULKAN_PIPELINE_SDF, 
       _TTYR_TERMINAL_VULKAN_PIPELINE_COUNT,     
    } TTYR_TERMINAL_VULKAN_PIPELINE;

/** @} */

/** @addtogroup lib_nhcss_functions
 *  @{
 */

    TTYR_TERMINAL_RESULT ttyr_terminal_createVulkanPipelines(
        nh_vk_Driver *Driver_p, nh_vk_Pipeline *Pipelines_p
    );
    
    void ttyr_terminal_destroyVulkanPipelines(
        nh_vk_Driver *Driver_p, nh_vk_Pipeline *Pipelines_p
    );

/** @} */

#endif 
