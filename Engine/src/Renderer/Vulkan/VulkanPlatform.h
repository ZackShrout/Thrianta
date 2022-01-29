#pragma once
#include "Defines.h"

struct platform_state;
struct vulkan_context;

b8 PlatformCreateVulkanSurface(struct vulkan_context* context);

/**
 * Appends the names of required extensions for this platform to
 * the namesDArray, which should be created and passed in.
 */
void PlatformGetRequiredExtensionNames(const char*** namesDArray);