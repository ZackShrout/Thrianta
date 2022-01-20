#include "VulkanBackEnd.h"
#include "VulkanTypes.inl"
#include "VulkanPlatform.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanRenderpass.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanUtils.h"
#include "VulkanFence.h"
#include "Core/Logger.h"
#include "Core/TMemory.h"
#include "Core/TString.h"
#include "Core/Application.h"
#include "Containers/DArray.h"

// static Vulkan context
static vulkan_context context;
static u32 cachedFramebufferWidth;
static u32 cachedFramebufferHeight;

VKAPI_ATTR VkBool32 VKAPI_CALL VKDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData);

s32 FindMemoryIndex(u32 typeFilter, u32 propertyFlags);

void CreateCommandBuffers(renderer_backend* backend);
void RegenerateFramebuffers(renderer_backend* backend, vulkan_swapchain* swapchain, vulkan_renderpass* renderpass);
b8 RecreateSwapchain(renderer_backend* backend);

b8 VulkanRendererBackendInitialize(renderer_backend* backend, const char* applicationName, struct platform_state* platState)
{
    // Function pointers
    context.FindMemoryIndex = FindMemoryIndex;
    
    // TODO: custom allocator.
    context.allocator = 0;

    ApplicationGetFramebufferSize(&cachedFramebufferWidth, &cachedFramebufferHeight);
    context.framebufferWidth = (cachedFramebufferWidth != 0) ? cachedFramebufferWidth : 800;
    context.framebufferHeight = (cachedFramebufferHeight != 0) ? cachedFramebufferHeight : 600;
    cachedFramebufferWidth = 0;
    cachedFramebufferHeight = 0;

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
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT; //|
                       //VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;  //|
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

    // Renderpass
    VulkanRenderpassCreate(
        &context,
        &context.mainRenderpass,
        0, 0, context.framebufferWidth, context.framebufferHeight,
        0.0f, 0.0f, 0.2f, 1.0f,
        1.0f,
        0);

    // Swapchain framebuffers.
    context.swapchain.framebuffers = DArrayReserve(vulkan_framebuffer, context.swapchain.imageCount);
    RegenerateFramebuffers(backend, &context.swapchain, &context.mainRenderpass);

    // Create command buffers.
    CreateCommandBuffers(backend);

    // Create sync objects.
    context.imageAvailableSemaphores = DArrayReserve(VkSemaphore, context.swapchain.maxFramesInFlight);
    context.queueCompleteSemaphores = DArrayReserve(VkSemaphore, context.swapchain.maxFramesInFlight);
    context.inFlightFences = DArrayReserve(vulkan_fence, context.swapchain.maxFramesInFlight);

    for (u8 i = 0; i < context.swapchain.maxFramesInFlight; i++)
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(context.device.logicalDevice, &semaphoreCreateInfo, context.allocator, &context.imageAvailableSemaphores[i]);
        vkCreateSemaphore(context.device.logicalDevice, &semaphoreCreateInfo, context.allocator, &context.queueCompleteSemaphores[i]);

        // Create the fence in a signaled state, indicating that the first frame has already been "rendered".
        // This will prevent the application from waiting indefinitely for the first frame to render since it
        // cannot be rendered until a frame is "rendered" before it.
        VulkanFenceCreate(&context, TRUE, &context.inFlightFences[i]);
    }

    // In flight fences should not yet exist at this point, so clear the list. These are stored in pointers
    // because the initial state should be 0, and will be 0 when not in use. Acutal fences are not owned
    // by this list.
    context.imagesInFlight = DArrayReserve(vulkan_fence, context.swapchain.imageCount);
    for (u32 i = 0; i < context.swapchain.imageCount; i++)
    {
        context.imagesInFlight[i] = 0;
    }
    
    TINFO("Vulkan renderer initialized successfully.");
    return TRUE;
}

