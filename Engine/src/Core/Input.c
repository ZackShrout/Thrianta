#include "Core/Input.h"
#include "Core/Event.h"
#include "Core/TMemory.h"
#include "Core/Logger.h"

typedef struct keyboard_state {
    b8 keys[256];
} keyboard_state;

typedef struct mouse_state {
    s16 x;
    s16 y;
    u8 buttons[BUTTON_MAX_BUTTONS];
} mouse_state;

typedef struct input_state {
    keyboard_state keyboardCurrent;
    keyboard_state keyboardPrevious;
    mouse_state mouseCurrent;
    mouse_state mousePrevious;
} input_state;

// Internal input state
static b8 initialized = FALSE;
static input_state state = {};

void InputInitialize()
{
    TZeroMemory(&state, sizeof(input_state));
    initialized = TRUE;
    TINFO("Input subsystem initialized.");
}

void InputShutdown()
{
    // TODO: Add shutdown routines when needed.
    initialized = FALSE;
}

void InputUpdate(f64 dt)
{
    if (!initialized)
    {
        return;
    }

    // Copy current states to previous states.
    TCopyMemory(&state.keyboardPrevious, &state.keyboardCurrent, sizeof(keyboard_state));
    TCopyMemory(&state.mousePrevious, &state.mouseCurrent, sizeof(mouse_state));
}

void InputProcessKey(keys key, b8 pressed)
{
    // Only handle this if the state actually changed.
    if (state.keyboardCurrent.keys[key] != pressed)
    {
        // Update internal state.
        state.keyboardCurrent.keys[key] = pressed;

        // Fire off an event for immediate processing.
        event_context context;
        context.data.u16[0] = key;
        EventFire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, 0, context);
    }
}

void InputProcessButton(buttons button, b8 pressed)
{
    // If the state changed, fire an event.
    if (state.mouseCurrent.buttons[button] != pressed)
    {
        state.mouseCurrent.buttons[button] = pressed;

        // Fire the event.
        event_context context;
        context.data.u16[0] = button;
        EventFire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, 0, context);
    }
}

void InputProcessMouseMove(s16 x, s16 y)
{
    // Only process if actually different
    if (state.mouseCurrent.x != x || state.mouseCurrent.y != y)
    {
        // NOTE: Enable this if debugging.
        // TDEBUG("Mouse pos: %i, %i!", x, y);

        // Update internal state.
        state.mouseCurrent.x = x;
        state.mouseCurrent.y = y;

        // Fire the event.
        event_context context;
        context.data.u16[0] = x;
        context.data.u16[1] = y;
        EventFire(EVENT_CODE_MOUSE_MOVED, 0, context);
    }
}

void InputProcessMouseWheel(s8 zDelta)
{
    // NOTE: no internal state to update.

    // Fire the event.
    event_context context;
    context.data.u8[0] = zDelta;
    EventFire(EVENT_CODE_MOUSE_WHEEL, 0, context);
}

// KEYBOARD INPUT
b8 InputIsKeyDown(keys key)
{
    if (!initialized)
    {
        return FALSE;
    }
    return state.keyboardCurrent.keys[key] == TRUE;
}

b8 InputIsKeyUp(keys key)
{
    if (!initialized)
    {
        return TRUE;
    }
    return state.keyboardCurrent.keys[key] == FALSE;
}

b8 InputWasKeyDown(keys key)
{
    if (!initialized)
    {
        return FALSE;
    }
    return state.keyboardPrevious.keys[key] == TRUE;
}

b8 InputWasKeyUp(keys key)
{
    if (!initialized)
    {
        return TRUE;
    }
    return state.keyboardPrevious.keys[key] == FALSE;
}

// MOUSE INPUT
b8 InputIsButtonDown(buttons button)
{
    if (!initialized)
    {
        return FALSE;
    }
    return state.mouseCurrent.buttons[button] == TRUE;
}

b8 InputIsButtonUp(buttons button)
{
    if (!initialized)
    {
        return TRUE;
    }
    return state.mouseCurrent.buttons[button] == FALSE;
}

b8 InputWasButtonDown(buttons button)
{
    if (!initialized)
    {
        return FALSE;
    }
    return state.mousePrevious.buttons[button] == TRUE;
}

b8 InputWasButtonUp(buttons button)
{
    if (!initialized)
    {
        return TRUE;
    }
    return state.mousePrevious.buttons[button] == FALSE;
}

void InputGetMousePosition(s32* x, s32* y)
{
    if (!initialized)
    {
        *x = 0;
        *y = 0;
        return;
    }
    *x = state.mouseCurrent.x;
    *y = state.mouseCurrent.y;
}

void InputGetPreviousMousePosition(s32* x, s32* y)
{
    if (!initialized)
    {
        *x = 0;
        *y = 0;
        return;
    }
    *x = state.mousePrevious.x;
    *y = state.mousePrevious.y;
}