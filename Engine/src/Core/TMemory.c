#include "TMemory.h"
#include "Core/TString.h"
#include "Core/Logger.h"
#include "Platform/Platform.h"

// TODO: Custom string lib
#include <string.h>
#include <stdio.h>

struct memory_stats
{
    u64 totalAllocated;
    u64 taggedAllocations[MEMORY_TAG_MAX_TAGS];
};

static const char* memoryTagStrings[MEMORY_TAG_MAX_TAGS] =
{
    "UNKNOWN    ",
    "ARRAY      ",
    "LINEAR_ALLC",
    "DARRAY     ",
    "DICT       ",
    "RING_QUEUE ",
    "BST        ",
    "STRING     ",
    "APPLICATION",
    "JOB        ",
    "TEXTURE    ",
    "MAT_INST   ",
    "RENDERER   ",
    "GAME       ",
    "TRANSFORM  ",
    "ENTITY     ",
    "ENTITY_NODE",
    "SCENE      "
};

typedef struct memory_system_state
{
    struct memory_stats stats;
    u64 allocCount;
} memory_system_state;

static memory_system_state* statePtr;

void InitializeMemory(u64* memoryRequirements, void* state)
{
    *memoryRequirements = sizeof(memory_system_state);
    if (state == 0) return;

    statePtr = state;
    statePtr->allocCount = 0;
    PlatformZeroMemory(&statePtr->stats, sizeof(statePtr->stats));
}

void ShutdownMemory(void* state)
{
    statePtr = 0;
}

void* TAllocate(u64 size, memory_tag tag)
{
    if (tag == MEMORY_TAG_UNKNOWN)
    {
        TWARN("TAllocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    if (statePtr)
    {
        statePtr->stats.totalAllocated += size;
        statePtr->stats.taggedAllocations[tag] += size;
        statePtr->allocCount++;
    }

    // TODO: Memory alignment
    void* block = PlatformAllocate(size, false);
    PlatformZeroMemory(block, size);
    return block;
}

void TFree(void* block, u64 size, memory_tag tag)
{
    if (tag == MEMORY_TAG_UNKNOWN)
    {
        TWARN("TFree called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    statePtr->stats.totalAllocated -= size;
    statePtr->stats.taggedAllocations[tag] -= size;

    // TODO: Memory alignment
    PlatformFree(block, false);
}

void* TZeroMemory(void* block, u64 size)
{
    return PlatformZeroMemory(block, size);
}

void* TCopyMemory(void* dest, const void* source, u64 size)
{
    return PlatformCopyMemory(dest, source, size);
}

void* TSetMemory(void* dest, s32 value, u64 size)
{
    return PlatformSetMemory(dest, value, size);
}

char* GetMemoryUsageStr()
{
    const u64 gib = 1024 * 1024 * 1024;
    const u64 mib = 1024 * 1024;
    const u64 kib = 1024;

    char buffer[8000] = "System memory use (tagged):\n";
    u64 offset = strlen(buffer);
    for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; i++)
    {
        char unit[4] = "XiB";
        float amount = 1.0f;
        if (statePtr->stats.taggedAllocations[i] >= gib)
        {
            unit[0] = 'G';
            amount = statePtr->stats.taggedAllocations[i] / (float)gib;
        }
        else if (statePtr->stats.taggedAllocations[i] >= mib)
        {
            unit[0] = 'M';
            amount = statePtr->stats.taggedAllocations[i] / (float)mib;
        }
        else if (statePtr->stats.taggedAllocations[i] >= kib)
        {
            unit[0] = 'K';
            amount = statePtr->stats.taggedAllocations[i] / (float)kib;
        }
        else
        {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (float)statePtr->stats.taggedAllocations[i];
        }

        s32 length = snprintf(buffer + offset, 8000, "  %s: %.2f%s\n", memoryTagStrings[i], amount, unit);
        offset += length;
    }
    // TODO: This is a memory leak danger
    char* outString = StringDuplicate(buffer);
    return outString;
}

u64 GetMemoryAllocCount()
{
    if (statePtr)
    {
        return statePtr->allocCount;
    }

    return 0;
}