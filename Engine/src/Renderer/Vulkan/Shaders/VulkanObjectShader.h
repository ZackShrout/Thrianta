#pragma once
#include "Renderer/Vulkan/VulkanTypes.inl"
#include "Renderer/RendererTypes.inl"

b8 VulkanObjectShaderCreate(vulkan_context* context, vulkan_object_shader* outShader);
void VulkanObjectShaderDestroy(vulkan_context* context, struct vulkan_object_shader* shader);
void VulkanObjectShaderUse(vulkan_context* context, struct vulkan_object_shader* shader);