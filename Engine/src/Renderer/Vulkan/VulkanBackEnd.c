#include "VulkanBackEnd.h"
#include "VulkanTypes.inl"
#include "VulkanPlatform.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "Core/Logger.h"
#include "Core/TString.h"
#include "Containers/DArray.h"

// static Vulkan context
static vulkan_context context;

VKAPI_ATTR VkBool32 VKAPI_CALL VKDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData);

s32 FindMemoryIndex(u32 typeFilter, u32 propertyFlags);

b8 VulkanRendererBackendInitialize(renderer_backend* backend, const char* applicationName, struct platform_state* platState)
{
    // Function pointers
    context.FindMemoryIndex = FindMemoryIndex;
    
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

    // Obtain a list of required extensions
    const char** requiredExtensions = DArrayCreate(const char*);
    DArrayPush(requiredExtensions, &VK_KHR_SURFACE_EXTENSION_NAME);  // Generic surface extension
    PlatformGetRequiredExtensionNames(&requiredExtensions); // Platform-specific extension(s)
#if defined(_DEBUG)
    DArrayPush(requiredExtensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME);  // debug utilities

    TDEBUG("Required extensions:");
    u32 length = DArrayLength(requiredExtensions);
    for (u32 i = 0; i < length; i++)
        TDEBUG(requiredExtensions[i]);
#endif // _DEBUG

    createInfo.enabledExtensionCount = DArrayLength(requiredExtensions);
    createInfo.ppEnabledExtensionNames = requiredExtensions;

    // Validation layers.
    const char** requiredValidationLayerNames = 0;
    u32 requiredValidationLayerCount = 0;

    // If validation should be done, get a list of the required validation layert names
    // and make sure they exist. Validation layers should only be enabled on non-release builds.
#if defined(_DEBUG)
    TINFO("Validation layers enabled. Enumerating...");

    // The list of validation layers required.
    requiredValidationLayerNames = DArrayCreate(const char*);
    DArrayPush(requiredValidationLayerNames, &"VK_LAYER_KHRONOS_validation");
    requiredValidationLayerCount = DArrayLength(requiredValidationLayerNames);

    // Obtain a list of available validation layers
    u32 availableLayerCount = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&availableLayerCount, 0));
    VkLayerProperties* availableLayers = DArrayReserve(VkLayerProperties, availableLayerCount);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers));

    // Verify all required layers are available.
    for (u32 i = 0; i < requiredValidationLayerCount; i++)
    {
        TINFO("Searching for layer: %s...", requiredValidationLayerNames[i]);
        b8 found = FALSE;
        for (u32 j = 0; j < availableLayerCount; j++)
        {
            if (StringsEqual(requiredValidationLayerNames[i], availableLayers[j].layerName))
            {
                found = TRUE;
                TINFO("Found.");
                break;
            }
        }

        if (!found)
        {
            TFATAL("Required validation layer is missing: %s", requiredValidationLayerNames[i]);
            return FALSE;
        }
    }
    TINFO("All required validation layers are present.");
#endif // _DEBUG

    createInfo.enabledLayerCount = requiredValidationLayerCount;
    createInfo.ppEnabledLayerNames = requiredValidationLayerNames;

    VK_CHECK(vkCreateInstance(&createInfo, context.allocator, &context.instance));
    TINFO("Vulkan Instance created.");

    // Debugger
#if defined(_DEBUG)
    TDEBUG("Creating Vulkan debugger...");
    u32 logSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;  //|
                                                                      //    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debugCreateInfo.messageSeverity = logSeverity;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugCreateInfo.pfnUserCallback = VKDebugCallback;

    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkCreateDebugUtilsMessengerEXT");
    TASSERT_MSG(func, "Failed to create debug messenger!");
    VK_CHECK(func(context.instance, &debugCreateInfo, context.allocator, &context.debugMessenger));
    TDEBUG("Vulkan debugger created.");
#endif // _DEBUG

    // Surface
    TDEBUG("Creating Vulkan surface...");
    if (!PlatformCreateVulkanSurface(platState, &context))
    {
        TERROR("Failed to create platform surface!");
        return FALSE;
    }
    TDEBUG("Vulkan surface created.");

    // Device creation
    if (!VulkanDeviceCreate(&context))
    {
        TERROR("Failed to create device!");
        return FALSE;
    }

    // Swapchain
    VulkanSwapchainCreate(
        &context,
        context.framebufferWidth,
        context.framebufferHeight,
        &context.swapchain);
    
    TINFO("Vulkan renderer initialized successfully.");
    return TRUE;
}

void VulkanRendererBackendShutdown(renderer_backend* backend)
{
    // Destroy in the opposite order of creation.

    // Swapchain
    VulkanSwapchainDestroy(&context, &context.swapchain);

    TDEBUG("Destroying Vulkan device...");
    VulkanDeviceDestroy(&context);

    TDEBUG("Destroying Vulkan surface...");
    if (context.surface) {
        vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);
        context.surface = 0;
    }
    
    TDEBUG("Destroying Vulkan debugger...");
    if (context.debugMessenger)
    {
        PFN_vkDestroyDebugUtilsMessengerEXT func =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
        func(context.instance, context.debugMessenger, context.allocator);
    }

    TDEBUG("Destroying Vulkan instance...");
    vkDestroyInstance(context.instance, context.allocator);
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

VKAPI_ATTR VkBool32 VKAPI_CALL VKDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData) {
    switch (messageSeverity) {
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            TERROR(callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            TWARN(callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            TINFO(callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            TTRACE(callbackData->pMessage);
            break;
    }
    return VK_FALSE;
}

s32 FindMemoryIndex(u32 typeFilter, u32 propertyFlags)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(context.device.physicalDevice, &memory_properties);

    for (u32 i = 0; i < memory_properties.memoryTypeCount; i++)
    {
        // Check each memory type to see if its bit is set to 1.
        if (typeFilter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
        {
            return i;
        }
    }

    TWARN("Unable to find suitable memory type!");
    return -1;
}