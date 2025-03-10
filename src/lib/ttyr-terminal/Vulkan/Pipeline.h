#ifndef TTYR_TERMINAL_VULKAN_PIPELINE_H
#define TTYR_TERMINAL_VULKAN_PIPELINE_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "../Common/Includes.h"

#include "nh-gfx/Vulkan/Driver.h"
#include "nh-gfx/Vulkan/Helper.h"

#endif

/** @addtogroup lib_nh-css_enums
 *  @{
 */

    typedef enum TTYR_TERMINAL_VULKAN_PIPELINE {
        TTYR_TERMINAL_VULKAN_PIPELINE_SDF, 
       _TTYR_TERMINAL_VULKAN_PIPELINE_COUNT,     
    } TTYR_TERMINAL_VULKAN_PIPELINE;

/** @} */

/** @addtogroup lib_nh-css_functions
 *  @{
 */

    TTYR_TERMINAL_RESULT ttyr_terminal_createVulkanPipelines(
        nh_gfx_VulkanDriver *Driver_p, nh_gfx_VulkanPipeline *Pipelines_p
    );
    
    void ttyr_terminal_destroyVulkanPipelines(
        nh_gfx_VulkanDriver *Driver_p, nh_gfx_VulkanPipeline *Pipelines_p
    );

/** @} */

#endif 
