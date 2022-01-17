#pragma once
#include "Renderer/RendererBackEnd.h"

b8 VulkanRendererBackendInitialize(renderer_backend* backend, const char* applicationName, struct platform_state* platState);
void VulkanRendererBackendShutdown(renderer_backend* backend);

void VulkanRendererBackendOnResize(renderer_backend* backend, u16 width, u16 height);

b8 VulkanRendererBackendBeginFrame(renderer_backend* backend, f32 dt);
b8 VulkanRendererBackendEndFrame(renderer_backend* backend, f32 dt);