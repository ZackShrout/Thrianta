#include "LinearAllocatorTests.h"
#include "../TestManager.h"
#include "../Expect.h"
#include <Memory/LinearAllocator.h>
#include <Defines.h>

u8 LinearAllocatorShouldCreateAndDestroy()
{
    linear_allocator alloc;
    LinearAllocatorCreate(sizeof(u64), 0, &alloc);

    ExpectShouldNotBe(0, alloc.memory);
    ExpectShouldBe(sizeof(u64), alloc.totalSize);
    ExpectShouldBe(0, alloc.allocated);

    LinearAllocatorDestroy(&alloc);

    ExpectShouldBe(0, alloc.memory);
    ExpectShouldBe(0, alloc.totalSize);
    ExpectShouldBe(0, alloc.allocated);

    return true;
}

u8 LinearAllocatorSingleAllocationAllSpace()
{
    linear_allocator alloc;
    LinearAllocatorCreate(sizeof(u64), 0, &alloc);

    // Single allocation.
    void* block = LinearAllocatorAllocate(&alloc, sizeof(u64));

    // Validate it
    ExpectShouldNotBe(0, block);
    ExpectShouldBe(sizeof(u64), alloc.allocated);

    LinearAllocatorDestroy(&alloc);

    return true;
}

u8 LinearAllocatorMultiAllocationAllSpace()
{
    u64 maxAllocs = 1024;
    linear_allocator alloc;
    LinearAllocatorCreate(sizeof(u64) * maxAllocs, 0, &alloc);

    // Multiple allocations - full.
    void* block;
    for (u64 i = 0; i < maxAllocs; i++)
    {
        block = LinearAllocatorAllocate(&alloc, sizeof(u64));
        // Validate it
        ExpectShouldNotBe(0, block);
        ExpectShouldBe(sizeof(u64) * (i + 1), alloc.allocated);
    }

    TDEBUG("Total allocated is: %lluB", alloc.allocated);

    LinearAllocatorDestroy(&alloc);

    return true;
}

u8 LinearAllocatorMultiAllocationOverAllocate()
{
    u64 maxAllocs = 3;
    linear_allocator alloc;
    LinearAllocatorCreate(sizeof(u64) * maxAllocs, 0, &alloc);

    // Multiple allocations - full.
    void* block;
    for (u64 i = 0; i < maxAllocs; i++)
    {
        block = LinearAllocatorAllocate(&alloc, sizeof(u64));
        // Validate it
        ExpectShouldNotBe(0, block);
        ExpectShouldBe(sizeof(u64) * (i + 1), alloc.allocated);
    }

    TDEBUG("Note: The following error is intentionally caused by this test.");

    // Ask for one more allocation. Should error and return 0.
    block = LinearAllocatorAllocate(&alloc, sizeof(u64));
    // Validate it - allocated should be unchanged.
    ExpectShouldBe(0, block);
    ExpectShouldBe(sizeof(u64) * (maxAllocs), alloc.allocated);

    LinearAllocatorDestroy(&alloc);

    return true;
}

u8 LinearAllocatorMultiAllocationAllSpaceThenFree()
{
    u64 maxAllocs = 1024;
    linear_allocator alloc;
    LinearAllocatorCreate(sizeof(u64) * maxAllocs, 0, &alloc);

    // Multiple allocations - full.
    void* block;
    for (u64 i = 0; i < maxAllocs; i++)
    {
        block = LinearAllocatorAllocate(&alloc, sizeof(u64));
        // Validate it
        ExpectShouldNotBe(0, block);
        ExpectShouldBe(sizeof(u64) * (i + 1), alloc.allocated);
    }

    // Validate that pointer is reset.
    LinearAllocatorFreeAll(&alloc);
    ExpectShouldBe(0, alloc.allocated);

    LinearAllocatorDestroy(&alloc);

    return true;
}

void LinearAllocatorRegisterTests()
{
    TestManagerRegisterTest(LinearAllocatorShouldCreateAndDestroy, "Linear allocator should create and destroy");
    TestManagerRegisterTest(LinearAllocatorSingleAllocationAllSpace, "Linear allocator single alloc for all space");
    TestManagerRegisterTest(LinearAllocatorMultiAllocationAllSpace, "Linear allocator multi alloc for all space");
    TestManagerRegisterTest(LinearAllocatorMultiAllocationOverAllocate, "Linear allocator try over allocate");
    TestManagerRegisterTest(LinearAllocatorMultiAllocationAllSpaceThenFree, "Linear allocator allocated should be 0 after FreeAll");
}