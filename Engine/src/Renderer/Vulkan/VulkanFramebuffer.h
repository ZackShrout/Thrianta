#pragma once
#include "VulkanTypes.inl"

void VulkanFramebufferCreate(
    vulkan_context* context,
    vulkan_renderpass* renderpass,
    u32 width,
    u32 height,
    u32 attachmentCount,
    VkImageView* attachments,
    vulkan_framebuffer* outFramebuffer);

void VulkanFramebufferDestroy(vulkan_context* context, vulkan_framebuffer* framebuffer);