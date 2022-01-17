#include "VulkanBackEnd.h"
#include "VulkanTypes.inl"
#include "Core/Logger.h"

// static Vulkan context
static vulkan_context context;

b8 VulkanRendererBackendInitialize(renderer_backend* backend, const char* applicationName, struct platform_state* platState)
{
    // TODO: custom allocator.
    context.allocator = 0;

    // Setup Vulkan instance.
    VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.apiVersion = VK_API_VERSION_1_2;
    appInfo.pApplicationName = applicationName;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Thrianta Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = 0;
    createInfo.ppEnabledExtensionNames = 0;
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = 0;

    VkResult result = vkCreateInstance(&createInfo, context.allocator, &context.instance);
    if(result != VK_SUCCESS)
    {
        TERROR("vkCreateInstance failed with result: %u", result);
        return FALSE;
    }

    TINFO("Vulkan renderer initialized successfully.");
    return TRUE;
}

void VulkanRendererBackendShutdown(renderer_backend* backend)
{
}

void VulkanRendererBackendOnResize(renderer_backend* backend, u16 width, u16 height)
{
}

b8 VulkanRendererBackendBeginFrame(renderer_backend* backend, f32 dt)
{
    return TRUE;
}

b8 VulkanRendererBackendEndFrame(renderer_backend* backend, f32 dt)
{
    return TRUE;
}