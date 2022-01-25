#include "Game.h"
#include <Entry.h>
#include <Core/TMemory.h>

// Define the function to create a game
b8 CreateGame(game* outGame)
{
    // Application configuration.
    outGame->appConfig.startPosX = 100;
    outGame->appConfig.startPosY = 100;
    outGame->appConfig.startWidth = 1280;
    outGame->appConfig.startHeight = 720;
    outGame->appConfig.name = "Thrianta Engine Testbed";
    outGame->Update = GameUpdate;
    outGame->Render = GameRender;
    outGame->Initialize = GameInitialize;
    outGame->OnResize = GameOnResize;

    // Create the game state.
    outGame->state = TAllocate(sizeof(game_state), MEMORY_TAG_GAME);

    return true;
}