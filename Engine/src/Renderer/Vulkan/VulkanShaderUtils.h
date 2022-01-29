#pragma once

#include "VulkanTypes.inl"

b8 CreateShaderModule(
    vulkan_context* context,
    const char* name,
    const char* typeStr,
    VkShaderStageFlagBits shaderStageFlag,
    u32 stageIndex,
    vulkan_shader_stage* shaderStages
);