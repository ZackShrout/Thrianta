#include "Game.h"

#include <Core/Logger.h>

b8 GameInitialize(game* gameInst)
{
    TDEBUG("GameInitialize() called!");
    return true;
}

b8 GameUpdate(game* gameInst, f32 dt)
{
    return true;
}

b8 GameRender(game* gameInst, f32 dt)
{
    return true;
}

void GameOnResize(game* gameInst, u32 width, u32 height) {}