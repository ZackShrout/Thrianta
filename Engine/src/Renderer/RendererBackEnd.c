#include "RendererBackEnd.h"
#include "Vulkan/VulkanBackEnd.h"

b8 RendererBackendCreate(renderer_backend_type type, renderer_backend* outRendererBackend)
{
    if (type == RENDERER_BACKEND_TYPE_VULKAN)
    {
        outRendererBackend->initialize = VulkanRendererBackendInitialize;
        outRendererBackend->shutdown = VulkanRendererBackendShutdown;
        outRendererBackend->begin_frame = VulkanRendererBackendBeginFrame;
        outRendererBackend->update_global_state = VulkanRendererUpdateGlobalState;
        outRendererBackend->end_frame = VulkanRendererBackendEndFrame;
        outRendererBackend->resized = VulkanRendererBackendOnResize;
        outRendererBackend->update_object = VulkanBackendUpdateObject;
        outRendererBackend->create_texture = VulkanRendererCreateTexture;
        outRendererBackend->destroy_texture = VulkanRendererDestroyTexture;

        return true;
    }

    return false;
}

void RendererBackendDestroy(renderer_backend* rendererBackend)
{
    rendererBackend->initialize = 0;
    rendererBackend->shutdown = 0;
    rendererBackend->begin_frame = 0;
    rendererBackend->update_global_state = 0;
    rendererBackend->end_frame = 0;
    rendererBackend->resized = 0;
    rendererBackend->update_object = 0;
    rendererBackend->create_texture = 0;
    rendererBackend->destroy_texture = 0;
}