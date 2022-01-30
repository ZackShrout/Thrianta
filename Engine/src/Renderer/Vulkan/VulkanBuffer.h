#pragma once
#include "VulkanTypes.inl"

b8 VulkanBufferCreate(
    vulkan_context* context,
    u64 size,
    VkBufferUsageFlagBits usage,
    u32 memoryPropertyFlags,
    b8 bindOnCreate,
    vulkan_buffer* outBuffer);
void VulkanBufferDestroy(vulkan_context* context, vulkan_buffer* buffer);
b8 VulkanBufferResize(
    vulkan_context* context,
    u64 newSize,
    vulkan_buffer* buffer,
    VkQueue queue,
    VkCommandPool pool);
void VulkanBufferBind(vulkan_context* context, vulkan_buffer* buffer, u64 offset);
void* VulkanBufferLockMemory(vulkan_context* context, vulkan_buffer* buffer, u64 offset, u64 size, u32 flags);
void VulkanBufferUnlockMemory(vulkan_context* context, vulkan_buffer* buffer);
void VulkanBufferLoadData(vulkan_context* context, vulkan_buffer* buffer, u64 offset, u64 size, u32 flags, const void* data);
void VulkanBufferCopyTo(
    vulkan_context* context,
    VkCommandPool pool,
    VkFence fence,
    VkQueue queue,
    VkBuffer source,
    u64 sourceOffset,
    VkBuffer dest,
    u64 destOffset,
    u64 size);