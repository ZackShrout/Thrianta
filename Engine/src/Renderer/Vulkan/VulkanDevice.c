#include "VulkanDevice.h"
#include "Core/Logger.h"
#include "Core/TString.h"
#include "Core/TMemory.h"
#include "Containers/DArray.h"

typedef struct vulkan_physical_device_requirements
{
    b8 graphics;
    b8 present;
    b8 compute;
    b8 transfer;
    // darray
    const char** deviceExtensionNames;
    b8 samplerAnisotropy;
    b8 discreteGPU;
} vulkan_physical_device_requirements;

typedef struct vulkan_physical_device_queue_family_info
{
    u32 graphicsFamilyIndex;
    u32 presentFamilyIndex;
    u32 computeFamilyIndex;
    u32 transferFamilyIndex;
} vulkan_physical_device_queue_family_info;

b8 SelectPhysicalDevice(vulkan_context* context);
b8 PhysicalDeviceMeetsRequirements(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const vulkan_physical_device_requirements* requirements,
    vulkan_physical_device_queue_family_info* outQueueFamilyInfo,
    vulkan_swapchain_support_info* outSwapChainSupport);

b8 VulkanDeviceCreate(vulkan_context* context)
{
    if (!SelectPhysicalDevice(context))
    {
        return false;
    }

    TINFO("Creating logical device...");
    // NOTE: Do not create additional queues for shared indices.
    b8 presentSharesGraphicsQueue = context->device.graphicsQueueIndex == context->device.presentQueueIndex;
    b8 transferSharesGraphicsQueue = context->device.graphicsQueueIndex == context->device.transferQueueIndex;
    b8 transferSharesPresentQueue = context->device.transferQueueIndex == context->device.presentQueueIndex;
    u32 indexCount = 1;
    if (!presentSharesGraphicsQueue)
    {
        indexCount++;
    }
    if (!transferSharesGraphicsQueue)
    {
        indexCount++;
    }
    if (transferSharesPresentQueue)
    {
        indexCount--;
    }
    u32 indices[32]; // indexCount
    u8 index = 0;
    indices[index++] = context->device.graphicsQueueIndex;
    if (!presentSharesGraphicsQueue)
    {
        indices[index++] = context->device.presentQueueIndex;
    }
    if (!transferSharesGraphicsQueue && !transferSharesPresentQueue)
    {
        indices[index++] = context->device.transferQueueIndex;
    }

    VkDeviceQueueCreateInfo queueCreateInfos[32]; // indexCount
    for (u32 i = 0; i < indexCount; ++i)
    {
        queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[i].queueFamilyIndex = indices[i];
        queueCreateInfos[i].queueCount = 1;
        // TODO: This is here for future multithreading, but will be handle better at that time
        // if (indices[i] == context->device.graphicsQueueIndex)
        // {
        //     queueCreateInfos[i].queueCount = 2;
        // }
        queueCreateInfos[i].flags = 0;
        queueCreateInfos[i].pNext = 0;
        f32 queuePriority = 1.0f;
        queueCreateInfos[i].pQueuePriorities = &queuePriority;
    }

    // Request device features.
    // TODO: should be config driven
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;  // Request anistrophy

    VkDeviceCreateInfo deviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    deviceCreateInfo.queueCreateInfoCount = indexCount;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = 1;
    const char* extensionNames = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    deviceCreateInfo.ppEnabledExtensionNames = &extensionNames;

    // Deprecated and ignored, so pass nothing.
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = 0;

    // Create the device.
    VK_CHECK(vkCreateDevice(
        context->device.physicalDevice,
        &deviceCreateInfo,
        context->allocator,
        &context->device.logicalDevice));

    TINFO("Logical device created.");

    // Get queues.
    vkGetDeviceQueue(
        context->device.logicalDevice,
        context->device.graphicsQueueIndex,
        0,
        &context->device.graphicsQueue);

    vkGetDeviceQueue(
        context->device.logicalDevice,
        context->device.presentQueueIndex,
        0,
        &context->device.presentQueue);

    vkGetDeviceQueue(
        context->device.logicalDevice,
        context->device.transferQueueIndex,
        0,
        &context->device.transferQueue);
    TINFO("Queues obtained.");

    // Create command pool for graphics queue.
    VkCommandPoolCreateInfo poolCreateInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    poolCreateInfo.queueFamilyIndex = context->device.graphicsQueueIndex;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK(vkCreateCommandPool(
        context->device.logicalDevice,
        &poolCreateInfo,
        context->allocator,
        &context->device.graphicsCommandPool));
    TINFO("Graphics command pool created.");

    return true;
}

