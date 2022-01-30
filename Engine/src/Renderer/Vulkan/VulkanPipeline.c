#include "VulkanPipeline.h"
#include "VulkanUtils.h"
#include "Core/TMemory.h"
#include "Core/Logger.h"
#include "Math/MathTypes.h"

b8 VulkanGraphicsPipelineCreate(
    vulkan_context* context,
    vulkan_renderpass* renderpass,
    u32 attributeCount,
    VkVertexInputAttributeDescription* attributes,
    u32 descSetLayoutCount,
    VkDescriptorSetLayout* descSetLayouts,
    u32 stageCount,
    VkPipelineShaderStageCreateInfo* stages,
    VkViewport viewport,
    VkRect2D scissor,
    b8 isWireframe,
    vulkan_pipeline* outPipeline)
{
    // Viewport state
    VkPipelineViewportStateCreateInfo viewportState = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterizerCreateInfo.depthClampEnable = VK_FALSE;
    rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerCreateInfo.polygonMode = isWireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    rasterizerCreateInfo.lineWidth = 1.0f;
    rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizerCreateInfo.depthBiasConstantFactor = 0.0f;
    rasterizerCreateInfo.depthBiasClamp = 0.0f;
    rasterizerCreateInfo.depthBiasSlopeFactor = 0.0f;

    // Multisampling.
    VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;
    multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisamplingCreateInfo.minSampleShading = 1.0f;
    multisamplingCreateInfo.pSampleMask = 0;
    multisamplingCreateInfo.alphaToCoverageEnable = VK_FALSE;
    multisamplingCreateInfo.alphaToOneEnable = VK_FALSE;

    // Depth and stencil testing.
    VkPipelineDepthStencilStateCreateInfo depthStencil = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachState;
    TZeroMemory(&colorBlendAttachState, sizeof(VkPipelineColorBlendAttachmentState));
    colorBlendAttachState.blendEnable = VK_TRUE;
    colorBlendAttachState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachState.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachState.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendStateCreateInfo.attachmentCount = 1;
    colorBlendStateCreateInfo.pAttachments = &colorBlendAttachState;

    // Dynamic state
    const u32 dynamicStateCount = 3;
    VkDynamicState dynamic_states[dynamicStateCount] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_LINE_WIDTH};

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamicStateCreateInfo.dynamicStateCount = dynamicStateCount;
    dynamicStateCreateInfo.pDynamicStates = dynamic_states;

    // Vertex input
    VkVertexInputBindingDescription bindingDesc;
    bindingDesc.binding = 0;  // Binding index
    bindingDesc.stride = sizeof(vertex_3d);
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;  // Move to next data entry for each vertex.

    // Attributes
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = attributeCount;
    vertexInputInfo.pVertexAttributeDescriptions = attributes;

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

    // Descriptor set layouts
    pipelineLayoutCreateInfo.setLayoutCount = descSetLayoutCount;
    pipelineLayoutCreateInfo.pSetLayouts = descSetLayouts;

    // Create the pipeline layout.
    VK_CHECK(vkCreatePipelineLayout(
        context->device.logicalDevice,
        &pipelineLayoutCreateInfo,
        context->allocator,
        &outPipeline->pipelineLayout));

    // Pipeline create
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipelineCreateInfo.stageCount = stageCount;
    pipelineCreateInfo.pStages = stages;
    pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssembly;

    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
    pipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
    pipelineCreateInfo.pDepthStencilState = &depthStencil;
    pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
    pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
    pipelineCreateInfo.pTessellationState = 0;

    pipelineCreateInfo.layout = outPipeline->pipelineLayout;

    pipelineCreateInfo.renderPass = renderpass->handle;
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = -1;

    VkResult result = vkCreateGraphicsPipelines(
        context->device.logicalDevice,
        VK_NULL_HANDLE,
        1,
        &pipelineCreateInfo,
        context->allocator,
        &outPipeline->handle);

    if (VulkanResultIsSuccess(result))
    {
        TDEBUG("Graphics pipeline created!");
        return true;
    }

    TERROR("vkCreateGraphicsPipelines failed with %s.", VulkanResultString(result, true));
    return false;
}

void VulkanPipelineDestroy(vulkan_context* context, vulkan_pipeline* pipeline) {
    if (pipeline)
    {
        // Destroy pipeline
        if (pipeline->handle)
        {
            vkDestroyPipeline(context->device.logicalDevice, pipeline->handle, context->allocator);
            pipeline->handle = 0;
        }

        // Destroy layout
        if (pipeline->pipelineLayout)
        {
            vkDestroyPipelineLayout(context->device.logicalDevice, pipeline->pipelineLayout, context->allocator);
            pipeline->pipelineLayout = 0;
        }
    }
}

void VulkanPipelineBind(vulkan_command_buffer* commandBuffer, VkPipelineBindPoint bindPoint, vulkan_pipeline* pipeline)
{
    vkCmdBindPipeline(commandBuffer->handle, bindPoint, pipeline->handle);
}