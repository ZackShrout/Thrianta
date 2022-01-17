#pragma once
#include "RendererTypes.inl"

struct static_mesh_data;
struct platform_state;

b8 RendererInitialize(const char* applicationName, struct platform_state* platState);
void RendererShutdown();

void RendererOnResized(u16 width, u16 height);

b8 RendererDrawFrame(render_packet* packet);