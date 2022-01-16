#include "Application.h"
#include "GameTypes.h"
#include "Logger.h"
#include "Platform/Platform.h"

typedef struct application_state
{
    game* gameInst;
    b8 isRunning;
    b8 isSuspended;
    platform_state platform;
    s16 width;
    s16 height;
    f64 lastTime;
} application_state;

static b8 initialized = FALSE;
static application_state appState;

b8 ApplicationCreate(game* gameInst)
{
    if (initialized)
    {
        TERROR("ApplicationCreate() called more than once.");
        return FALSE;
    }

    appState.gameInst = gameInst;

    // Initialize subsystems.
    InitializeLogging();

    // TODO: Remove this
    TFATAL("A test message: %f", 3.14f);
    TERROR("A test message: %f", 3.14f);
    TWARN("A test message: %f", 3.14f);
    TINFO("A test message: %f", 3.14f);
    TDEBUG("A test message: %f", 3.14f);
    TTRACE("A test message: %f", 3.14f);

    appState.isRunning = TRUE;
    appState.isSuspended = FALSE;

    if (!PlatformStartup(
            &appState.platform,
            gameInst->appConfig.name,
            gameInst->appConfig.startPosX,
            gameInst->appConfig.startPosY,
            gameInst->appConfig.startWidth,
            gameInst->appConfig.startHeight))
    {
        return FALSE;
    }

    // Initialize the game.
    if (!appState.gameInst->Initialize(appState.gameInst)) {
        TFATAL("Game failed to initialize.");
        return FALSE;
    }

    appState.gameInst->OnResize(appState.gameInst, appState.width, appState.height);

    initialized = TRUE;

    return TRUE;
}

b8 ApplicationRun()
{
    while (appState.isRunning)
    {
        if(!PlatformPumpMessages(&appState.platform))
        {
            appState.isRunning = FALSE;
        }

        if(!appState.isSuspended)
        {
            if (!appState.gameInst->Update(appState.gameInst, (f32)0))
            {
                TFATAL("Game update failed, shutting down.");
                appState.isRunning = FALSE;
                break;
            }

            // Call the game's render routine.
            if (!appState.gameInst->Render(appState.gameInst, (f32)0))
            {
                TFATAL("Game render failed, shutting down.");
                appState.isRunning = FALSE;
                break;
            }
        }
    }

    appState.isRunning = FALSE;

    PlatformShutdown(&appState.platform);

    return TRUE;
}