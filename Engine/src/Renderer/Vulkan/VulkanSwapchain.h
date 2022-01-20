#pragma once
#include "VulkanTypes.inl"

void VulkanSwapchainCreate(
    vulkan_context* context,
    u32 width,
    u32 height,
    vulkan_swapchain* outSwapchain);

void VulkanSwapchainRecreate(
    vulkan_context* context,
    u32 width,
    u32 height,
    vulkan_swapchain* swapchain);

void VulkanSwapchainDestroy(
    vulkan_context* context,
    vulkan_swapchain* swapchain);

b8 VulkanSwapchainAcquireNextImageIndex(
    vulkan_context* context,
    vulkan_swapchain* swapchain,
    u64 timeoutNS,
    VkSemaphore imageAvailableSemaphores,
    VkFence fence,
    u32* outImageIndex);

void VulkanSwapchainPresent(
    vulkan_context* context,
    vulkan_swapchain* swapchain,
    VkQueue graphicsQueue,
    VkQueue presentQueue,
    VkSemaphore renderCompleteSemaphore,
    u32 presentImageIndex);