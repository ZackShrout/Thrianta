#pragma once
#include "RendererTypes.inl"

struct platform_state;

b8 RendererBackendCreate(renderer_backend_type type, struct platform_state* platState, renderer_backend* outRendererBackend);
void RendererBackendDestroy(renderer_backend* rendererBackend);