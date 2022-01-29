#include "RendererFrontEnd.h"
#include "RendererBackEnd.h"
#include "Core/Logger.h"
#include "Core/TMemory.h"

typedef struct renderer_system_state {
    renderer_backend backend;
} renderer_system_state;

static renderer_system_state* statePtr;

b8 RendererSystemInitialize(u64* memoryRequirement, void* state, const char* applicationName)
{
    *memoryRequirement = sizeof(renderer_system_state);
    if (state == 0) return true;

    statePtr = state;

    // TODO: make this configurable.
    RendererBackendCreate(RENDERER_BACKEND_TYPE_VULKAN, &statePtr->backend);
    statePtr->backend.frameNumber = 0;

    if (!statePtr->backend.initialize(&statePtr->backend, applicationName))
    {
        TFATAL("Renderer backend failed to initialize. Shutting down.");
        return false;
    }

    return true;
}

void RendererSystemShutdown(void* state)
{
    if (statePtr)
    {
        statePtr->backend.shutdown(&statePtr->backend);
    }

    statePtr = 0;
}

b8 RendererBeginFrame(f32 dt)
{
    if (!statePtr) return false;
    
    return statePtr->backend.begin_frame(&statePtr->backend, dt);
}

b8 RendererEndFrame(f32 dt)
{
    if (!statePtr) return false;
    
    b8 result = statePtr->backend.end_frame(&statePtr->backend, dt);
    statePtr->backend.frameNumber++;
    return result;
}

void RendererOnResized(u16 width, u16 height) {
    if (statePtr)
    {
        statePtr->backend.resized(&statePtr->backend, width, height);
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