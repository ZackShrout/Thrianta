#include "VulkanObjectShader.h"
#include "Core/Logger.h"
#include "Core/TMemory.h"
#include "Math/MathTypes.h"
#include "Renderer/Vulkan/VulkanShaderUtils.h"
#include "Renderer/Vulkan/VulkanPipeline.h"

#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"

b8 VulkanObjectShaderCreate(vulkan_context* context, vulkan_object_shader* outShader) {
    // Shader module init per stage.
    char stageTypeStrs[OBJECT_SHADER_STAGE_COUNT][5] = {"vert", "frag"};
    VkShaderStageFlagBits stageTypes[OBJECT_SHADER_STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; i++)
    {
        if (!CreateShaderModule(context, BUILTIN_SHADER_NAME_OBJECT, stageTypeStrs[i], stageTypes[i], i, outShader->stages))
        {
            TERROR("Unable to create %s shader module for '%s'.", stageTypeStrs[i], BUILTIN_SHADER_NAME_OBJECT);
            return false;
        }
    }

    // TODO: Descriptors

    // Pipeline creation
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (f32)context->framebufferHeight;
    viewport.width = (f32)context->framebufferWidth;
    viewport.height = -(f32)context->framebufferHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Scissor
    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = context->framebufferWidth;
    scissor.extent.height = context->framebufferHeight;

    // Attributes
    u32 offset = 0;
    const s32 attributeCount = 1;
    VkVertexInputAttributeDescription attributeDesc[attributeCount];
    
    // Position
    VkFormat formats[attributeCount] =
    {
        VK_FORMAT_R32G32B32_SFLOAT
    };
    u64 sizes[attributeCount] =
    {
        sizeof(vec3)
    };

    for (u32 i = 0; i < attributeCount; i++)
    {
        attributeDesc[i].binding = 0;   // binding index - should match binding desc
        attributeDesc[i].location = i;  // attrib location
        attributeDesc[i].format = formats[i];
        attributeDesc[i].offset = offset;
        offset += sizes[i];
    }

    // TODO: Desciptor set layouts.

    // Stages
    // NOTE: Should match the number of shader->stages.
    VkPipelineShaderStageCreateInfo stageCreateInfos[OBJECT_SHADER_STAGE_COUNT];
    TZeroMemory(stageCreateInfos, sizeof(stageCreateInfos));
    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; i++)
    {
        stageCreateInfos[i].sType = outShader->stages[i].shaderStageCreateInfo.sType;
        stageCreateInfos[i] = outShader->stages[i].shaderStageCreateInfo;
    }

    if (!VulkanGraphicsPipelineCreate(
            context,
            &context->mainRenderpass,
            attributeCount,
            attributeDesc,
            0,
            0,
            OBJECT_SHADER_STAGE_COUNT,
            stageCreateInfos,
            viewport,
            scissor,
            false,
            &outShader->pipeline))
    {
        TERROR("Failed to load graphics pipeline for object shader.");
        return false;
    }

    return true;
}

void VulkanObjectShaderDestroy(vulkan_context* context, struct vulkan_object_shader* shader)
{
    VulkanPipelineDestroy(context, &shader->pipeline);
    
    // Destroy shader modules.
    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; i++)
    {
        vkDestroyShaderModule(context->device.logicalDevice, shader->stages[i].handle, context->allocator);
        shader->stages[i].handle = 0;
    }
}

void VulkanObjectShaderUse(vulkan_context* context, struct vulkan_object_shader* shader)
{

}