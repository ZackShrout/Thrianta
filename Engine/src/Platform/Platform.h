#pragma once

#include "Defines.h"

b8 PlatformSystemStartup(
    u64* memoryRequirements,
    void* state,
    const char* applicationName,
    s32 x,
    s32 y,
    s32 width,
    s32 height);

void PlatformSystemShutdown(void* state);

b8 PlatformPumpMessages();

void* PlatformAllocate(u64 size, b8 aligned);
void PlatformFree(void* block, b8 aligned);
void* PlatformZeroMemory(void* block, u64 size);
void* PlatformCopyMemory(void* dest, const void* source, u64 size);
void* PlatformSetMemory(void* dest, s32 value, u64 size);

void PlatformConsoleWrite(const char* message, u8 colour);
void PlatformConsoleWriteError(const char* message, u8 colour);

f64 PlatformGetAbsoluteTime();

// Sleep on the thread for the provided ms. This blocks the main thread.
// Should only be used for giving time back to the OS for unused update power.
// Therefore it is not exported.
void PlatformSleep(u64 ms);