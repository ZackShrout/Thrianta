#pragma once
#include "RendererTypes.inl"

struct static_mesh_data;
struct platform_state;

b8 RendererSystemInitialize(u64* memoryRequirement, void* state, const char* applicationName);
void RendererSystemShutdown(void* state);
void RendererOnResized(u16 width, u16 height);
b8 RendererDrawFrame(render_packet* packet);
void RendererCreateTexture(
    const char* name,
    b8 autoRelease,
    s32 width,
    s32 height,
    s32 channelCount,
    const u8* pixels,
    b8 hasTransparency,
    struct texture* outTexture);
void RendererDestroyTexture(struct texture* texture);

// HACK: this should not be exposed outside the engine.
TAPI void RendererSetView(mat4 view);