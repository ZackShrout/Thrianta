#include "VulkanFence.h"
#include "Core/Logger.h"

void VulkanFenceCreate(
    vulkan_context* context,
    b8 createSignaled,
    vulkan_fence* outFence)
{
    // Make sure to signal the fence if required.
    outFence->isSignaled = createSignaled;
    VkFenceCreateInfo fenceCreateInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    if (outFence->isSignaled)
    {
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }

    VK_CHECK(vkCreateFence(
        context->device.logicalDevice,
        &fenceCreateInfo,
        context->allocator,
        &outFence->handle));
}

void VulkanFenceDestroy(vulkan_context* context, vulkan_fence* fence)
{
    if (fence->handle)
    {
        vkDestroyFence(
            context->device.logicalDevice,
            fence->handle,
            context->allocator);
        fence->handle = 0;
    }
    fence->isSignaled = false;
}

b8 VulkanFenceWait(vulkan_context* context, vulkan_fence* fence, u64 timeoutNS)
{
    if (!fence->isSignaled)
    {
        VkResult result = vkWaitForFences(
            context->device.logicalDevice,
            1,
            &fence->handle,
            true,
            timeoutNS);
        switch (result) {
            case VK_SUCCESS:
                fence->isSignaled = true;
                return true;
            case VK_TIMEOUT:
                TWARN("vk_fence_wait - Timed out");
                break;
            case VK_ERROR_DEVICE_LOST:
                TERROR("vk_fence_wait - VK_ERROR_DEVICE_LOST.");
                break;
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                TERROR("vk_fence_wait - VK_ERROR_OUT_OF_HOST_MEMORY.");
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                TERROR("vk_fence_wait - VK_ERROR_OUT_OF_DEVICE_MEMORY.");
                break;
            default:
                TERROR("vk_fence_wait - An unknown error has occurred.");
                break;
        }
    }
    else
    {
        // If already signaled, do not wait.
        return true;
    }

    return false;
}

void VulkanFenceReset(vulkan_context* context, vulkan_fence* fence)
{
    if (fence->isSignaled)
    {
        VK_CHECK(vkResetFences(context->device.logicalDevice, 1, &fence->handle));
        fence->isSignaled = false;
    }
}