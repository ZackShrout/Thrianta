#include "RendererFrontEnd.h"
#include "RendererBackEnd.h"
#include "Core/Logger.h"
#include "Core/TMemory.h"

struct platform_state;

// Backend render context.
static renderer_backend* backend = 0;

b8 RendererInitialize(const char* applicationName, struct platform_state* platState)
{
    backend = TAllocate(sizeof(renderer_backend), MEMORY_TAG_RENDERER);

    // TODO: make this configurable.
    RendererBackendCreate(RENDERER_BACKEND_TYPE_VULKAN, platState, backend);
    backend->frameNumber = 0;

    if (!backend->initialize(backend, applicationName, platState))
    {
        TFATAL("Renderer backend failed to initialize. Shutting down.");
        return false;
    }

    return true;
}

void RendererShutdown()
{
    backend->shutdown(backend);
    TFree(backend, sizeof(renderer_backend), MEMORY_TAG_RENDERER);
}

b8 RendererBeginFrame(f32 dt)
{
    return backend->begin_frame(backend, dt);
}

b8 RendererEndFrame(f32 dt)
{
    b8 result = backend->end_frame(backend, dt);
    backend->frameNumber++;
    return result;
}

void RendererOnResized(u16 width, u16 height) {
    if (backend)
    {
        backend->resized(backend, width, height);
    }
    else
    {
        TWARN("renderer backend does not exist to accept resize: %i %i", width, height);
    }
}

b8 RendererDrawFrame(render_packet* packet)
{
    // If the begin frame returned successfully, mid-frame operations may continue.
    if (RendererBeginFrame(packet->dt))
    {
        // TODO:
        // ...
        // ...
        // End the frame. If this fails, it is likely unrecoverable.
        b8 result = RendererEndFrame(packet->dt);

        if (!result)
        {
            TERROR("RendererEndFrame() failed. Application shutting down...");
            return false;
        }
    }

    return true;
}