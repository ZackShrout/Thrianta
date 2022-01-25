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
static b8 initialized = false;
static input_state state = {};

void InputInitialize()
{
    TZeroMemory(&state, sizeof(input_state));
    initialized = true;
    TINFO("Input subsystem initialized.");
}

void InputShutdown()
{
    // TODO: Add shutdown routines when needed.
    initialized = false;
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
    if (key == KEY_LALT)
        {
            TINFO("Left alt pressed.");
        }
        else if (key == KEY_RALT)
        {
            TINFO("Right alt pressed.");
        }
        
        if (key == KEY_LSHIFT)
        {
            TINFO("Left shift pressed.");
        }
        else if (key == KEY_RSHIFT)
        {
            TINFO("Right shift pressed.");
        }

        if (key == KEY_LCONTROL)
        {
            TINFO("Left control pressed.");
        }
        else if (key == KEY_RCONTROL)
        {
            TINFO("Right control pressed.");
        }
    
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
        return false;
    }
    return state.keyboardCurrent.keys[key] == true;
}

b8 InputIsKeyUp(keys key)
{
    if (!initialized)
    {
        return true;
    }
    return state.keyboardCurrent.keys[key] == false;
}

b8 InputWasKeyDown(keys key)
{
    if (!initialized)
    {
        return false;
    }
    return state.keyboardPrevious.keys[key] == true;
}

b8 InputWasKeyUp(keys key)
{
    if (!initialized)
    {
        return true;
    }
    return state.keyboardPrevious.keys[key] == false;
}

// MOUSE INPUT
b8 InputIsButtonDown(buttons button)
{
    if (!initialized)
    {
        return false;
    }
    return state.mouseCurrent.buttons[button] == true;
}

b8 InputIsButtonUp(buttons button)
{
    if (!initialized)
    {
        return true;
    }
    return state.mouseCurrent.buttons[button] == false;
}

b8 InputWasButtonDown(buttons button)
{
    if (!initialized)
    {
        return false;
    }
    return state.mousePrevious.buttons[button] == true;
}

b8 InputWasButtonUp(buttons button)
{
    if (!initialized)
    {
        return true;
    }
    return state.mousePrevious.buttons[button] == false;
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