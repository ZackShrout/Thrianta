#include "VulkanImage.h"
#include "VulkanDevice.h"
#include "Core/TMemory.h"
#include "Core/Logger.h"

void VulkanImageCreate(
    vulkan_context* context,
    VkImageType imageType,
    u32 width,
    u32 height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags memoryFlags,
    b32 createView,
    VkImageAspectFlags viewAspectFlags,
    vulkan_image* outImage)
{

    // Copy params
    outImage->width = width;
    outImage->height = height;

    // Creation info.
    VkImageCreateInfo imageCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = 1;  // TODO: Support configurable depth.
    imageCreateInfo.mipLevels = 4;     // TODO: Support mip mapping
    imageCreateInfo.arrayLayers = 1;   // TODO: Support number of layers in the image.
    imageCreateInfo.format = format;
    imageCreateInfo.tiling = tiling;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = usage;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;          // TODO: Configurable sample count.
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // TODO: Configurable sharing mode.

    VK_CHECK(vkCreateImage(context->device.logicalDevice, &imageCreateInfo, context->allocator, &outImage->handle));

    // Query memory requirements.
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(context->device.logicalDevice, outImage->handle, &memoryRequirements);

    s32 memoryType = context->FindMemoryIndex(memoryRequirements.memoryTypeBits, memoryFlags);
    if (memoryType == -1)
    {
        TERROR("Required memory type not found. Image not valid.");
    }

    // Allocate memory
    VkMemoryAllocateInfo memoryAllocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = memoryType;
    VK_CHECK(vkAllocateMemory(context->device.logicalDevice, &memoryAllocateInfo, context->allocator, &outImage->memory));

    // Bind the memory
    VK_CHECK(vkBindImageMemory(context->device.logicalDevice, outImage->handle, outImage->memory, 0));  // TODO: configurable memory offset.

    // Create view
    if (createView)
    {
        outImage->view = 0;
        VulkanImageViewCreate(context, format, outImage, viewAspectFlags);
    }
}

void VulkanImageViewCreate(
    vulkan_context* context,
    VkFormat format,
    vulkan_image* image,
    VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewCreateInfo.image = image->handle;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;  // TODO: Make configurable.
    viewCreateInfo.format = format;
    viewCreateInfo.subresourceRange.aspectMask = aspectFlags;

    // TODO: Make configurable
    viewCreateInfo.subresourceRange.baseMipLevel = 0;
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(context->device.logicalDevice, &viewCreateInfo, context->allocator, &image->view));
}

void VulkanImageDestroy(vulkan_context* context, vulkan_image* image)
{
    if (image->view)
    {
        vkDestroyImageView(context->device.logicalDevice, image->view, context->allocator);
        image->view = 0;
    }
    if (image->memory)
    {
        vkFreeMemory(context->device.logicalDevice, image->memory, context->allocator);
        image->memory = 0;
    }
    if (image->handle)
    {
        vkDestroyImage(context->device.logicalDevice, image->handle, context->allocator);
        image->handle = 0;
    }
}