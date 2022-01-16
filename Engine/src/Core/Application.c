#include "Application.h"
#include "GameTypes.h"
#include "Logger.h"
#include "Platform/Platform.h"
#include "Core/TMemory.h"
#include "Core/Event.h"
#include "Core/Input.h"

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

// Event handlers
b8 ApplicationOnEvent(u16 code, void* sender, void* listenerInst, event_context context);
b8 ApplicationOnKey(u16 code, void* sender, void* listenerInst, event_context context);

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
    InputInitialize();

    // TODO: Remove this
    TFATAL("A test message: %f", 3.14f);
    TERROR("A test message: %f", 3.14f);
    TWARN("A test message: %f", 3.14f);
    TINFO("A test message: %f", 3.14f);
    TDEBUG("A test message: %f", 3.14f);
    TTRACE("A test message: %f", 3.14f);

    appState.isRunning = TRUE;
    appState.isSuspended = FALSE;

    if(!EventInitialize())
    {
        TERROR("Event system failed initialization. Application cannot continue.");
        return FALSE;
    }

    EventRegister(EVENT_CODE_APPLICATION_QUIT, 0, ApplicationOnEvent);
    EventRegister(EVENT_CODE_KEY_PRESSED, 0, ApplicationOnKey);
    EventRegister(EVENT_CODE_KEY_RELEASED, 0, ApplicationOnKey);

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
    TINFO(GetMemoryUsageStr());

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

            // NOTE: Input update/state copying should always be handled
            // after any input should be recorded; I.E. before this line.
            // As a safety, input is the last thing to be updated before
            // this frame ends.
            InputUpdate(0);
        }
    }

    appState.isRunning = FALSE;

    // Shutdown event system.
    EventUnregister(EVENT_CODE_APPLICATION_QUIT, 0, ApplicationOnEvent);
    EventUnregister(EVENT_CODE_KEY_PRESSED, 0, ApplicationOnKey);
    EventUnregister(EVENT_CODE_KEY_RELEASED, 0, ApplicationOnKey);
    EventShutdown();
    InputShutdown();
    PlatformShutdown(&appState.platform);

    return TRUE;
}

b8 ApplicationOnEvent(u16 code, void* sender, void* listenerInst, event_context context) {
    switch (code)
    {
        case EVENT_CODE_APPLICATION_QUIT:
        {
            TINFO("EVENT_CODE_APPLICATION_QUIT recieved, shutting down.\n");
            appState.isRunning = FALSE;
            return TRUE;
        }
    }

    return FALSE;
}

b8 ApplicationOnKey(u16 code, void* sender, void* listenerInst, event_context context) {
    if (code == EVENT_CODE_KEY_PRESSED)
    {
        u16 keyCode = context.data.u16[0];
        if (keyCode == KEY_ESCAPE)
        {
            // NOTE: Technically firing an event to itself, but there may be other listeners.
            event_context data = {};
            EventFire(EVENT_CODE_APPLICATION_QUIT, 0, data);

            // Block anything else from processing this.
            return TRUE;
        }
        else if (keyCode == KEY_A)
        {
            // Example on checking for a key
            TDEBUG("Explicit - A key pressed!");
        }
        else
        {
            TDEBUG("'%c' key pressed in window.", keyCode);
        }
    }
    else if (code == EVENT_CODE_KEY_RELEASED)
    {
        u16 keyCode = context.data.u16[0];
        if (keyCode == KEY_B)
        {
            // Example on checking for a key
            TDEBUG("Explicit - B key released!");
        }
        else
        {
            TDEBUG("'%c' key released in window.", keyCode);
        }
    }
    return FALSE;
}