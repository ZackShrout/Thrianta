#include "Game.h"

#include <Core/Logger.h>
#include <Core/TMemory.h>
#include <Core/Input.h>

b8 GameInitialize(game* gameInst)
{
    TDEBUG("GameInitialize() called!");
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
    
    return true;
}

b8 GameRender(game* gameInst, f32 dt)
{
    return true;
}

void GameOnResize(game* gameInst, u32 width, u32 height) {}