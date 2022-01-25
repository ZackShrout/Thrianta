#include "TMath.h"
#include "Platform/Platform.h"

#include <math.h>
#include <stdlib.h>

static b8 randSeeded = false;

/**
 * Note that these are here in order to prevent having to import the
 * entire <math.h> everywhere.
 */
f32 tsin(f32 x)
{
    return sinf(x);
}

f32 tcos(f32 x)
{
    return cosf(x);
}

f32 ttan(f32 x)
{
    return tanf(x);
}

f32 tacos(f32 x)
{
    return acosf(x);
}

f32 tsqrt(f32 x)
{
    return sqrtf(x);
}

f32 tabs(f32 x)
{
    return fabsf(x);
}

s32 trandom()
{
    if (!randSeeded)
    {
        srand((u32)PlatformGetAbsoluteTime());
        randSeeded = true;
    }
    return rand();
}

s32 trandom_in_range(s32 min, s32 max)
{
    if (!randSeeded)
    {
        srand((u32)PlatformGetAbsoluteTime());
        randSeeded = true;
    }
    return (rand() % (max - min + 1)) + min;
}

f32 tkrandom()
{
    return (float)trandom() / (f32)RAND_MAX;
}

f32 tkrandom_in_range(f32 min, f32 max)
{
    return min + ((float)trandom() / ((f32)RAND_MAX / (max - min)));
}