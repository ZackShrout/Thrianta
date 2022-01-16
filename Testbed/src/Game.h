#pragma once

#include <Defines.h>
#include <GameTypes.h>

typedef struct game_state {
    f32 deltaTime;
} game_state;

b8 GameInitialize(game* gameInst);

b8 GameUpdate(game* gameInst, f32 deltaTime);

b8 GameRender(game* gameInst, f32 deltaTime);

void GameOnResize(game* gameInst, u32 width, u32 height);