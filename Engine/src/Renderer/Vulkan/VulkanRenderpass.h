#pragma once
#include "VulkanTypes.inl"

void VulkanRenderpassCreate(
    vulkan_context* context, 
    vulkan_renderpass* outRenderpass,
    f32 x, f32 y, f32 w, f32 h,
    f32 r, f32 g, f32 b, f32 a,
    f32 depth,
    u32 stencil);

void VulkanRenderpassDestroy(vulkan_context* context, vulkan_renderpass* renderpass);

void VulkanRenderpassBegin(
    vulkan_command_buffer* cmdBuffer, 
    vulkan_renderpass* renderpass,
    VkFramebuffer frameBuffer);

void VulkanRenderpassEnd(vulkan_command_buffer* cmdBuffer, vulkan_renderpass* renderpass);