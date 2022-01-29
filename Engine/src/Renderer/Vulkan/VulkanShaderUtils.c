#include "VulkanShaderUtils.h"
#include "Core/Logger.h"
#include "Core/TString.h"
#include "Core/TMemory.h"
#include "Platform/Filesystem.h"

b8 CreateShaderModule(
    vulkan_context* context,
    const char* name,
    const char* typeStr,
    VkShaderStageFlagBits shaderStageFlag,
    u32 stageIndex,
    vulkan_shader_stage* shaderStages)
{
    // Build file name.
    char fileName[512];
    StringFormat(fileName, "assets/shaders/%s.%s.spv", name, typeStr);

    TZeroMemory(&shaderStages[stageIndex].create_info, sizeof(VkShaderModuleCreateInfo));
    shaderStages[stageIndex].create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    // Obtain file handle.
    file_handle handle;
    if (!FilesystemOpen(fileName, FILE_MODE_READ, true, &handle))
    {
        TERROR("Unable to read shader module: %s.", fileName);
        return false;
    }

    // Read the entire file as binary.
    u64 size = 0;
    u8* fileBuffer = 0;
    if (!FilesystemReadAllBytes(&handle, &fileBuffer, &size))
    {
        TERROR("Unable to binary read shader module: %s.", fileName);
        return false;
    }
    shaderStages[stageIndex].create_info.codeSize = size;
    shaderStages[stageIndex].create_info.pCode = (u32*)fileBuffer;

    // Close the file.
    FilesystemClose(&handle);

    VK_CHECK(vkCreateShaderModule(
        context->device.logicalDevice,
        &shaderStages[stageIndex].create_info,
        context->allocator,
        &shaderStages[stageIndex].handle));

    // Shader stage info
    TZeroMemory(&shaderStages[stageIndex].shader_stage_create_info, sizeof(VkPipelineShaderStageCreateInfo));
    shaderStages[stageIndex].shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[stageIndex].shader_stage_create_info.stage = shaderStageFlag;
    shaderStages[stageIndex].shader_stage_create_info.module = shaderStages[stageIndex].handle;
    shaderStages[stageIndex].shader_stage_create_info.pName = "main";

    if (fileBuffer)
    {
        TFree(fileBuffer, sizeof(u8) * size, MEMORY_TAG_STRING);
        fileBuffer = 0;
    }

    return true;
}