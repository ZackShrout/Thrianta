#pragma once

#include "Defines.h"
#include "Math/MathTypes.h"

typedef enum renderer_backend_type
{
    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_OPENGL,
    RENDERER_BACKEND_TYPE_DIRECTX,
    RENDERER_BACKEND_TYPE_METAL
} renderer_backend_type;

typedef struct global_uniform_object {
    mat4 projection;   // 64 bytes
    mat4 view;         // 64 bytes
    mat4 m_reserved0;  // 64 bytes, reserved for future use
    mat4 m_reserved1;  // 64 bytes, reserved for future use
} global_uniform_object;

typedef struct renderer_backend
{
    u64 frameNumber;
    b8 (*initialize)(struct renderer_backend* backend, const char* applicationName);
    void (*shutdown)(struct renderer_backend* backend);
    void (*resized)(struct renderer_backend* backend, u16 width, u16 height);
    b8 (*begin_frame)(struct renderer_backend* backend, f32 dt);
    void (*update_global_state)(mat4 projection, mat4 view, vec3 viewPosition, vec4 ambientColor, s32 mode);
    b8 (*end_frame)(struct renderer_backend* backend, f32 dt);   
    void (*update_object)(mat4 model);
} renderer_backend;

typedef struct render_packet
{
    f32 dt;
} render_packet;