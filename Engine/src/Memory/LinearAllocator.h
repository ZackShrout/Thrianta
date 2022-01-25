#pragma once
#include "Defines.h"

typedef struct linear_allocator
{
    u64     totalSize;
    u64     allocated;
    void*   memory;
    b8      ownsMemory;
} linear_allocator;

TAPI void LinearAllocatorCreate(u64 totalSize, void* memory, linear_allocator* outAllocator);
TAPI void LinearAllocatorDestroy(linear_allocator* allocator);
TAPI void* LinearAllocatorAllocate(linear_allocator* allocator, u64 size);
TAPI void LinearAllocatorFreeAll(linear_allocator* allocator);