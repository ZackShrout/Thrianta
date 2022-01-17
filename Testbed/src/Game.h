#pragma once

#include <Defines.h>
#include <GameTypes.h>

typedef struct game_state {
    f32 dt;
} game_state;

b8 GameInitialize(game* gameInst);

b8 GameUpdate(game* gameInst, f32 dt);

b8 GameRender(game* gameInst, f32 dt);

void GameOnResize(game* gameInst, u32 width, u32 height);