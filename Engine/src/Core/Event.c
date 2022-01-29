#include "Core/Event.h"
#include "Core/TMemory.h"
#include "Containers/DArray.h"

typedef struct registered_event
{
    void* listener;
    PFN_on_event callback;
} registered_event;

typedef struct event_code_entry
{
    registered_event* events;
} event_code_entry;

// This should be more than enough codes...
#define MAX_MESSAGE_CODES 16384

// State structure.
typedef struct event_system_state
{
    // Lookup table for event codes.
    event_code_entry registered[MAX_MESSAGE_CODES];
} event_system_state;

/**
 * Event system internal statePtr->
 */
static event_system_state* statePtr;

void EventSystemInitialize(u64* memoryRequirements, void* state)
{
    *memoryRequirements = sizeof(event_system_state);
    if (state == 0) return;

    TZeroMemory(state, sizeof(state));
    statePtr = state;
}

void EventSystemShutdown(void* state)
{
    if (statePtr)
    {
        // Free the events arrays. And objects pointed to should be destroyed on their own.
        for (u16 i = 0; i < MAX_MESSAGE_CODES; i++)
        {
            if (statePtr->registered[i].events != 0)
            {
                DArrayDestroy(statePtr->registered[i].events);
                statePtr->registered[i].events = 0;
            }
        }
    }

    statePtr = 0;
}

b8 EventRegister(u16 code, void* listener, PFN_on_event onEvent)
{
    if (!statePtr) return false;

    if (statePtr->registered[code].events == 0)
    {
        statePtr->registered[code].events = DArrayCreate(registered_event);
    }

    u64 registeredCount = DArrayLength(statePtr->registered[code].events);
    for (u64 i = 0; i < registeredCount; i++)
    {
        if (statePtr->registered[code].events[i].listener == listener)
        {
            // TODO: warn
            return false;
        }
    }

    // If at this point, no duplicate was found. Proceed with registration.
    registered_event event;
    event.listener = listener;
    event.callback = onEvent;
    DArrayPush(statePtr->registered[code].events, event);

    return true;
}

b8 EventUnregister(u16 code, void* listener, PFN_on_event onEvent)
{
    if (!statePtr) return false;

    // On nothing is registered for the code, boot out.
    if (statePtr->registered[code].events == 0)
    {
        // TODO: warn
        return false;
    }

    u64 registeredCount = DArrayLength(statePtr->registered[code].events);
    for (u64 i = 0; i < registeredCount; i++)
    {
        registered_event e = statePtr->registered[code].events[i];
        if (e.listener == listener && e.callback == onEvent)
        {
            // Found one, remove it
            registered_event poppedEvent;
            DArrayPopAt(statePtr->registered[code].events, i, &poppedEvent);
            return true;
        }
    }

    // Not found.
    return false;
}

b8 EventFire(u16 code, void* sender, event_context context)
{
    if (!statePtr) return false;

    // If nothing is registered for the code, boot out.
    if (statePtr->registered[code].events == 0)
    {
        return false;
    }

    u64 registeredCount = DArrayLength(statePtr->registered[code].events);
    for (u64 i = 0; i < registeredCount; i++)
    {
        registered_event e = statePtr->registered[code].events[i];
        if (e.callback(code, sender, e.listener, context))
        {
            // Message has been handled, do not send to other listeners.
            return true;
        }
    }

    // Not found.
    return false;
}