#pragma once

#include "Defines.h"
#include "Core/Asserts.h"

#include <vulkan/vulkan.h>

// Checks the given expression's return value against VK_SUCCESS.
#define VK_CHECK(expr)               \
    {                                \
        TASSERT(expr == VK_SUCCESS); \
    }

typedef struct vulkan_swapchain_support_info
{
    VkSurfaceCapabilitiesKHR capabilities;
    u32 formatCount;
    VkSurfaceFormatKHR* formats;
    u32 presentModeCount;
    VkPresentModeKHR* presentModes;
} vulkan_swapchain_support_info;

typedef struct vulkan_device {
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    vulkan_swapchain_support_info swapchainSupport;
    s32 graphicsQueueIndex;
    s32 presentQueueIndex;
    s32 transferQueueIndex;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;
} vulkan_device;

typedef struct vulkan_context
{
    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkSurfaceKHR surface;

#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debugMessenger;
#endif

    vulkan_device device;
} vulkan_context;