void VulkanDeviceDestroy(vulkan_context* context)
{
    // Unset queues
    context->device.graphicsQueue = 0;
    context->device.presentQueue = 0;
    context->device.transferQueue = 0;

    TINFO("Destroying command pools...");
    vkDestroyCommandPool(
        context->device.logicalDevice,
        context->device.graphicsCommandPool,
        context->allocator);

    // Destroy logical device
    TINFO("Destroying logical device...");
    if (context->device.logicalDevice) {
        vkDestroyDevice(context->device.logicalDevice, context->allocator);
        context->device.logicalDevice = 0;
    }

    // Physical devices are not destroyed.
    TINFO("Releasing physical device resources...");
    context->device.physicalDevice = 0;

    if (context->device.swapchainSupport.formats) {
        TFree(
            context->device.swapchainSupport.formats,
            sizeof(VkSurfaceFormatKHR) * context->device.swapchainSupport.formatCount,
            MEMORY_TAG_RENDERER);
        context->device.swapchainSupport.formats = 0;
        context->device.swapchainSupport.formatCount = 0;
    }

    if (context->device.swapchainSupport.presentModes) {
        TFree(
            context->device.swapchainSupport.presentModes,
            sizeof(VkPresentModeKHR) * context->device.swapchainSupport.presentModeCount,
            MEMORY_TAG_RENDERER);
        context->device.swapchainSupport.presentModes = 0;
        context->device.swapchainSupport.presentModeCount = 0;
    }

    TZeroMemory(
        &context->device.swapchainSupport.capabilities,
        sizeof(context->device.swapchainSupport.capabilities));

    context->device.graphicsQueueIndex = -1;
    context->device.presentQueueIndex = -1;
    context->device.transferQueueIndex = -1;
}

void VulkanDeviceQuerySwapchainSupport(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    vulkan_swapchain_support_info* outSupportInfo)
{
    // Surface capabilities
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physicalDevice,
        surface,
        &outSupportInfo->capabilities));

    // Surface formats
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice,
        surface,
        &outSupportInfo->formatCount,
        0));

    if (outSupportInfo->formatCount != 0)
    {
        if (!outSupportInfo->formats)
        {
            outSupportInfo->formats = TAllocate(sizeof(VkSurfaceFormatKHR) * outSupportInfo->formatCount, MEMORY_TAG_RENDERER);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
            physicalDevice,
            surface,
            &outSupportInfo->formatCount,
            outSupportInfo->formats));
    }

    // Present modes
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice,
        surface,
        &outSupportInfo->presentModeCount,
        0));
    if (outSupportInfo->presentModeCount != 0)
    {
        if (!outSupportInfo->presentModes)
        {
            outSupportInfo->presentModes = TAllocate(sizeof(VkPresentModeKHR) * outSupportInfo->presentModeCount, MEMORY_TAG_RENDERER);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice,
            surface,
            &outSupportInfo->presentModeCount,
            outSupportInfo->presentModes));
    }
}

b8 VulkanDeviceDetectDepthFormat(vulkan_device* device)
{
    // Format candidates
    const u64 candidate_count = 3;
    VkFormat candidates[3] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT};

    u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    for (u64 i = 0; i < candidate_count; i++)
    {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(device->physicalDevice, candidates[i], &properties);

        if ((properties.linearTilingFeatures & flags) == flags)
        {
            device->depthFormat = candidates[i];
            return true;
        }
        else if ((properties.optimalTilingFeatures & flags) == flags)
        {
            device->depthFormat = candidates[i];
            return true;
        }
    }

    return false;
}

