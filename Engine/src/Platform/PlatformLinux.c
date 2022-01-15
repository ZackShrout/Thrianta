#include "Platform.h"

// Linux platform layer.
#if TPLATFORM_LINUX

#include "Core/Logger.h"

#include <xcb/xcb.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>  // sudo apt-get install libx11-dev
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>  // sudo apt-get install libxkbcommon-x11-dev
#include <sys/time.h>

#if _POSIX_C_SOURCE >= 199309L
#include <time.h>  // nanosleep
#else
#include <unistd.h>  // usleep
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct internal_state
{
    Display* display;
    xcb_connection_t* connection;
    xcb_window_t window;
    xcb_screen_t* screen;
    xcb_atom_t wm_protocols;
    xcb_atom_t wm_delete_win;
} internal_state;

b8 PlatformStartup(
    platform_state* platState,
    const char* applicationName,
    s32 x,
    s32 y,
    s32 width,
    s32 height)
{
    // Create the internal state.
    platState->internalState = malloc(sizeof(internal_state));
    internal_state* state = (internal_state*)platState->internalState;

    // Connect to X
    state->display = XOpenDisplay(NULL);

    // Turn off key repeats.
    XAutoRepeatOff(state->display);

    // Retrieve the connection from the display.
    state->connection = XGetXCBConnection(state->display);

    if (xcb_connection_has_error(state->connection)) {
        TFATAL("Failed to connect to X server via XCB.");
        return FALSE;
    }

    // Get data from the X server
    const struct xcb_setup_t* setup = xcb_get_setup(state->connection);

    // Loop through screens using iterator
    xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);
    int screen_p = 0;
    for (s32 s = screen_p; s > 0; s--) {
        xcb_screen_next(&it);
    }

    // After screens have been looped through, assign it.
    state->screen = it.data;

    // Allocate a XID for the window to be created.
    state->window = xcb_generate_id(state->connection);

    // Register event types.
    // XCB_CW_BACK_PIXEL = filling then window bg with a single colour
    // XCB_CW_EVENT_MASK is required.
    u32 eventMask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

    // Listen for keyboard and mouse buttons
    u32 eventValues = XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
                       XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
                       XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_POINTER_MOTION |
                       XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    // Values to be sent over XCB (bg colour, events)
    u32 valueList[] = { state->screen->black_pixel, eventValues };

    // Create the window
    xcb_void_cookie_t cookie = xcb_create_window(
        state->connection,
        XCB_COPY_FROM_PARENT,  // depth
        state->window,
        state->screen->root,            // parent
        x,                              //x
        y,                              //y
        width,                          //width
        height,                         //height
        0,                              // No border
        XCB_WINDOW_CLASS_INPUT_OUTPUT,  //class
        state->screen->root_visual,
        eventMask,
        valueList);

    // Change the title
    xcb_change_property(
        state->connection,
        XCB_PROP_MODE_REPLACE,
        state->window,
        XCB_ATOM_WM_NAME,
        XCB_ATOM_STRING,
        8,  // data should be viewed 8 bits at a time
        strlen(applicationName),
        applicationName);

    // Tell the server to notify when the window manager
    // attempts to destroy the window.
    xcb_intern_atom_cookie_t wm_delete_cookie = xcb_intern_atom(
        state->connection,
        0,
        strlen("WM_DELETE_WINDOW"),
        "WM_DELETE_WINDOW");
    xcb_intern_atom_cookie_t wm_protocols_cookie = xcb_intern_atom(
        state->connection,
        0,
        strlen("WM_PROTOCOLS"),
        "WM_PROTOCOLS");
    xcb_intern_atom_reply_t* wm_delete_reply = xcb_intern_atom_reply(
        state->connection,
        wm_delete_cookie,
        NULL);
    xcb_intern_atom_reply_t* wm_protocols_reply = xcb_intern_atom_reply(
        state->connection,
        wm_protocols_cookie,
        NULL);
    state->wm_delete_win = wm_delete_reply->atom;
    state->wm_protocols = wm_protocols_reply->atom;

    xcb_change_property(
        state->connection,
        XCB_PROP_MODE_REPLACE,
        state->window,
        wm_protocols_reply->atom,
        4,
        32,
        1,
        &wm_delete_reply->atom);

    // Map the window to the screen
    xcb_map_window(state->connection, state->window);

    // Flush the stream
    s32 stream_result = xcb_flush(state->connection);
    if (stream_result <= 0) {
        TFATAL("An error occurred when flusing the stream: %d", stream_result);
        return FALSE;
    }

    return TRUE;
}

void PlatformShutdown(platform_state* platState)
{
    // Simply cold-cast to the known type.
    internal_state* state = (internal_state*)platState->internalState;

    // Turn key repeats back on since this is global for the OS... just... wow.
    XAutoRepeatOn(state->display);

    xcb_destroy_window(state->connection, state->window);
}

b8 PlatformPumpMessages(platform_state* platState)
{
    // Simply cold-cast to the known type.
    internal_state* state = (internal_state*)platState->internalState;

    xcb_generic_event_t* event;
    xcb_client_message_event_t* cm;

    b8 quitFlagged = FALSE;

    // Poll for events until null is returned.
    while (event != 0)
    {
        event = xcb_poll_for_event(state->connection);
        if (event == 0)
        {
            break;
        }

        // Input events
        switch (event->response_type & ~0x80)
        {
            case XCB_KEY_PRESS:
            case XCB_KEY_RELEASE:
            {
                // TODO: Key presses and releases
            } break;
            case XCB_BUTTON_PRESS:
            case XCB_BUTTON_RELEASE:
            {
                // TODO: Mouse button presses and releases
            }
            case XCB_MOTION_NOTIFY:
                // TODO: mouse movement
                break;

            case XCB_CONFIGURE_NOTIFY:
            {
                // TODO: Resizing
            }

            case XCB_CLIENT_MESSAGE:
            {
                cm = (xcb_client_message_event_t*)event;

                // Window close
                if (cm->data.data32[0] == state->wm_delete_win)
                {
                    quitFlagged = TRUE;
                }
            } break;
            default:
                // Something else
                break;
        }

        free(event);
    }
    return !quitFlagged;
}

void* PlatformAllocate(u64 size, b8 aligned)
{
    return malloc(size);
}

void PlatformFree(void* block, b8 aligned)
{
    free(block);
}

void* PlatformZeroMemory(void* block, u64 size)
{
    return memset(block, 0, size);
}

void* PlatformCopyMemory(void* dest, const void* source, u64 size)
{
    return memcpy(dest, source, size);
}

void* PlatformSetMemory(void* dest, s32 value, u64 size)
{
    return memset(dest, value, size);
}

void PlatformConsoleWrite(const char* message, u8 colour)
{
    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    const char* colourStrings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
    printf("\033[%sm%s\033[0m", colourStrings[colour], message);
}

void PlatformConsoleWriteError(const char* message, u8 colour)
{
    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    const char* colourStrings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
    printf("\033[%sm%s\033[0m", colourStrings[colour], message);
}

f64 PlatformGetAbsoluteTime()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec + now.tv_nsec * 0.000000001;
}

void PlatformSleep(u64 ms)
{
#if _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000 * 1000;
    nanosleep(&ts, 0);
#else
    if (ms >= 1000)
    {
        sleep(ms / 1000);
    }
    usleep((ms % 1000) * 1000);
#endif
}

#endif // TPLATFORM_LINUX