// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Initialize.h"
#include "Macros.h"

#include "../Vulkan/Pipeline.h"

#include "nh-gfx/Vulkan/GPU.h"
#include "nh-gfx/Vulkan/Vulkan.h"

#include "nh-core/Config/Config.h"
#include "nh-core/System/Memory.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// DEBUG ===========================================================================================

TK_TERMINAL_RESULT tk_terminal_initialize()
{
    for (int i = 0; i < NH_VULKAN.GPUs.size; ++i) 
    {
        nh_gfx_VulkanGPU *GPU_p = NH_VULKAN.GPUs.pp[0];

        GPU_p->Term.Pipelines_p = (nh_gfx_VulkanPipeline*)nh_core_allocate(sizeof(nh_gfx_VulkanPipeline) * _TK_TERMINAL_VULKAN_PIPELINE_COUNT);
        TK_TERMINAL_CHECK_MEM(GPU_p->Term.Pipelines_p)
 
        TK_TERMINAL_CHECK(tk_terminal_createVulkanPipelines(&GPU_p->Driver, GPU_p->Term.Pipelines_p))
    }

    return TK_TERMINAL_SUCCESS;
}
