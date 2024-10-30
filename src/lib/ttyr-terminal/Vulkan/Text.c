// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES =======================================================================================

#include "Text.h"

#include "../Common/Macros.h"

#include "nh-core/Util/Array.h"
#include "nh-gfx/Vulkan/Texture.h"
#include "nh-gfx/Common/Macros.h"

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>
#include <float.h>

// FUNCTIONS =======================================================================================

static TTYR_TERMINAL_RESULT ttyr_terminal_createTextVertices(
    nh_gfx_VulkanDriver *Driver_p, ttyr_terminal_VulkanText *Text_p, nh_core_Array *Vertices_p, nh_core_Array *Indices_p)
{
    VkBufferCreateInfo BufferCreateInfo = 
    {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = Vertices_p->length * sizeof(float),
        .usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    nh_gfx_VulkanBufferInfo BufferInfo = {
        .Info_p           = &BufferCreateInfo,
        .data_p           = Vertices_p->p,
        .mapMemory        = true,
        .memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        .createInfo       = false,
    };

    NH_CORE_CHECK_2(TTYR_TERMINAL_ERROR_BAD_STATE, nh_gfx_createVulkanBuffer(
        Driver_p, &BufferInfo, &Text_p->VertexBuffer
    ))

    BufferCreateInfo.size  = Indices_p->length * sizeof(uint32_t);
    BufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    BufferInfo.data_p = Indices_p->p;
    BufferInfo.Info_p = &BufferCreateInfo;
   
    NH_CORE_CHECK_2(TTYR_TERMINAL_ERROR_BAD_STATE, nh_gfx_createVulkanBuffer(
        Driver_p, &BufferInfo, &Text_p->IndexBuffer 
    ))
 
    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_createTextUniform(
    nh_gfx_VulkanDriver *Driver_p, ttyr_terminal_VulkanText *Text_p)
{
    // MVP
    float vsData_p[16];
    for (int i = 0; i < 16; ++i) {vsData_p[i] = 0.0;}
    vsData_p[0] = vsData_p[5] = vsData_p[10] = vsData_p[15] = 1.0;
    
    VkBufferCreateInfo VSInfo = 
    {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext       = VK_NULL_HANDLE,
        .size        = sizeof(float) * 16,  // MVP
        .usage       = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,  
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    nh_gfx_VulkanBufferInfo BufferInfo =
    {
        .Info_p           = &VSInfo,
        .data_p           = vsData_p,
        .mapMemory        = true,
        .memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        .createInfo       = false,
    };

    NH_CORE_CHECK_2(TTYR_TERMINAL_ERROR_BAD_STATE, nh_gfx_createVulkanBuffer(
        Driver_p, &BufferInfo, &Text_p->VertShaderBuffer
    ))

    // text color, etc.
    float fsData_p[11];
    fsData_p[0]  = 0.0f;
    fsData_p[1]  = 1.0f;
    fsData_p[2]  = 0.0f;
    fsData_p[3]  = 1.0f;
    fsData_p[10] = 10.0f;

    for (int i = 4; i < 10; ++i) {fsData_p[i] = 0.0f;}

    VkBufferCreateInfo FSInfo = 
    {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext       = VK_NULL_HANDLE,
        .size        = sizeof(float) * 11,
        .usage       = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    BufferInfo.data_p = fsData_p;
    BufferInfo.Info_p = &FSInfo;

    NH_CORE_CHECK_2(TTYR_TERMINAL_ERROR_BAD_STATE, nh_gfx_createVulkanBuffer(
        Driver_p, &BufferInfo, &Text_p->FragShaderBuffer
    ))

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_createTextDescriptor(
    nh_gfx_VulkanGPU *GPU_p, ttyr_terminal_VulkanText *Text_p)
{
    nh_gfx_VulkanDriver *Driver_p = &GPU_p->Driver;

    // create
    VkDescriptorSetLayoutBinding LayoutBindings_p[3];
    VkDescriptorSetLayoutBinding LayoutBinding1 =
    {
        .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .stageFlags      = VK_SHADER_STAGE_VERTEX_BIT,
        .binding         = 0,
        .descriptorCount = 1 
    };
    VkDescriptorSetLayoutBinding LayoutBinding2 =
    {
        .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
        .binding         = 1,
        .descriptorCount = 1 
    };
    VkDescriptorSetLayoutBinding LayoutBinding3 =
    {
        .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
        .binding         = 2,
        .descriptorCount = 1 
    };
    LayoutBindings_p[0] = LayoutBinding1;
    LayoutBindings_p[1] = LayoutBinding2;
    LayoutBindings_p[2] = LayoutBinding3;

    VkDescriptorSetLayoutCreateInfo LayoutInfo = 
    {
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pBindings    = LayoutBindings_p,
        .bindingCount = 3
    };
    VkDescriptorSetAllocateInfo AllocateInfo = 
    {
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool     = Driver_p->DescriptorPool_p[0],
        .descriptorSetCount = 1,
    };

    NH_CORE_CHECK_2(TTYR_TERMINAL_ERROR_BAD_STATE, nh_gfx_createVulkanDescriptorSet(Driver_p, &AllocateInfo, &LayoutInfo, &Text_p->DescriptorSet))

    return TTYR_TERMINAL_SUCCESS;
}

static TTYR_TERMINAL_RESULT ttyr_terminal_updateTextDescriptor(
    nh_gfx_VulkanGPU *GPU_p, ttyr_terminal_VulkanText *Text_p, nh_vk_Texture *Texture_p)
{
    nh_gfx_VulkanDriver *Driver_p = &GPU_p->Driver;

    // update
    VkDescriptorBufferInfo vsDescrBufferInfo = 
    {
        .buffer = Text_p->VertShaderBuffer.Buffer,
        .offset = 0,
        .range  = sizeof(float) * 16
    };
    VkDescriptorBufferInfo fsDescrBufferInfo = 
    {
        .buffer = Text_p->FragShaderBuffer.Buffer,
        .offset = 0,
        .range  = sizeof(float) * 11
    };        
    VkDescriptorImageInfo ImageInfo = 
    {
        .sampler     = Texture_p->Sampler,
        .imageView   = Texture_p->ImageView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };
    VkWriteDescriptorSet vsUniformBufferDescriptorSet = 
    {
        .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet          = Text_p->DescriptorSet.DescriptorSet,
        .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .dstBinding      = 0,
        .pBufferInfo     = &vsDescrBufferInfo,
        .descriptorCount = 1
    };
    VkWriteDescriptorSet fsTextureSamplerDescriptorSet = 
    {
        .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet          = Text_p->DescriptorSet.DescriptorSet,
        .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .dstBinding      = 1,
        .pImageInfo      = &ImageInfo,
        .descriptorCount = 1
    };
    VkWriteDescriptorSet fsUniformBufferDescriptorSet = 
    {
        .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet          = Text_p->DescriptorSet.DescriptorSet,
        .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .dstBinding      = 2,
        .pBufferInfo     = &fsDescrBufferInfo,
        .descriptorCount = 1
    };

    VkWriteDescriptorSet Sets_p[3] = {vsUniformBufferDescriptorSet, fsTextureSamplerDescriptorSet, fsUniformBufferDescriptorSet};
    Driver_p->Functions.vkUpdateDescriptorSets(Driver_p->Device, 3, Sets_p, 0, VK_NULL_HANDLE);

    return TTYR_TERMINAL_SUCCESS;
}

TTYR_TERMINAL_RESULT ttyr_terminal_initVulkanText(
    nh_gfx_VulkanGPU *GPU_p, ttyr_terminal_VulkanText *Text_p)
{
    Text_p->destroy = false;
    TTYR_TERMINAL_CHECK(ttyr_terminal_createTextDescriptor(GPU_p, Text_p))

    return TTYR_TERMINAL_SUCCESS;
}

TTYR_TERMINAL_RESULT ttyr_terminal_updateVulkanText(
    nh_gfx_VulkanGPU *GPU_p, nh_gfx_FontInstance *FontInstance_p, ttyr_terminal_VulkanText *Text_p, nh_core_Array *Vertices_p, 
    nh_core_Array *Indices_p)
{
    nh_vk_Texture *Texture_p = nh_vk_reloadFontTexture(GPU_p, FontInstance_p->Font_p);

    if (Text_p->destroy) {
        nh_gfx_destroyVulkanBuffer(&GPU_p->Driver, &Text_p->VertexBuffer);
        nh_gfx_destroyVulkanBuffer(&GPU_p->Driver, &Text_p->IndexBuffer);
        nh_gfx_destroyVulkanBuffer(&GPU_p->Driver, &Text_p->FragShaderBuffer);
        nh_gfx_destroyVulkanBuffer(&GPU_p->Driver, &Text_p->VertShaderBuffer);
    }

    TTYR_TERMINAL_CHECK(ttyr_terminal_createTextVertices(&GPU_p->Driver, Text_p, Vertices_p, Indices_p))
    TTYR_TERMINAL_CHECK(ttyr_terminal_createTextUniform(&GPU_p->Driver, Text_p))
    TTYR_TERMINAL_CHECK(ttyr_terminal_updateTextDescriptor(GPU_p, Text_p, Texture_p))

    Text_p->destroy = true;

    return TTYR_TERMINAL_SUCCESS;
}

//TTYR_TERMINAL_RESULT ttyr_terminal_destroyVulkanText(
//    nh_gfx_VulkanGPU *GPU_p, ttyr_terminal_VulkanText *Text_p)
//{
//    nh_gfx_destroyVulkanBuffer(&GPU_p->Driver, &Text_p->VertexBuffer);
//    nh_gfx_destroyVulkanBuffer(&GPU_p->Driver, &Text_p->IndexBuffer);
//    nh_gfx_destroyVulkanBuffer(&GPU_p->Driver, &Text_p->FragShaderBuffer);
//    nh_gfx_destroyVulkanBuffer(&GPU_p->Driver, &Text_p->VertShaderBuffer);
////    nh_vk_destroyDescriptorSet(&GPU_p->Driver, &Text_p->DescriptorSet, &(GPU_p->Driver.DescriptorPool_p[0])); 
//
//return NH_SUCCESS;
//}
