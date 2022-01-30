#pragma once
#include "VulkanTypes.inl"

b8 VulkanGraphicsPipelineCreate(
    vulkan_context* context,
    vulkan_renderpass* renderpass,
    u32 attributeCount,
    VkVertexInputAttributeDescription* attributes,
    u32 descSetLayoutCount,
    VkDescriptorSetLayout* descSetLayouts,
    u32 stageCount,
    VkPipelineShaderStageCreateInfo* stages,
    VkViewport viewport,
    VkRect2D scissor,
    b8 isWireframe,
    vulkan_pipeline* outPipeline);
void VulkanPipelineDestroy(vulkan_context* context, vulkan_pipeline* pipeline);
void VulkanPipelineBind(vulkan_command_buffer* commandBuffer, VkPipelineBindPoint bindPoint, vulkan_pipeline* pipeline);