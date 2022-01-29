#include "VulkanObjectShader.h"
#include "Core/Logger.h"
#include "Renderer/Vulkan/VulkanShaderUtils.h"

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

    // Descriptors

    return true;
}

void VulkanObjectShaderDestroy(vulkan_context* context, struct vulkan_object_shader* shader)
{

}

void VulkanObjectShaderUse(vulkan_context* context, struct vulkan_object_shader* shader)
{

}