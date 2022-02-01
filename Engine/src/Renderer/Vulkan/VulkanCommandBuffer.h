#pragma once
#include "VulkanTypes.inl"

void VulkanCommandBufferAllocate(
    vulkan_context* context,
    VkCommandPool pool,
    b8 isPrimary,
    vulkan_command_buffer* outCommandBuffer);

void VulkanCommandBufferFree(
    vulkan_context* context,
    VkCommandPool pool,
    vulkan_command_buffer* cmdBuffer);

void VulkanCommandBufferBegin(
    vulkan_command_buffer* cmdBuffer,
    b8 isSingleUse,
    b8 isRenderpassContinue,
    b8 isSimultaneousUse);

void VulkanCommandBufferEnd(vulkan_command_buffer* cmdBuffer);

void VulkanCommandBufferUpdateSubmitted(vulkan_command_buffer* cmdBuffer);

void VulkanCommandBufferReset(vulkan_command_buffer* cmdBuffer);

/**
 * Allocates and begins recording to outCommandBuffer.
 */
void VulkanCommandBufferAllocateAndBeginSingleUse(
    vulkan_context* context,
    VkCommandPool pool,
    vulkan_command_buffer* outCommandBuffer);

/**
 * Ends recording, submits to and waits for queue operation and frees the provided command buffer.
 */
void VulkanCommandBufferEndSingleUse(
    vulkan_context* context,
    VkCommandPool pool,
    vulkan_command_buffer* cmdBuffer,
    VkQueue queue);