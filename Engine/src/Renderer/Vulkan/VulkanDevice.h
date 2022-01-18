#pragma once
#include "VulkanTypes.inl"

b8 VulkanDeviceCreate(vulkan_context* context);

void VulkanDeviceDestroy(vulkan_context* context);

void VulkanDeviceQuerySwapchainSupport(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    vulkan_swapchain_support_info* outSupportInfo);

b8 VulkanDeviceDetectDepthFormat(vulkan_device* device);