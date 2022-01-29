#include "RendererBackEnd.h"
#include "Vulkan/VulkanBackEnd.h"

b8 RendererBackendCreate(renderer_backend_type type, renderer_backend* outRendererBackend)
{
    if (type == RENDERER_BACKEND_TYPE_VULKAN)
    {
        outRendererBackend->initialize = VulkanRendererBackendInitialize;
        outRendererBackend->shutdown = VulkanRendererBackendShutdown;
        outRendererBackend->begin_frame = VulkanRendererBackendBeginFrame;
        outRendererBackend->end_frame = VulkanRendererBackendEndFrame;
        outRendererBackend->resized = VulkanRendererBackendOnResize;

        return true;
    }

    return false;
}

void RendererBackendDestroy(renderer_backend* rendererBackend)
{
    rendererBackend->initialize = 0;
    rendererBackend->shutdown = 0;
    rendererBackend->begin_frame = 0;
    rendererBackend->end_frame = 0;
    rendererBackend->resized = 0;
}