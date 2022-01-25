#include "Application.h"
#include "GameTypes.h"
#include "Logger.h"
#include "Platform/Platform.h"
#include "Core/TMemory.h"
#include "Core/Event.h"
#include "Core/Input.h"
#include "Core/Clock.h"
#include "Renderer/RendererFrontEnd.h"

typedef struct application_state
{
    game* gameInst;
    b8 isRunning;
    b8 isSuspended;
    platform_state platform;
    s16 width;
    s16 height;
    clock clock;
    f64 lastTime;
} application_state;

static b8 initialized = false;
static application_state appState;

// Event handlers
b8 ApplicationOnEvent(u16 code, void* sender, void* listenerInst, event_context context);
b8 ApplicationOnKey(u16 code, void* sender, void* listenerInst, event_context context);
b8 ApplicationOnResized(u16 code, void* sender, void* listenerInst, event_context context);

b8 ApplicationCreate(game* gameInst)
{
    if (initialized)
    {
        TERROR("ApplicationCreate() called more than once.");
        return false;
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

    appState.isRunning = true;
    appState.isSuspended = false;

    if(!EventInitialize())
    {
        TERROR("Event system failed initialization. Application cannot continue.");
        return false;
    }

    EventRegister(EVENT_CODE_APPLICATION_QUIT, 0, ApplicationOnEvent);
    EventRegister(EVENT_CODE_KEY_PRESSED, 0, ApplicationOnKey);
    EventRegister(EVENT_CODE_KEY_RELEASED, 0, ApplicationOnKey);
    EventRegister(EVENT_CODE_RESIZED, 0, ApplicationOnResized);

    if (!PlatformStartup(
            &appState.platform,
            gameInst->appConfig.name,
            gameInst->appConfig.startPosX,
            gameInst->appConfig.startPosY,
            gameInst->appConfig.startWidth,
            gameInst->appConfig.startHeight))
    {
        return false;
    }

    // Renderer startup
    if (!RendererInitialize(gameInst->appConfig.name, &appState.platform)) {
        TFATAL("Failed to initialize renderer. Aborting application.");
        return false;
    }

    // Initialize the game.
    if (!appState.gameInst->Initialize(appState.gameInst)) {
        TFATAL("Game failed to initialize.");
        return false;
    }

    appState.gameInst->OnResize(appState.gameInst, appState.width, appState.height);

    initialized = true;

    return true;
}

b8 ApplicationRun()
{
    ClockStart(&appState.clock);
    ClockUpdate(&appState.clock);
    appState.lastTime = appState.clock.elapsed;
    f64 runningTime = 0;
    u8 frameCount = 0;
    f64 targetFrameSeconds = 1.0f / 60;
    
    TINFO(GetMemoryUsageStr());

    while (appState.isRunning)
    {
        if(!PlatformPumpMessages(&appState.platform))
        {
            appState.isRunning = false;
        }

        if(!appState.isSuspended)
        {
            ClockUpdate(&appState.clock);
            f64 currentTime = appState.clock.elapsed;
            f64 delta = currentTime - appState.lastTime;
            f64 frameStartTime = PlatformGetAbsoluteTime();
            
            if (!appState.gameInst->Update(appState.gameInst, (f32)delta))
            {
                TFATAL("Game update failed, shutting down.");
                appState.isRunning = false;
                break;
            }

            // Call the game's render routine.
            if (!appState.gameInst->Render(appState.gameInst, (f32)delta))
            {
                TFATAL("Game render failed, shutting down.");
                appState.isRunning = false;
                break;
            }

            // TODO: refactor packet creation
            render_packet packet;
            packet.dt = delta;
            RendererDrawFrame(&packet);

            // Figure our how long the frame took and, if below...
            f64 frameEndTime = PlatformGetAbsoluteTime();
            f64 frameElapsedTime = frameEndTime - frameStartTime;
            runningTime += frameElapsedTime;
            f64 remainingSeconds = targetFrameSeconds - frameElapsedTime;

            if (remainingSeconds > 0)
            {
                u64 remainingMs = remainingSeconds * 1000;

                // If there is time left, give it back to the OS
                b8 limitFrames = false;
                if (remainingMs > 0 && limitFrames)
                {
                    PlatformSleep(remainingMs - 1);
                }

                frameCount++;
            }

            // NOTE: Input update/state copying should always be handled
            // after any input should be recorded; I.E. before this line.
            // As a safety, input is the last thing to be updated before
            // this frame ends.
            InputUpdate(delta);

            // Update last time
            appState.lastTime = currentTime;
        }
    }

    appState.isRunning = false;

    // Shutdown event system.
    EventUnregister(EVENT_CODE_APPLICATION_QUIT, 0, ApplicationOnEvent);
    EventUnregister(EVENT_CODE_KEY_PRESSED, 0, ApplicationOnKey);
    EventUnregister(EVENT_CODE_KEY_RELEASED, 0, ApplicationOnKey);
    EventUnregister(EVENT_CODE_RESIZED, 0, ApplicationOnResized);
    EventShutdown();
    InputShutdown();
    RendererShutdown();
    PlatformShutdown(&appState.platform);

    return true;
}

void ApplicationGetFramebufferSize(u32* width, u32* height) {
    *width = appState.width;
    *height = appState.height;
}

b8 ApplicationOnEvent(u16 code, void* sender, void* listenerInst, event_context context) {
    switch (code)
    {
        case EVENT_CODE_APPLICATION_QUIT:
        {
            TINFO("EVENT_CODE_APPLICATION_QUIT recieved, shutting down.\n");
            appState.isRunning = false;
            return true;
        }
    }

    return false;
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
            return true;
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
    return false;
}

b8 ApplicationOnResized(u16 code, void* sender, void* listenerInst, event_context context) {
    if (code == EVENT_CODE_RESIZED)
    {
        u16 width = context.data.u16[0];
        u16 height = context.data.u16[1];

        // Check if different. If so, trigger a resize event.
        if (width != appState.width || height != appState.height)
        {
            appState.width = width;
            appState.height = height;

            TDEBUG("Window resize: %i, %i", width, height);

            // Handle minimization
            if (width == 0 || height == 0)
            {
                TINFO("Window minimized, suspending application.");
                appState.isSuspended = true;
                return true;
            }
            else
            {
                if (appState.isSuspended)
                {
                    TINFO("Window restored, resuming application.");
                    appState.isSuspended = false;
                }
                appState.gameInst->OnResize(appState.gameInst, width, height);
                RendererOnResized(width, height);
            }
        }
    }

    // Event purposely not handled to allow other listeners to get this.
    return false;
}