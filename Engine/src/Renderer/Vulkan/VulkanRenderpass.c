#include "VulkanRenderpass.h"
#include "Core/TMemory.h"

void VulkanRenderpassCreate(
    vulkan_context* context, 
    vulkan_renderpass* outRenderpass,
    f32 x, f32 y, f32 w, f32 h,
    f32 r, f32 g, f32 b, f32 a,
    f32 depth,
    u32 stencil)
{
    outRenderpass->x = x;
    outRenderpass->y = y;
    outRenderpass->w = w;
    outRenderpass->h = h;

    outRenderpass->r = r;
    outRenderpass->g = g;
    outRenderpass->b = b;
    outRenderpass->a = a;

    outRenderpass->depth = depth;
    outRenderpass->stencil = stencil;

    // Main subpass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    // Attachments TODO: make this configurable.
    u32 attachmentDescriptionCount = 2;
    VkAttachmentDescription attachmentDescriptions[attachmentDescriptionCount];

    // Color attachment
    VkAttachmentDescription colorAttachment;
    colorAttachment.format = context->swapchain.imageFormat.format; // TODO: configurable
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;      // Do not expect any particular layout before render pass starts.
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;  // Transitioned to after the render pass
    colorAttachment.flags = 0;

    attachmentDescriptions[0] = colorAttachment;

    VkAttachmentReference colorAttachmentReference;
    colorAttachmentReference.attachment = 0;  // Attachment description array index
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;

    // Depth attachment, if there is one
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = context->device.depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachmentDescriptions[1] = depthAttachment;

    // Depth attachment reference
    VkAttachmentReference depthAttachmentReference;
    depthAttachmentReference.attachment = 1;
    depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // TODO: other attachment types (input, resolve, preserve)

    // Depth stencil data.
    subpass.pDepthStencilAttachment = &depthAttachmentReference;

    // Input from a shader
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = 0;

    // Attachments used for multisampling colour attachments
    subpass.pResolveAttachments = 0;

    // Attachments not used in this subpass, but must be preserved for the next.
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = 0;

    // Render pass dependencies. TODO: make this configurable.
    VkSubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    // Render pass create.
    VkRenderPassCreateInfo renderPassCreateInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    renderPassCreateInfo.attachmentCount = attachmentDescriptionCount;
    renderPassCreateInfo.pAttachments = attachmentDescriptions;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;
    renderPassCreateInfo.pNext = 0;
    renderPassCreateInfo.flags = 0;

    VK_CHECK(vkCreateRenderPass(
        context->device.logicalDevice,
        &renderPassCreateInfo,
        context->allocator,
        &outRenderpass->handle));
}

void VulkanRenderpassDestroy(vulkan_context* context, vulkan_renderpass* renderpass)
{
    if (renderpass && renderpass->handle)
    {
        vkDestroyRenderPass(context->device.logicalDevice, renderpass->handle, context->allocator);
        renderpass->handle = 0;
    }
}

void VulkanRenderpassBegin(
    vulkan_command_buffer* commandBuffer,
    vulkan_renderpass* renderpass,
    VkFramebuffer frameBuffer)
{
    VkRenderPassBeginInfo beginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    beginInfo.renderPass = renderpass->handle;
    beginInfo.framebuffer = frameBuffer;
    beginInfo.renderArea.offset.x = renderpass->x;
    beginInfo.renderArea.offset.y = renderpass->y;
    beginInfo.renderArea.extent.width = renderpass->w;
    beginInfo.renderArea.extent.height = renderpass->h;

    VkClearValue clear_values[2];
    TZeroMemory(clear_values, sizeof(VkClearValue) * 2);
    clear_values[0].color.float32[0] = renderpass->r;
    clear_values[0].color.float32[1] = renderpass->g;
    clear_values[0].color.float32[2] = renderpass->b;
    clear_values[0].color.float32[3] = renderpass->a;
    clear_values[1].depthStencil.depth = renderpass->depth;
    clear_values[1].depthStencil.stencil = renderpass->stencil;

    beginInfo.clearValueCount = 2;
    beginInfo.pClearValues = clear_values;

    vkCmdBeginRenderPass(commandBuffer->handle, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    commandBuffer->state = COMMAND_BUFFER_STATE_IN_RENDER_PASS;
}

void VulkanRenderpassEnd(vulkan_command_buffer* commandBuffer, vulkan_renderpass* renderpass)
{
    vkCmdEndRenderPass(commandBuffer->handle);
    commandBuffer->state = COMMAND_BUFFER_STATE_RECORDING;
}