#include "VulkanObjectShader.h"
#include "Core/Logger.h"
#include "Core/TMemory.h"
#include "Math/MathTypes.h"
#include "Renderer/Vulkan/VulkanShaderUtils.h"
#include "Renderer/Vulkan/VulkanPipeline.h"
#include "Renderer/Vulkan/VulkanBuffer.h"

#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"

b8 VulkanObjectShaderCreate(vulkan_context* context, vulkan_object_shader* outShader)
{
    // Shader module init per stage.
    char stageTypeStrs[OBJECT_SHADER_STAGE_COUNT][5] = {"vert", "frag"};
    VkShaderStageFlagBits stageTypes[OBJECT_SHADER_STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; i++)
    {
        if (!CreateShaderModule(context, BUILTIN_SHADER_NAME_OBJECT, stageTypeStrs[i], stageTypes[i], i, outShader->stages))
        {
            TERROR("Unable to create %s shader module for '%s'.", stageTypeStrs[i], BUILTIN_SHADER_NAME_OBJECT);
            return false;
        }
    }

    // Global Descriptors
    VkDescriptorSetLayoutBinding globalUBOLayoutBinding;
    globalUBOLayoutBinding.binding = 0;
    globalUBOLayoutBinding.descriptorCount = 1;
    globalUBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    globalUBOLayoutBinding.pImmutableSamplers = 0;
    globalUBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo globalLayoutInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    globalLayoutInfo.bindingCount = 1;
    globalLayoutInfo.pBindings = &globalUBOLayoutBinding;
    VK_CHECK(vkCreateDescriptorSetLayout(context->device.logicalDevice, &globalLayoutInfo, context->allocator, &outShader->globalDescSetLayout));

    // Global descriptor pool: Used for global items such as view/projection matrix.
    VkDescriptorPoolSize globalPoolSize;
    globalPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    globalPoolSize.descriptorCount = context->swapchain.imageCount;

    VkDescriptorPoolCreateInfo globalPoolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    globalPoolInfo.poolSizeCount = 1;
    globalPoolInfo.pPoolSizes = &globalPoolSize;
    globalPoolInfo.maxSets = context->swapchain.imageCount;
    VK_CHECK(vkCreateDescriptorPool(context->device.logicalDevice, &globalPoolInfo, context->allocator, &outShader->globalDescPool));

    // Pipeline creation
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (f32)context->framebufferHeight;
    viewport.width = (f32)context->framebufferWidth;
    viewport.height = -(f32)context->framebufferHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Scissor
    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = context->framebufferWidth;
    scissor.extent.height = context->framebufferHeight;

    // Attributes
    u32 offset = 0;
    const s32 attributeCount = 1;
    VkVertexInputAttributeDescription attributeDesc[attributeCount];

    // Position
    VkFormat formats[attributeCount] =
    {
        VK_FORMAT_R32G32B32_SFLOAT
    };
    u64 sizes[attributeCount] =
    {
        sizeof(vec3)
    };

    for (u32 i = 0; i < attributeCount; i++)
    {
        attributeDesc[i].binding = 0;   // binding index - should match binding desc
        attributeDesc[i].location = i;  // attrib location
        attributeDesc[i].format = formats[i];
        attributeDesc[i].offset = offset;
        offset += sizes[i];
    }

    // Desciptor set layouts.
    const s32 descSetLayoutCount = 1;
    VkDescriptorSetLayout layouts[1] =
    {
        outShader->globalDescSetLayout
    };

    // Stages
    // NOTE: Should match the number of shader->stages.
    VkPipelineShaderStageCreateInfo stageCreateInfos[OBJECT_SHADER_STAGE_COUNT];
    TZeroMemory(stageCreateInfos, sizeof(stageCreateInfos));
    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; i++)
    {
        stageCreateInfos[i].sType = outShader->stages[i].shaderStageCreateInfo.sType;
        stageCreateInfos[i] = outShader->stages[i].shaderStageCreateInfo;
    }

    if (!VulkanGraphicsPipelineCreate(
            context,
            &context->mainRenderpass,
            attributeCount,
            attributeDesc,
            descSetLayoutCount,
            layouts,
            OBJECT_SHADER_STAGE_COUNT,
            stageCreateInfos,
            viewport,
            scissor,
            false,
            &outShader->pipeline))
    {
        TERROR("Failed to load graphics pipeline for object shader.");
        return false;
    }

    // Create uniform buffer.
    if (!VulkanBufferCreate(
            context,
            sizeof(global_uniform_object),
            /*VK_BUFFER_USAGE_TRANSFER_DST_BIT | */VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            /*VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | */VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            true,
            &outShader->globalUniformBuffer))
    {
        TERROR("Vulkan buffer creation failed for object shader.");
        return false;
    }

    // Allocate global descriptor sets.
    VkDescriptorSetLayout globalLayouts[3] =
    {
        outShader->globalDescSetLayout,
        outShader->globalDescSetLayout,
        outShader->globalDescSetLayout
    };

    VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = outShader->globalDescPool;
    allocInfo.descriptorSetCount = context->swapchain.imageCount;
    allocInfo.pSetLayouts = globalLayouts;
    VK_CHECK(vkAllocateDescriptorSets(context->device.logicalDevice, &allocInfo, outShader->globalDescSets));

    return true;
}

void VulkanObjectShaderDestroy(vulkan_context* context, struct vulkan_object_shader* shader)
{
    VkDevice logicalDevice = context->device.logicalDevice;

    // Destroy uniform buffer.
    VulkanBufferDestroy(context, &shader->globalUniformBuffer);

    // Destroy pipeline.
    VulkanPipelineDestroy(context, &shader->pipeline);

    // Destroy global descriptor pool.
    vkDestroyDescriptorPool(logicalDevice, shader->globalDescPool, context->allocator);

    // Destroy descriptor set layouts.
    vkDestroyDescriptorSetLayout(logicalDevice, shader->globalDescSetLayout, context->allocator);
    
    // Destroy shader modules.
    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; i++)
    {
        vkDestroyShaderModule(context->device.logicalDevice, shader->stages[i].handle, context->allocator);
        shader->stages[i].handle = 0;
    }
}

void VulkanObjectShaderUse(vulkan_context* context, struct vulkan_object_shader* shader)
{
    u32 imageIndex = context->imageIndex;
    VulkanPipelineBind(&context->graphicsCommandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, &shader->pipeline);
}

void VulkanObjectShaderUpdateGlobalState(vulkan_context* context, struct vulkan_object_shader* shader)
{
    u32 imageIndex = context->imageIndex;
    VkCommandBuffer cmdBuffer = context->graphicsCommandBuffers[imageIndex].handle;
    VkDescriptorSet globalDesc = shader->globalDescSets[imageIndex];

    // Bind the global descriptor set to be updated.
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline.pipelineLayout, 0, 1, &globalDesc, 0, 0);

    // Configure the descriptors for the given index.
    u32 range = sizeof(global_uniform_object);
    u64 offset = 0;

    // Copy data to buffer
    VulkanBufferLoadData(context, &shader->globalUniformBuffer, offset, range, 0, &shader->globalUBO);

    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = shader->globalUniformBuffer.handle;
    bufferInfo.offset = offset;
    bufferInfo.range = range;

    // Update descriptor sets.
    VkWriteDescriptorSet descriptor_write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    descriptor_write.dstSet = shader->globalDescSets[imageIndex];
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(context->device.logicalDevice, 1, &descriptor_write, 0, 0);
}