#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanUtils.h"
#include "Core/Logger.h"
#include "Core/TMemory.h"

b8 VulkanBufferCreate(
    vulkan_context* context,
    u64 size,
    VkBufferUsageFlagBits usage,
    u32 memoryPropertyFlags,
    b8 bindOnCreate,
    vulkan_buffer* outBuffer)
{
    TZeroMemory(outBuffer, sizeof(vulkan_buffer));
    outBuffer->totalSize = size;
    outBuffer->usage = usage;
    outBuffer->memoryPropertyFlags = memoryPropertyFlags;

    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // NOTE: Only used in one queue.

    VK_CHECK(vkCreateBuffer(context->device.logicalDevice, &bufferInfo, context->allocator, &outBuffer->handle));

    // Gather memory requirements.
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(context->device.logicalDevice, outBuffer->handle, &requirements);
    outBuffer->memoryIndex = context->FindMemoryIndex(requirements.memoryTypeBits, outBuffer->memoryPropertyFlags);
    if (outBuffer->memoryIndex == -1)
    {
        TERROR("Unable to create vulkan buffer because the required memory type index was not found.");
        return false;
    }

    // Allocate memory info
    VkMemoryAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize = requirements.size;
    allocInfo.memoryTypeIndex = (u32)outBuffer->memoryIndex;

    // Allocate the memory.
    VkResult result = vkAllocateMemory(
        context->device.logicalDevice,
        &allocInfo,
        context->allocator,
        &outBuffer->memory);

    if (result != VK_SUCCESS)
    {
        TERROR("Unable to create vulkan buffer because the required memory allocation failed. Error: %i", result);
        return false;
    }

    if (bindOnCreate)
        VulkanBufferBind(context, outBuffer, 0);

    return true;
}

void VulkanBufferDestroy(vulkan_context* context, vulkan_buffer* buffer) {
    if (buffer->memory)
    {
        vkFreeMemory(context->device.logicalDevice, buffer->memory, context->allocator);
        buffer->memory = 0;
    }
    if (buffer->handle)
    {
        vkDestroyBuffer(context->device.logicalDevice, buffer->handle, context->allocator);
        buffer->handle = 0;
    }
    buffer->totalSize = 0;
    buffer->usage = 0;
    buffer->isLocked = false;
}

b8 VulkanBufferResize(
    vulkan_context* context,
    u64 newSize,
    vulkan_buffer* buffer,
    VkQueue queue,
    VkCommandPool pool)
{
    // Create new buffer.
    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = newSize;
    bufferInfo.usage = buffer->usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // NOTE: Only used in one queue.

    VkBuffer newBuffer;
    VK_CHECK(vkCreateBuffer(context->device.logicalDevice, &bufferInfo, context->allocator, &newBuffer));

    // Gather memory requirements.
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(context->device.logicalDevice, newBuffer, &requirements);

    // Allocate memory info
    VkMemoryAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize = requirements.size;
    allocInfo.memoryTypeIndex = (u32)buffer->memoryIndex;

    // Allocate the memory.
    VkDeviceMemory newMemory;
    VkResult result = vkAllocateMemory(context->device.logicalDevice, &allocInfo, context->allocator, &newMemory);
    if (result != VK_SUCCESS)
    {
        TERROR("Unable to resize vulkan buffer because the required memory allocation failed. Error: %i", result);
        return false;
    }

    // Bind the new buffer's memory
    VK_CHECK(vkBindBufferMemory(context->device.logicalDevice, newBuffer, newMemory, 0));

    // Copy over the data
    VulkanBufferCopyTo(context, pool, 0, queue, buffer->handle, 0, newBuffer, 0, buffer->totalSize);

    // Make sure anything potentially using these is finished.
    vkDeviceWaitIdle(context->device.logicalDevice);

    // Destroy the old
    if (buffer->memory)
    {
        vkFreeMemory(context->device.logicalDevice, buffer->memory, context->allocator);
        buffer->memory = 0;
    }
    if (buffer->handle)
    {
        vkDestroyBuffer(context->device.logicalDevice, buffer->handle, context->allocator);
        buffer->handle = 0;
    }

    // Set new properties
    buffer->totalSize = newSize;
    buffer->memory = newMemory;
    buffer->handle = newBuffer;

    return true;
}

void VulkanBufferBind(vulkan_context* context, vulkan_buffer* buffer, u64 offset)
{
    VK_CHECK(vkBindBufferMemory(context->device.logicalDevice, buffer->handle, buffer->memory, offset));
}

void* VulkanBufferLockMemory(vulkan_context* context, vulkan_buffer* buffer, u64 offset, u64 size, u32 flags)
{
    void* data;
    VK_CHECK(vkMapMemory(context->device.logicalDevice, buffer->memory, offset, size, flags, &data));
    return data;
}

void VulkanBufferUnlockMemory(vulkan_context* context, vulkan_buffer* buffer)
{
    vkUnmapMemory(context->device.logicalDevice, buffer->memory);
}

void VulkanBufferLoadData(vulkan_context* context, vulkan_buffer* buffer, u64 offset, u64 size, u32 flags, const void* data)
{
    void* dataPtr;
    VK_CHECK(vkMapMemory(context->device.logicalDevice, buffer->memory, offset, size, flags, &dataPtr));
    TCopyMemory(dataPtr, data, size);
    vkUnmapMemory(context->device.logicalDevice, buffer->memory);
}

void VulkanBufferCopyTo(
    vulkan_context* context,
    VkCommandPool pool,
    VkFence fence,
    VkQueue queue,
    VkBuffer source,
    u64 sourceOffset,
    VkBuffer dest,
    u64 destOffset,
    u64 size)
{
    vkQueueWaitIdle(queue);
    // Create a one-time-use command buffer.
    vulkan_command_buffer tempCmdBuffer;
    VulkanCommandBufferAllocateAndBeginSingleUse(context, pool, &tempCmdBuffer);

    // Prepare the copy command and add it to the command buffer.
    VkBufferCopy copyRegion;
    copyRegion.srcOffset = sourceOffset;
    copyRegion.dstOffset = destOffset;
    copyRegion.size = size;

    vkCmdCopyBuffer(tempCmdBuffer.handle, source, dest, 1, &copyRegion);

    // Submit the buffer for execution and wait for it to complete.
    VulkanCommandBufferEndSingleUse(context, pool, &tempCmdBuffer, queue);
}