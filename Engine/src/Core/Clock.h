#pragma once
#include "Defines.h"

typedef struct clock {
    f64 startTime;
    f64 elapsed;
} clock;

// Updates the provided clock. Should be called just before checking elapsed time.
// Has no effect on non-started clocks.
void ClockUpdate(clock* clock);

// Starts the provided clock. Resets elapsed time.
void ClockStart(clock* clock);

// Stops the provided clock. Does not reset elapsed time.
void ClockStop(clock* clock);