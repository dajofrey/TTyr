// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Initialize.h"
#include "Macros.h"
#include "Data/ttyr-terminal.conf.inc"

#include "../Vulkan/Pipeline.h"

#include "../../../../external/Netzhaut/src/lib/nhgfx/Vulkan/GPU.h"
#include "../../../../external/Netzhaut/src/lib/nhgfx/Vulkan/Vulkan.h"

#include "../../../../external/Netzhaut/src/lib/nhcore/Config/Config.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/System/Logger.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/System/Memory.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// DEBUG ===========================================================================================

TTYR_TERMINAL_RESULT ttyr_terminal_initialize()
{
TTYR_TERMINAL_BEGIN()

    nh_core_appendConfig(ttyr_terminal_conf_inc, ttyr_terminal_conf_inc_len, NH_FALSE);

    for (int i = 0; i < NH_VULKAN.GPUs.size; ++i) 
    {
        nh_vk_GPU *GPU_p = NH_VULKAN.GPUs.pp[0];

        GPU_p->Term.Pipelines_p = nh_core_allocate(sizeof(nh_vk_Pipeline) * _TTYR_TERMINAL_VULKAN_PIPELINE_COUNT);
        TTYR_TERMINAL_CHECK_MEM(GPU_p->Term.Pipelines_p)
 
        TTYR_TERMINAL_CHECK(ttyr_terminal_createVulkanPipelines(&GPU_p->Driver, GPU_p->Term.Pipelines_p))
    }

TTYR_TERMINAL_DIAGNOSTIC_END(TTYR_TERMINAL_SUCCESS)
}

