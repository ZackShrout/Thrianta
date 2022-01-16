#include "Game.h"

#include <Core/Logger.h>

b8 GameInitialize(game* gameInst)
{
    TDEBUG("GameInitialize() called!");
    return TRUE;
}

b8 GameUpdate(game* gameInst, f32 deltaTime)
{
    return TRUE;
}

b8 GameRender(game* gameInst, f32 deltaTime)
{
    return TRUE;
}

void GameOnResize(game* gameInst, u32 width, u32 height) {}