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
 * Event system internal state.
 */
static b8 isInitialized = false;
static event_system_state state;

b8 EventInitialize()
{
    if (isInitialized == true)
    {
        return false;
    }
    isInitialized = false;
    TZeroMemory(&state, sizeof(state));

    isInitialized = true;

    return true;
}

void EventShutdown()
{
    // Free the events arrays. And objects pointed to should be destroyed on their own.
    for(u16 i = 0; i < MAX_MESSAGE_CODES; i++)
    {
        if(state.registered[i].events != 0)
        {
            DArrayDestroy(state.registered[i].events);
            state.registered[i].events = 0;
        }
    }
}

b8 EventRegister(u16 code, void* listener, PFN_on_event onEvent)
{
    if(isInitialized == false)
    {
        return false;
    }

    if(state.registered[code].events == 0)
    {
        state.registered[code].events = DArrayCreate(registered_event);
    }

    u64 registeredCount = DArrayLength(state.registered[code].events);
    for(u64 i = 0; i < registeredCount; i++)
    {
        if(state.registered[code].events[i].listener == listener)
        {
            // TODO: warn
            return false;
        }
    }

    // If at this point, no duplicate was found. Proceed with registration.
    registered_event event;
    event.listener = listener;
    event.callback = onEvent;
    DArrayPush(state.registered[code].events, event);

    return true;
}

b8 EventUnregister(u16 code, void* listener, PFN_on_event onEvent)
{
    if(isInitialized == false)
    {
        return false;
    }

    // On nothing is registered for the code, boot out.
    if(state.registered[code].events == 0)
    {
        // TODO: warn
        return false;
    }

    u64 registeredCount = DArrayLength(state.registered[code].events);
    for(u64 i = 0; i < registeredCount; i++)
    {
        registered_event e = state.registered[code].events[i];
        if(e.listener == listener && e.callback == onEvent)
        {
            // Found one, remove it
            registered_event poppedEvent;
            DArrayPopAt(state.registered[code].events, i, &poppedEvent);
            return true;
        }
    }

    // Not found.
    return false;
}

b8 EventFire(u16 code, void* sender, event_context context)
{
    if(isInitialized == false)
    {
        return false;
    }

    // If nothing is registered for the code, boot out.
    if(state.registered[code].events == 0)
    {
        return false;
    }

    u64 registeredCount = DArrayLength(state.registered[code].events);
    for(u64 i = 0; i < registeredCount; i++)
    {
        registered_event e = state.registered[code].events[i];
        if(e.callback(code, sender, e.listener, context))
        {
            // Message has been handled, do not send to other listeners.
            return true;
        }
    }

    // Not found.
    return false;
}