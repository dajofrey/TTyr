// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Render.h"
#include "Text.h"
#include "Pipeline.h"

#include "../Common/Macros.h"
#include "../Common/Config.h"

#include "nhgfx/Base/Viewport.h"
#include "nhgfx/Vulkan/GPU.h"

#include <stdlib.h>
#include <string.h>

// TEXT ============================================================================================

static void ttyr_terminal_recordText(
    ttyr_terminal_VulkanText *Text_p, nh_vk_GPU *GPU_p, VkCommandBuffer *CommandBuffer_p, int indices)
{
TTYR_TERMINAL_BEGIN()

    GPU_p->Driver.Functions.vkCmdBindPipeline(
        *CommandBuffer_p, VK_PIPELINE_BIND_POINT_GRAPHICS, 
        GPU_p->Term.Pipelines_p[TTYR_TERMINAL_VULKAN_PIPELINE_SDF].Pipeline
    );
    VkDeviceSize size_p[1] = {0};

    GPU_p->Driver.Functions.vkCmdBindVertexBuffers(
        *CommandBuffer_p, 0, 1, &Text_p->VertexBuffer.Buffer, size_p
    );
    
    GPU_p->Driver.Functions.vkCmdBindDescriptorSets(
        *CommandBuffer_p, VK_PIPELINE_BIND_POINT_GRAPHICS,
        GPU_p->Term.Pipelines_p[TTYR_TERMINAL_VULKAN_PIPELINE_SDF].PipelineLayout,
        0, 1, &Text_p->DescriptorSet.DescriptorSet, 0, VK_NULL_HANDLE
    );

    GPU_p->Driver.Functions.vkCmdBindIndexBuffer(
        *CommandBuffer_p, Text_p->IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32
    );

    GPU_p->Driver.Functions.vkCmdDrawIndexed(
        *CommandBuffer_p, indices, 1, 0, 0, 0
    );

TTYR_TERMINAL_SILENT_END()
}

// RENDER ==========================================================================================

TTYR_TERMINAL_RESULT ttyr_terminal_renderUsingVulkan(
    ttyr_terminal_Graphics *Graphics_p)
{
TTYR_TERMINAL_BEGIN()

    ttyr_terminal_Config Config = ttyr_terminal_getConfig();
 
    nh_gfx_FontInstance *FontInstance_p = nh_gfx_claimFontInstance(
        Graphics_p->State.Fonts.pp[Graphics_p->State.font], Config.fontSize
    );

    TTYR_TERMINAL_CHECK(ttyr_terminal_updateVulkanText(
        Graphics_p->State.Viewport_p->Surface_p->Vulkan.GPU_p, FontInstance_p, &Graphics_p->Data1.Foreground.Vulkan, 
        &Graphics_p->Data1.Foreground.Vertices, &Graphics_p->Data1.Foreground.Indices
    ))

    nh_gfx_beginRecording(Graphics_p->State.Viewport_p);

    for (int i = 0; i < Graphics_p->State.Viewport_p->Vulkan.images; ++i) {
        ttyr_terminal_recordText(
            &Graphics_p->Data1.Foreground.Vulkan, Graphics_p->State.Viewport_p->Surface_p->Vulkan.GPU_p,
            Graphics_p->State.Viewport_p->Vulkan.CommandBuffers_pp[i], Graphics_p->Data1.Foreground.Indices.length 
        );
    }

    nh_gfx_endRecording(Graphics_p->State.Viewport_p, NH_FALSE);

TTYR_TERMINAL_END(TTYR_TERMINAL_SUCCESS)
}

