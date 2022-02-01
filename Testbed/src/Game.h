#pragma once

#include <Defines.h>
#include <GameTypes.h>
#include <Math/MathTypes.h>

typedef struct game_state {
    f32 dt;
    mat4 view;
    vec3 cameraPosition;
    vec3 cameraEuler;
    b8 cameraViewDirty;
} game_state;

b8 GameInitialize(game* gameInst);

b8 GameUpdate(game* gameInst, f32 dt);

b8 GameRender(game* gameInst, f32 dt);

void GameOnResize(game* gameInst, u32 width, u32 height);