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

// Internal input state pointer
static input_state* statePtr;

void InputSystemInitialize(u64* memoryRequirements, void* state)
{
    *memoryRequirements = sizeof(input_state);
    if (state == 0) return;
    
    TZeroMemory(state, sizeof(input_state));
    statePtr = state;
    TINFO("Input subsystem initialized.");
}

void InputSystemShutdown(void* state)
{
    // TODO: Add shutdown routines when needed.
    statePtr = 0;
}

void InputUpdate(f64 dt)
{
    if (!statePtr) return;

    // Copy current states to previous states.
    TCopyMemory(&statePtr->keyboardPrevious, &statePtr->keyboardCurrent, sizeof(keyboard_state));
    TCopyMemory(&statePtr->mousePrevious, &statePtr->mouseCurrent, sizeof(mouse_state));
}

void InputProcessKey(keys key, b8 pressed)
{
    // Only handle this if the state actually changed.
    if (statePtr && statePtr->keyboardCurrent.keys[key] != pressed)
    {
        // Update internal state.
        statePtr->keyboardCurrent.keys[key] = pressed;

        if (key == KEY_LALT)
        {
            TINFO("Left alt %s.", pressed ? "pressed" : "released");
        }
        else if (key == KEY_RALT)
        {
            TINFO("Right alt %s.", pressed ? "pressed" : "released");
        }
        
        if (key == KEY_LSHIFT)
        {
            TINFO("Left shift %s.", pressed ? "pressed" : "released");
        }
        else if (key == KEY_RSHIFT)
        {
            TINFO("Right shift %s.", pressed ? "pressed" : "released");
        }

        if (key == KEY_LCONTROL)
        {
            TINFO("Left control %s.", pressed ? "pressed" : "released");
        }
        else if (key == KEY_RCONTROL)
        {
            TINFO("Right control %s.", pressed ? "pressed" : "released");
        }

        // Fire off an event for immediate processing.
        event_context context;
        context.data.u16[0] = key;
        EventFire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, 0, context);
    }
}

void InputProcessButton(buttons button, b8 pressed)
{
    // If the state changed, fire an event.
    if (statePtr->mouseCurrent.buttons[button] != pressed)
    {
        statePtr->mouseCurrent.buttons[button] = pressed;

        // Fire the event.
        event_context context;
        context.data.u16[0] = button;
        EventFire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, 0, context);
    }
}

void InputProcessMouseMove(s16 x, s16 y)
{
    // Only process if actually different
    if (statePtr->mouseCurrent.x != x || statePtr->mouseCurrent.y != y)
    {
        // NOTE: Enable this if debugging.
        // TDEBUG("Mouse pos: %i, %i!", x, y);

        // Update internal state.
        statePtr->mouseCurrent.x = x;
        statePtr->mouseCurrent.y = y;

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
    if (!statePtr) return false;

    return statePtr->keyboardCurrent.keys[key] == true;
}

b8 InputIsKeyUp(keys key)
{
    if (!statePtr) return true;

    return statePtr->keyboardCurrent.keys[key] == false;
}

b8 InputWasKeyDown(keys key)
{
    if (!statePtr) return false;

    return statePtr->keyboardPrevious.keys[key] == true;
}

b8 InputWasKeyUp(keys key)
{
    if (!statePtr) return true;

    return statePtr->keyboardPrevious.keys[key] == false;
}

// MOUSE INPUT
b8 InputIsButtonDown(buttons button)
{
    if (!statePtr) return false;

    return statePtr->mouseCurrent.buttons[button] == true;
}

b8 InputIsButtonUp(buttons button)
{
    if (!statePtr) return true;

    return statePtr->mouseCurrent.buttons[button] == false;
}

b8 InputWasButtonDown(buttons button)
{
    if (!statePtr) return false;

    return statePtr->mousePrevious.buttons[button] == true;
}

b8 InputWasButtonUp(buttons button)
{
    if (!statePtr) return true;

    return statePtr->mousePrevious.buttons[button] == false;
}

void InputGetMousePosition(s32* x, s32* y)
{
    if (!statePtr)
    {
        *x = 0;
        *y = 0;
        return;
    }
    *x = statePtr->mouseCurrent.x;
    *y = statePtr->mouseCurrent.y;
}

void InputGetPreviousMousePosition(s32* x, s32* y)
{
    if (!statePtr)
    {
        *x = 0;
        *y = 0;
        return;
    }
    *x = statePtr->mousePrevious.x;
    *y = statePtr->mousePrevious.y;
}