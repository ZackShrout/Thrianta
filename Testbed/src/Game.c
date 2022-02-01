#include "Game.h"

#include <Core/Logger.h>
#include <Core/TMemory.h>
#include <Core/Input.h>
#include <Math/TMath.h>

// HACK: This should not be available outside the engine.
#include <Renderer/RendererFrontEnd.h>

void RecalculateViewMatrix(game_state* state)
{
    if (state->cameraViewDirty)
    {
        mat4 rotation = mat4_euler_xyz(state->cameraEuler.x, state->cameraEuler.y, state->cameraEuler.z);
        mat4 translation = mat4_translation(state->cameraPosition);

        state->view = mat4_mul(rotation, translation);
        state->view = mat4_inverse(state->view);

        state->cameraViewDirty = false;
    }
}

void CameraYaw(game_state* state, f32 amount)
{
    state->cameraEuler.y += amount;
    state->cameraViewDirty = true;
}

void CameraPitch(game_state* state, f32 amount)
{
    state->cameraEuler.x += amount;

    // Clamp to avoid Gimball lock.
    f32 limit = deg_to_rad(89.0f);
    state->cameraEuler.x = TCLAMP(state->cameraEuler.x, -limit, limit);

    state->cameraViewDirty = true;
}

b8 GameInitialize(game* gameInst)
{
    TDEBUG("GameInitialize() called!");

    game_state* state = (game_state*)gameInst->state;

    state->cameraPosition = (vec3){0, 0, 30.0f};
    state->cameraEuler = vec3_zero();

    state->view = mat4_translation(state->cameraPosition);
    state->view = mat4_inverse(state->view);
    state->cameraViewDirty = true;

    return true;
}

b8 GameUpdate(game* gameInst, f32 dt)
{
    static u64 allocCount = 0;
    u64 previousAllocCount = allocCount;
    allocCount = GetMemoryAllocCount();
    if (InputIsKeyUp('M') && InputWasKeyDown('M'))
    {
        TDEBUG("Allocations: %llu (%llu this frame).", allocCount, allocCount - previousAllocCount);
    }

    game_state* state = (game_state*)gameInst->state;

    // HACK: temp hack to move camera around.
    if (InputIsKeyDown('A') || InputIsKeyDown(KEY_LEFT))
    {
        CameraYaw(state, 1.0f * dt);
    }

    if (InputIsKeyDown('D') || InputIsKeyDown(KEY_RIGHT))
    {
        CameraYaw(state, -1.0f * dt);
    }

    if (InputIsKeyDown(KEY_UP))
    {
        CameraPitch(state, 1.0f * dt);
    }

    if (InputIsKeyDown(KEY_DOWN))
    {
        CameraPitch(state, -1.0f * dt);
    }

    f32 tempMoveSpeed = 50.0f;
    vec3 velocity = vec3_zero();

    if (InputIsKeyDown('W'))
    {
        vec3 forward = mat4_forward(state->view);
        velocity = vec3_add(velocity, forward);
    }

    if (InputIsKeyDown('S'))
    {
        vec3 backward = mat4_backward(state->view);
        velocity = vec3_add(velocity, backward);
    }

    if (InputIsKeyDown('Q'))
    {
        vec3 left = mat4_left(state->view);
        velocity = vec3_add(velocity, left);
    }

    if (InputIsKeyDown('E'))
    {
        vec3 right = mat4_right(state->view);
        velocity = vec3_add(velocity, right);
    }

    if (InputIsKeyDown(KEY_SPACE))
    {
        velocity.y += 1.0f;
    }

    if (InputIsKeyDown('X'))
    {
        velocity.y -= 1.0f;
    }

    vec3 z = vec3_zero();
    if (!vec3_compare(z, velocity, 0.0002f))
    {
        // Be sure to normalize the velocity before applying speed.
        vec3_normalize(&velocity);
        state->cameraPosition.x += velocity.x * tempMoveSpeed * dt;
        state->cameraPosition.y += velocity.y * tempMoveSpeed * dt;
        state->cameraPosition.z += velocity.z * tempMoveSpeed * dt;
        state->cameraViewDirty = true;
    }

    RecalculateViewMatrix(state);

    // HACK: This should not be available outside the engine.
    RendererSetView(state->view);
    
    return true;
}

b8 GameRender(game* gameInst, f32 dt)
{
    return true;
}

void GameOnResize(game* gameInst, u32 width, u32 height) {}