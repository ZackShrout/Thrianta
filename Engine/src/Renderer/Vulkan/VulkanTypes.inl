#pragma once

#include "Defines.h"
#include "Core/Asserts.h"
#include "Renderer/RendererTypes.inl"
#include <vulkan/vulkan.h>

// Checks the given expression's return value against VK_SUCCESS.
#define VK_CHECK(expr)               \
    {                                \
        TASSERT(expr == VK_SUCCESS); \
    }

typedef struct vulkan_buffer
{
    u64 totalSize;
    VkBuffer handle;
    VkBufferUsageFlagBits usage;
    b8 isLocked;
    VkDeviceMemory memory;
    s32 memoryIndex;
    u32 memoryPropertyFlags;
} vulkan_buffer;

typedef struct vulkan_swapchain_support_info
{
    VkSurfaceCapabilitiesKHR capabilities;
    u32 formatCount;
    VkSurfaceFormatKHR* formats;
    u32 presentModeCount;
    VkPresentModeKHR* presentModes;
} vulkan_swapchain_support_info;

typedef struct vulkan_device
{
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    vulkan_swapchain_support_info swapchainSupport;
    s32 graphicsQueueIndex;
    s32 presentQueueIndex;
    s32 transferQueueIndex;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;
    VkCommandPool graphicsCommandPool;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;
    VkFormat depthFormat;
    b8 supportsDeviceLocalHostVisible;
} vulkan_device;

typedef struct vulkan_image
{
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    u32 width;
    u32 height;
} vulkan_image;

typedef enum vulkan_render_pass_state
{
    READY,
    RECORDING,
    IN_RENDER_PASS,
    RECORDING_ENDED,
    SUBMITTED,
    NOT_ALLOCATED
} vulkan_render_pass_state;

typedef struct vulkan_renderpass
{
    VkRenderPass handle;
    f32 x, y, w, h;
    f32 r, g, b, a;
    f32 depth;
    u32 stencil;
    vulkan_render_pass_state state;
} vulkan_renderpass;

typedef struct vulkan_framebuffer
{
    VkFramebuffer handle;
    u32 attachmentCount;
    VkImageView* attachments;
    vulkan_renderpass* renderpass;
} vulkan_framebuffer;

typedef struct vulkan_swapchain
{
    VkSurfaceFormatKHR imageFormat;
    u8 maxFramesInFlight;
    VkSwapchainKHR handle;
    u32 imageCount;
    VkImage* images;
    VkImageView* views;
    vulkan_image depthAttachment;
    vulkan_framebuffer* framebuffers; // framebuffers used for on-screen rendering.
} vulkan_swapchain;

typedef enum vulkan_command_buffer_state
{
    COMMAND_BUFFER_STATE_READY,
    COMMAND_BUFFER_STATE_RECORDING,
    COMMAND_BUFFER_STATE_IN_RENDER_PASS,
    COMMAND_BUFFER_STATE_RECORDING_ENDED,
    COMMAND_BUFFER_STATE_SUBMITTED,
    COMMAND_BUFFER_STATE_NOT_ALLOCATED
} vulkan_command_buffer_state;

typedef struct vulkan_command_buffer
{
    VkCommandBuffer handle;
    vulkan_command_buffer_state state; // Command buffer state.
} vulkan_command_buffer;

typedef struct vulkan_fence
{
    VkFence handle;
    b8 isSignaled;
} vulkan_fence;

typedef struct vulkan_shader_stage
{
    VkShaderModuleCreateInfo createInfo;
    VkShaderModule handle;
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
} vulkan_shader_stage;

typedef struct vulkan_pipeline
{
    VkPipeline handle;
    VkPipelineLayout pipelineLayout;
} vulkan_pipeline;

#define OBJECT_SHADER_STAGE_COUNT 2
typedef struct vulkan_object_shader
{
    vulkan_shader_stage stages[OBJECT_SHADER_STAGE_COUNT]; // vertex, fragment
    VkDescriptorPool globalDescPool;
    VkDescriptorSetLayout globalDescSetLayout;
    VkDescriptorSet globalDescSets[3]; // One descriptor set per frame - max 3 for triple-buffering.
    global_uniform_object globalUBO; // Global uniform object.
    vulkan_buffer globalUniformBuffer; // Global uniform buffer.
    vulkan_pipeline pipeline;
} vulkan_object_shader;

typedef struct vulkan_context
{
    u32 framebufferWidth; // The framebuffer's current width.
    u32 framebufferHeight; // The framebuffer's current height.
    // Current generation of framebuffer size. If it does not match framebufferSizeLastGeneration,
    // a new one should be generated.
    u64 framebufferSizeGeneration;
    // The generation of the framebuffer when it was last created. Set to framebufferSizeGeneration
    // when updated.
    u64 framebufferSizeLastGeneration;
    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkSurfaceKHR surface;
    vulkan_device device;
    vulkan_swapchain swapchain;
    vulkan_renderpass mainRenderpass;
    vulkan_buffer objectVertexBuffer;
    vulkan_buffer objectIndexBuffer;
    vulkan_command_buffer* graphicsCommandBuffers; // darray
    VkSemaphore* imageAvailableSemaphores; // darray
    VkSemaphore* queueCompleteSemaphores; // darray
    u32 inFlightFenceCount;
    vulkan_fence* inFlightFences;
    vulkan_fence** imagesInFlight; // Holds pointers to fences which exist and are owned elsewhere.
    u32 imageIndex;
    u32 currentFrame;
    b8 recreatingSwapchain;
    vulkan_object_shader objectShader;
    u64 geometryVertexOffset;
    u64 geometryIndexOffset;
    s32 (*FindMemoryIndex)(u32 typeFilter, u32 propertyFlags);

#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
} vulkan_context;

typedef struct vulkan_texture_data {
    vulkan_image image;
    VkSampler sampler;
} vulkan_texture_data;