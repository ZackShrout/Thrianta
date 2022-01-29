#pragma once
#include "RendererTypes.inl"

struct platform_state;

b8 RendererBackendCreate(renderer_backend_type type, renderer_backend* outRendererBackend);
void RendererBackendDestroy(renderer_backend* rendererBackend);