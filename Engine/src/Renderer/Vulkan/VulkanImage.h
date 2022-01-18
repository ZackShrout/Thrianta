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

void VulkanImageDestroy(vulkan_context* context, vulkan_image* image);