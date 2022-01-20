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
    vulkan_command_buffer* commandBuffer);

void VulkanCommandBufferBegin(
    vulkan_command_buffer* commandBuffer,
    b8 isSingleUse,
    b8 isRenderpassContinue,
    b8 isSimultaneousUse);

void VulkanCommandBufferEnd(vulkan_command_buffer* commandBuffer);

void VulkanCommandBufferUpdateSubmitted(vulkan_command_buffer* commandBuffer);

void VulkanCommandBufferReset(vulkan_command_buffer* commandBuffer);

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
    vulkan_command_buffer* commandBuffer,
    VkQueue queue);