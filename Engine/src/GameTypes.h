#pragma once

#include "Core/Application.h"

/**
 * Represents the basic game state in a game.
 * Called for creation by the application.
 */
typedef struct game {
    // The application configuration.
    application_config appConfig;

    // Function pointer to game's initialize function.
    b8 (*Initialize)(struct game* gameInst);

    // Function pointer to game's update function.
    b8 (*Update)(struct game* gameInst, f32 dt);

    // Function pointer to game's render function.
    b8 (*Render)(struct game* gameInst, f32 dt);

    // Function pointer to handle resizes, if applicable.
    void (*OnResize)(struct game* gameInst, u32 width, u32 height);

    // Game-specific game state. Created and managed by the game.
    void* state;
} game;