#include "LinearAllocator.h"
#include "Core/TMemory.h"
#include "Core/Logger.h"

void LinearAllocatorCreate(u64 totalSize, void* memory, linear_allocator* outAllocator)
{
    if (outAllocator)
    {
        outAllocator->totalSize = totalSize;
        outAllocator->allocated = 0;
        outAllocator->ownsMemory = (memory == 0);
        if (memory)
        {
            outAllocator->memory = memory;
        }
        else
        {
            outAllocator->memory = TAllocate(totalSize, MEMORY_TAG_LINEAR_ALLOCATOR);
        }
    }
}

void LinearAllocatorDestroy(linear_allocator* allocator)
{
    if (allocator)
    {
        allocator->allocated = 0;
        if (allocator->ownsMemory && allocator->memory)
        {
            TFree(allocator->memory, allocator->totalSize, MEMORY_TAG_LINEAR_ALLOCATOR);
        }
        allocator->memory = 0;
        allocator->totalSize = 0;
        allocator->ownsMemory = false;
    }
}

void* LinearAllocatorAllocate(linear_allocator* allocator, u64 size)
{
    if (allocator && allocator->memory)
    {
        if (allocator->allocated + size > allocator->totalSize)
        {
            u64 remaining = allocator->totalSize - allocator->allocated;
            TERROR("LinearAllocatorAllocate - Tried to allocate %lluB, only %lluB remaining.", size, remaining);
            return 0;
        }

        void* block = ((u8*)allocator->memory) + allocator->allocated;
        allocator->allocated += size;
        return block;
    }

    TERROR("LinearAllocatorAllocate - Provided allocator not initialized.");
    return 0;
}

void LinearAllocatorFreeAll(linear_allocator* allocator)
{
    if (allocator && allocator->memory)
    {
        allocator->allocated = 0;
        TZeroMemory(allocator->memory, allocator->totalSize);
    }
}