void VulkanRendererBackendShutdown(renderer_backend* backend)
{
    vkDeviceWaitIdle(context.device.logicalDevice);
    
    // Destroy in the opposite order of creation.

    // Sync objects
    for (u8 i = 0; i < context.swapchain.maxFramesInFlight; i++)
    {
        if (context.imageAvailableSemaphores[i])
        {
            vkDestroySemaphore(
                context.device.logicalDevice,
                context.imageAvailableSemaphores[i],
                context.allocator);
            context.imageAvailableSemaphores[i] = 0;
        }
        if (context.queueCompleteSemaphores[i])
        {
            vkDestroySemaphore(
                context.device.logicalDevice,
                context.queueCompleteSemaphores[i],
                context.allocator);
            context.queueCompleteSemaphores[i] = 0;
        }
        VulkanFenceDestroy(&context, &context.inFlightFences[i]);
    }
    DArrayDestroy(context.imageAvailableSemaphores);
    context.imageAvailableSemaphores = 0;

    DArrayDestroy(context.queueCompleteSemaphores);
    context.queueCompleteSemaphores = 0;

    DArrayDestroy(context.inFlightFences);
    context.inFlightFences = 0;

    DArrayDestroy(context.imagesInFlight);
    context.imagesInFlight = 0;

    // Command buffers
    for (u32 i = 0; i < context.swapchain.imageCount; i++)
    {
        if (context.graphicsCommandBuffers[i].handle)
        {
            VulkanCommandBufferFree(
                &context,
                context.device.graphicsCommandPool,
                &context.graphicsCommandBuffers[i]);
            context.graphicsCommandBuffers[i].handle = 0;
        }
    }
    DArrayDestroy(context.graphicsCommandBuffers);
    context.graphicsCommandBuffers = 0;

    // Destroy framebuffers
    for (u32 i = 0; i < context.swapchain.imageCount; i++)
    {
        VulkanFramebufferDestroy(&context, &context.swapchain.framebuffers[i]);
    }

    // Renderpass
    VulkanRenderpassDestroy(&context, &context.mainRenderpass);

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
    // Update the "framebuffer size generation", a counter which indicates when the
    // framebuffer size has been updated.
    cachedFramebufferWidth = width;
    cachedFramebufferHeight = height;
    context.framebufferSizeGeneration++;

    TINFO("Vulkan renderer backend->resized: w/h/gen: %i/%i/%llu", width, height, context.framebufferSizeGeneration);
}

b8 VulkanRendererBackendBeginFrame(renderer_backend* backend, f32 dt)
{
    vulkan_device* device = &context.device;

    // Check if recreating swap chain and boot out.
    if (context.recreatingSwapchain)
    {
        VkResult result = vkDeviceWaitIdle(device->logicalDevice);
        if (!VulkanResultIsSuccess(result))
        {
            TERROR("vulkanRendererBackendBeginFrame vkDeviceWaitIdle (1) failed: '%s'", VulkanResultString(result, TRUE));
            return FALSE;
        }
        TINFO("Recreating swapchain, booting.");
        return FALSE;
    }

    // Check if the framebuffer has been resized. If so, a new swapchain must be created.
    if (context.framebufferSizeGeneration != context.framebufferSizeLastGeneration)
    {
        VkResult result = vkDeviceWaitIdle(device->logicalDevice);
        if (!VulkanResultIsSuccess(result))
        {
            TERROR("VulkanRendererBackendBeginFrame vkDeviceWaitIdle (2) failed: '%s'", VulkanResultString(result, TRUE));
            return FALSE;
        }

        // If the swapchain recreation failed (because, for example, the window was minimized),
        // boot out before unsetting the flag.
        if (!RecreateSwapchain(backend))
        {
            return FALSE;
        }

        TINFO("Resized, booting.");
        return FALSE;
    }

    // Wait for the execution of the current frame to complete. The fence being free will allow this one to move on.
    if (!VulkanFenceWait(
            &context,
            &context.inFlightFences[context.currentFrame],
            UINT64_MAX))
    {
        TWARN("In-flight fence wait failure!");
        return FALSE;
    }

    // Acquire the next image from the swap chain. Pass along the semaphore that should signaled when this completes.
    // This same semaphore will later be waited on by the queue submission to ensure this image is available.
    if (!VulkanSwapchainAcquireNextImageIndex(
            &context,
            &context.swapchain,
            UINT64_MAX,
            context.imageAvailableSemaphores[context.currentFrame],
            0,
            &context.imageIndex))
    {
        return FALSE;
    }

    // Begin recording commands.
    vulkan_command_buffer* commandBuffer = &context.graphicsCommandBuffers[context.imageIndex];
    VulkanCommandBufferReset(commandBuffer);
    VulkanCommandBufferBegin(commandBuffer, FALSE, FALSE, FALSE);

    // Dynamic state
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (f32)context.framebufferHeight;
    viewport.width = (f32)context.framebufferWidth;
    viewport.height = -(f32)context.framebufferHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Scissor
    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = context.framebufferWidth;
    scissor.extent.height = context.framebufferHeight;

    vkCmdSetViewport(commandBuffer->handle, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer->handle, 0, 1, &scissor);

    context.mainRenderpass.w = context.framebufferWidth;
    context.mainRenderpass.h = context.framebufferHeight;

    // Begin the render pass.
    VulkanRenderpassBegin(
        commandBuffer,
        &context.mainRenderpass,
        context.swapchain.framebuffers[context.imageIndex].handle);
    return TRUE;
}

