#pragma once
#include "VulkanTypes.inl"

void VulkanImageCreate(
    vulkan_context* context,
    VkImageType imageType,
    u32 width,
    u32 height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags memoryFlags,
    b32 createView,
    VkImageAspectFlags viewAspectFlags,
    vulkan_image* outImage);
void VulkanImageViewCreate(
    vulkan_context* context,
    VkFormat format,
    vulkan_image* image,
    VkImageAspectFlags aspectFlags);
/**
 * Transitions the provided image from oldLayout to newLayout.
 */
void VulkanImageTransitionLayout(
    vulkan_context* context,
    vulkan_command_buffer* cmdBuffer,
    vulkan_image* image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout);
/**
 * Copies data in buffer to provided image.
 * @param context The Vulkan context.
 * @param image The image to copy the buffer's data to.
 * @param buffer The buffer whose data will be copied.
 */
void VulkanImageCopyFromBuffer(
    vulkan_context* context,
    vulkan_image* image,
    VkBuffer buffer,
    vulkan_command_buffer* cmdBuffer);
void VulkanImageDestroy(vulkan_context* context, vulkan_image* image);