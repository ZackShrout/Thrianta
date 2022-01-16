#pragma once

#include "Core/Application.h"
#include "Core/Logger.h"
#include "GameTypes.h"

// Externally-defined function to create a game.
extern b8 CreateGame(game* outGame);

/**
 * The main entry point of the application.
 */
int main(void) {
    // Request the game instance from the application.
    game gameInst;
    if (!CreateGame(&gameInst)) {
        TFATAL("Could not create game!");
        return -1;
    }

    // Ensure the function pointers exist.
    if (!gameInst.Render || !gameInst.Update || !gameInst.Initialize || !gameInst.OnResize) {
        TFATAL("The game's function pointers must be assigned!");
        return -2;
    }

    // Initialization.
    if (!ApplicationCreate(&gameInst)) {
        TINFO("Application failed to create!.");
        return 1;
    }

    // Begin the game loop.
    if(!ApplicationRun()) {
        TINFO("Application did not shutdown gracefully.");
        return 2;
    }

    return 0;
}