b8 VulkanRendererBackendEndFrame(renderer_backend* backend, f32 dt)
{
    vulkan_command_buffer* commandBuffer = &context.graphicsCommandBuffers[context.imageIndex];

    // End renderpass
    VulkanRenderpassEnd(commandBuffer, &context.mainRenderpass);

    VulkanCommandBufferEnd(commandBuffer);

    // Make sure the previous frame is not using this image (i.e. its fence is being waited on)
    if (context.imagesInFlight[context.imageIndex] != VK_NULL_HANDLE) // was frame
    {
        VulkanFenceWait(
            &context,
            context.imagesInFlight[context.imageIndex],
            UINT64_MAX);
    }

    // Mark the image fence as in-use by this frame.
    context.imagesInFlight[context.imageIndex] = &context.inFlightFences[context.currentFrame];

    // Reset the fence for use on the next frame
    VulkanFenceReset(&context, &context.inFlightFences[context.currentFrame]);

    // Submit the queue and wait for the operation to complete.
    // Begin queue submission
    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};

    // Command buffer(s) to be executed.
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer->handle;

    // The semaphore(s) to be signaled when the queue is complete.
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &context.queueCompleteSemaphores[context.currentFrame];

    // Wait semaphore ensures that the operation cannot begin until the image is available.
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &context.imageAvailableSemaphores[context.currentFrame];

    // Each semaphore waits on the corresponding pipeline stage to complete. 1:1 ratio.
    // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent colour attachment
    // writes from executing until the semaphore signals (i.e. one frame is presented at a time)
    VkPipelineStageFlags flags[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pWaitDstStageMask = flags;

    VkResult result = vkQueueSubmit(
        context.device.graphicsQueue,
        1,
        &submitInfo,
        context.inFlightFences[context.currentFrame].handle);
    if (result != VK_SUCCESS)
    {
        TERROR("vkQueueSubmit failed with result: %s", VulkanResultString(result, TRUE));
        return FALSE;
    }

    VulkanCommandBufferUpdateSubmitted(commandBuffer);
    // End queue submission

    // Give the image back to the swapchain.
    VulkanSwapchainPresent(
        &context,
        &context.swapchain,
        context.device.graphicsQueue,
        context.device.presentQueue,
        context.queueCompleteSemaphores[context.currentFrame],
        context.imageIndex);
    
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

void CreateCommandBuffers(renderer_backend* backend)
{
    if (!context.graphicsCommandBuffers)
    {
        context.graphicsCommandBuffers = DArrayReserve(vulkan_command_buffer, context.swapchain.imageCount);
        for (u32 i = 0; i < context.swapchain.imageCount; i++)
        {
            TZeroMemory(&context.graphicsCommandBuffers[i], sizeof(vulkan_command_buffer));
        }
    }

    for (u32 i = 0; i < context.swapchain.imageCount; i++)
    {
        if (context.graphicsCommandBuffers[i].handle)
        {
            VulkanCommandBufferFree(
                &context,
                context.device.graphicsCommandPool,
                &context.graphicsCommandBuffers[i]);
        }
        TZeroMemory(&context.graphicsCommandBuffers[i], sizeof(vulkan_command_buffer));
        VulkanCommandBufferAllocate(
            &context,
            context.device.graphicsCommandPool,
            TRUE,
            &context.graphicsCommandBuffers[i]);
    }

    TINFO("Vulkan command buffers created.");
}

void RegenerateFramebuffers(renderer_backend* backend, vulkan_swapchain* swapchain, vulkan_renderpass* renderpass) {
    for (u32 i = 0; i < swapchain->imageCount; i++)
    {
        // TODO: make this dynamic based on the currently configured attachments
        u32 attachmentCount = 2;
        VkImageView attachments[] = {
            swapchain->views[i],
            swapchain->depthAttachment.view};

        VulkanFramebufferCreate(
            &context,
            renderpass,
            context.framebufferWidth,
            context.framebufferHeight,
            attachmentCount,
            attachments,
            &context.swapchain.framebuffers[i]);
    }
}

b8 RecreateSwapchain(renderer_backend* backend) {
    // If already being recreated, do not try again.
    if (context.recreatingSwapchain)
    {
        TDEBUG("RecreateSwapchain called when already recreating. Booting.");
        return FALSE;
    }

    // Detect if the window is too small to be drawn to
    if (context.framebufferWidth == 0 || context.framebufferHeight == 0)
    {
        TDEBUG("RecreateSwapchain called when window is < 1 in a dimension. Booting.");
        return FALSE;
    }

    // Mark as recreating if the dimensions are valid.
    context.recreatingSwapchain = TRUE;

    // Wait for any operations to complete.
    vkDeviceWaitIdle(context.device.logicalDevice);

    // Clear these out just in case.
    for (u32 i = 0; i < context.swapchain.imageCount; i++)
    {
        context.imagesInFlight[i] = 0;
    }

    // Requery support
    VulkanDeviceQuerySwapchainSupport(
        context.device.physicalDevice,
        context.surface,
        &context.device.swapchainSupport);
    VulkanDeviceDetectDepthFormat(&context.device);

    VulkanSwapchainRecreate(
        &context,
        cachedFramebufferWidth,
        cachedFramebufferHeight,
        &context.swapchain);

    // Sync the framebuffer size with the cached sizes.
    context.framebufferWidth = cachedFramebufferWidth;
    context.framebufferHeight = cachedFramebufferHeight;
    context.mainRenderpass.w = context.framebufferWidth;
    context.mainRenderpass.h = context.framebufferHeight;
    cachedFramebufferWidth = 0;
    cachedFramebufferHeight = 0;

    // Update framebuffer size generation.
    context.framebufferSizeLastGeneration = context.framebufferSizeGeneration;

    // cleanup swapchain
    for (u32 i = 0; i < context.swapchain.imageCount; i++)
    {
        VulkanCommandBufferFree(&context, context.device.graphicsCommandPool, &context.graphicsCommandBuffers[i]);
    }

    // Framebuffers.
    for (u32 i = 0; i < context.swapchain.imageCount; i++)
    {
        VulkanFramebufferDestroy(&context, &context.swapchain.framebuffers[i]);
    }

    context.mainRenderpass.x = 0;
    context.mainRenderpass.y = 0;
    context.mainRenderpass.w = context.framebufferWidth;
    context.mainRenderpass.h = context.framebufferHeight;

    RegenerateFramebuffers(backend, &context.swapchain, &context.mainRenderpass);

    CreateCommandBuffers(backend);

    // Clear the recreating flag.
    context.recreatingSwapchain = FALSE;

    return TRUE;
}