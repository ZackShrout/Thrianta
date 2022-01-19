#pragma once
#include "VulkanTypes.inl"

void VulkanFenceCreate(
    vulkan_context* context,
    b8 createSignaled,
    vulkan_fence* outFence);

void VulkanFenceDestroy(vulkan_context* context, vulkan_fence* fence);

b8 VulkanFenceWait(vulkan_context* context, vulkan_fence* fence, u64 timeoutNS);

void VulkanFenceReset(vulkan_context* context, vulkan_fence* fence);