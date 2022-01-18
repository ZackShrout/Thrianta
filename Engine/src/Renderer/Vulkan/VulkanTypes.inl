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

typedef struct vulkan_device
{
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
    VkFormat depthFormat;
} vulkan_device;

typedef struct vulkan_image
{
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    u32 width;
    u32 height;
} vulkan_image;

typedef struct vulkan_swapchain
{
    VkSurfaceFormatKHR imageFormat;
    u8 maxFramesInFlight;
    VkSwapchainKHR handle;
    u32 imageCount;
    VkImage* images;
    VkImageView* views;
    vulkan_image depthAttachment;
} vulkan_swapchain;

typedef struct vulkan_context
{
    u32 framebufferWidth; // The framebuffer's current width.
    u32 framebufferHeight; // The framebuffer's current height.
    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkSurfaceKHR surface;
    vulkan_device device;
    vulkan_swapchain swapchain;
    u32 imageIndex;
    u32 currentFrame;
    b8 recreatingSwapchain;
    s32 (*FindMemoryIndex)(u32 typeFilter, u32 propertyFlags);

#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
} vulkan_context;