#include "Clock.h"
#include "Platform/Platform.h"

void ClockUpdate(clock* clock)
{
    if (clock->startTime != 0)
    {
        clock->elapsed = PlatformGetAbsoluteTime() - clock->startTime;
    }
}

void ClockStart(clock* clock)
{
    clock->startTime = PlatformGetAbsoluteTime();
    clock->elapsed = 0;
}

void ClockStop(clock* clock)
{
    clock->startTime = 0;
}