b8 SelectPhysicalDevice(vulkan_context* context)
{
    u32 physicalDeviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physicalDeviceCount, 0));
    if (physicalDeviceCount == 0)
    {
        TFATAL("No devices which support Vulkan were found.");
        return false;
    }
    const u32 maxDeviceCount = 32;
    VkPhysicalDevice physicalDevices[maxDeviceCount]; // physicalDeviceCount
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physicalDeviceCount, physicalDevices));
    for (u32 i = 0; i < physicalDeviceCount; i++)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(physicalDevices[i], &features);

        VkPhysicalDeviceMemoryProperties memory;
        vkGetPhysicalDeviceMemoryProperties(physicalDevices[i], &memory);

        // TODO: These requirements should probably be driven by engine
        // configuration.
        vulkan_physical_device_requirements requirements = {};
        requirements.graphics = true;
        requirements.present = true;
        requirements.transfer = true;
        // NOTE: Enable this if compute will be required.
        // requirements.compute = true;
        requirements.samplerAnisotropy = true;
        requirements.discreteGPU = true;
        requirements.deviceExtensionNames = DArrayCreate(const char*);
        DArrayPush(requirements.deviceExtensionNames, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        vulkan_physical_device_queue_family_info queueInfo = {};
        b8 result = PhysicalDeviceMeetsRequirements(
            physicalDevices[i],
            context->surface,
            &properties,
            &features,
            &requirements,
            &queueInfo,
            &context->device.swapchainSupport);

        if (result)
        {
            TINFO("Selected device: '%s'.", properties.deviceName);
            // GPU type, etc.
            switch (properties.deviceType) {
                default:
                case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                    TINFO("GPU type is Unknown.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    TINFO("GPU type is Integrated.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    TINFO("GPU type is Descrete.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    TINFO("GPU type is Virtual.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    TINFO("GPU type is CPU.");
                    break;
            }

            TINFO("GPU Driver version: %d.%d.%d",
                VK_VERSION_MAJOR(properties.driverVersion),
                VK_VERSION_MINOR(properties.driverVersion),
                VK_VERSION_PATCH(properties.driverVersion));

            // Vulkan API version.
            TINFO("Vulkan API version: %d.%d.%d",
                VK_VERSION_MAJOR(properties.apiVersion),
                VK_VERSION_MINOR(properties.apiVersion),
                VK_VERSION_PATCH(properties.apiVersion));

            // Memory information
            for (u32 j = 0; j < memory.memoryHeapCount; j++)
            {
                f32 memorySizeGib = (((f32)memory.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f);
                if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                {
                    TINFO("Local GPU memory: %.2f GiB", memorySizeGib);
                }
                else
                {
                    TINFO("Shared System memory: %.2f GiB", memorySizeGib);
                }
            }

            context->device.physicalDevice = physicalDevices[i];
            context->device.graphicsQueueIndex = queueInfo.graphicsFamilyIndex;
            context->device.presentQueueIndex = queueInfo.presentFamilyIndex;
            context->device.transferQueueIndex = queueInfo.transferFamilyIndex;
            // NOTE: set compute index here if needed.

            // Keep a copy of properties, features and memory info for later use.
            context->device.properties = properties;
            context->device.features = features;
            context->device.memory = memory;
            break;
        }
    }

    // Ensure a device was selected
    if (!context->device.physicalDevice)
    {
        TERROR("No physical devices were found which meet the requirements.");
        return false;
    }

    TINFO("Physical device selected.");
    return true;
}

b8 PhysicalDeviceMeetsRequirements(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const vulkan_physical_device_requirements* requirements,
    vulkan_physical_device_queue_family_info* outQueueInfo,
    vulkan_swapchain_support_info* outSwapChainSupport)
{
    // Evaluate device properties to determine if it meets the needs of our applcation.
    outQueueInfo->graphicsFamilyIndex = -1;
    outQueueInfo->presentFamilyIndex = -1;
    outQueueInfo->computeFamilyIndex = -1;
    outQueueInfo->transferFamilyIndex = -1;

    // Discrete GPU?
    if (requirements->discreteGPU)
    {
        if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            TINFO("Device is not a discrete GPU, and one is required. Skipping.");
            return false;
        }
    }

    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 0);
    VkQueueFamilyProperties queueFamilies[32]; // queueFamilyCount
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    // Look at each queue and see what queues it supports
    TINFO("Graphics | Present | Compute | Transfer | Name");
    u8 minTransferScore = 255;
    for (u32 i = 0; i < queueFamilyCount; i++)
    {
        u8 currentTransferScore = 0;

        // Graphics queue?
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            outQueueInfo->graphicsFamilyIndex = i;
            ++currentTransferScore;
        }

        // Compute queue?
        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            outQueueInfo->computeFamilyIndex = i;
            ++currentTransferScore;
        }

        // Transfer queue?
        if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            // Take the index if it is the current lowest. This increases the
            // liklihood that it is a dedicated transfer queue.
            if (currentTransferScore <= minTransferScore)
            {
                minTransferScore = currentTransferScore;
                outQueueInfo->transferFamilyIndex = i;
            }
        }

        // Present queue?
        VkBool32 supportsPresent = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supportsPresent));
        if (supportsPresent)
        {
            outQueueInfo->presentFamilyIndex = i;
        }
    }

    // Print out some info about the device
    TINFO("       %d |       %d |       %d |        %d | %s",
          outQueueInfo->graphicsFamilyIndex != -1,
          outQueueInfo->presentFamilyIndex != -1,
          outQueueInfo->computeFamilyIndex != -1,
          outQueueInfo->transferFamilyIndex != -1,
          properties->deviceName);

    if ((!requirements->graphics || (requirements->graphics && outQueueInfo->graphicsFamilyIndex != -1)) &&
        (!requirements->present || (requirements->present && outQueueInfo->presentFamilyIndex != -1)) &&
        (!requirements->compute || (requirements->compute && outQueueInfo->computeFamilyIndex != -1)) &&
        (!requirements->transfer || (requirements->transfer && outQueueInfo->transferFamilyIndex != -1)))
    {
        TINFO("Device meets queue requirements.");
        TTRACE("Graphics Family Index: %i", outQueueInfo->graphicsFamilyIndex);
        TTRACE("Present Family Index:  %i", outQueueInfo->presentFamilyIndex);
        TTRACE("Transfer Family Index: %i", outQueueInfo->transferFamilyIndex);
        TTRACE("Compute Family Index:  %i", outQueueInfo->computeFamilyIndex);

        // Query swapchain support.
        VulkanDeviceQuerySwapchainSupport(
            device,
            surface,
            outSwapChainSupport);

        if (outSwapChainSupport->formatCount < 1 || outSwapChainSupport->presentModeCount < 1)
        {
            if (outSwapChainSupport->formats)
            {
                TFree(outSwapChainSupport->formats, sizeof(VkSurfaceFormatKHR) * outSwapChainSupport->formatCount, MEMORY_TAG_RENDERER);
            }
            if (outSwapChainSupport->presentModes)
            {
                TFree(outSwapChainSupport->presentModes, sizeof(VkPresentModeKHR) * outSwapChainSupport->presentModeCount, MEMORY_TAG_RENDERER);
            }
            TINFO("Required swapchain support not present, skipping device.");
            return false;
        }

        // Device extensions.
        if (requirements->deviceExtensionNames)
        {
            u32 availableExtensionCount = 0;
            VkExtensionProperties* availableExtensions = 0;
            VK_CHECK(vkEnumerateDeviceExtensionProperties(
                device,
                0,
                &availableExtensionCount,
                0));
            if (availableExtensionCount != 0)
            {
                availableExtensions = TAllocate(sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
                VK_CHECK(vkEnumerateDeviceExtensionProperties(
                    device,
                    0,
                    &availableExtensionCount,
                    availableExtensions));

                u32 requiredExtensionCount = DArrayLength(requirements->deviceExtensionNames);
                for (u32 i = 0; i < requiredExtensionCount; i++)
                {
                    b8 found = false;
                    for (u32 j = 0; j < availableExtensionCount; j++)
                    {
                        if (StringsEqual(requirements->deviceExtensionNames[i], availableExtensions[j].extensionName))
                        {
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                    {
                        TINFO("Required extension not found: '%s', skipping device.", requirements->deviceExtensionNames[i]);
                        TFree(availableExtensions, sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
                        return false;
                    }
                }
            }
            TFree(availableExtensions, sizeof(VkExtensionProperties) * availableExtensionCount, MEMORY_TAG_RENDERER);
        }

        // Sampler anisotropy
        if (requirements->samplerAnisotropy && !features->samplerAnisotropy)
        {
            TINFO("Device does not support samplerAnisotropy, skipping.");
            return false;
        }

        // Device meets all requirements.
        return true;
    }

    return false;
}