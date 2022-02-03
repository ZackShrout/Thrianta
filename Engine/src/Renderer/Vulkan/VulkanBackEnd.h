#pragma once
#include "Renderer/RendererBackEnd.h"
#include "Resources/ResourceTypes.h"

b8 VulkanRendererBackendInitialize(renderer_backend* backend, const char* applicationName);
void VulkanRendererBackendShutdown(renderer_backend* backend);
void VulkanRendererBackendOnResize(renderer_backend* backend, u16 width, u16 height);
b8 VulkanRendererBackendBeginFrame(renderer_backend* backend, f32 dt);
void VulkanRendererUpdateGlobalState(mat4 projection, mat4 view, vec3 view_position, vec4 ambient_colour, s32 mode);
b8 VulkanRendererBackendEndFrame(renderer_backend* backend, f32 dt);
void VulkanBackendUpdateObject(mat4 model);
void VulkanRendererCreateTexture(const char* name, b8 auto_release, s32 width, s32 height, s32 channelCount, const u8* pixels, b8 hasTransparency, texture* outTexture);
void VulkanRendererDestroyTexture(texture* texture);