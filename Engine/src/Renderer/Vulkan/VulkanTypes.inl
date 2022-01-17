#pragma once

#include "Defines.h"
#include "Core/Asserts.h"

#include <vulkan/vulkan.h>

// Checks the given expression's return value against VK_SUCCESS.
#define VK_CHECK(expr)               \
    {                                \
        TASSERT(expr == VK_SUCCESS); \
    }


typedef struct vulkan_context
{
    VkInstance instance;
    VkAllocationCallbacks* allocator;

#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
} vulkan_context;