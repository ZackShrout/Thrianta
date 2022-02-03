#pragma once

#include "Math/MathTypes.h"

typedef struct texture
{
    u32 id;
    u32 width;
    u32 height;
    u8 channelCount;
    b8 hasTransparency;
    u32 generation;
    void* internalData;
} texture;