#include "Core/TString.h"
#include "Core/TMemory.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

u64 StringLength(const char* str)
{
    return strlen(str);
}

char* StringDuplicate(const char* str)
{
    u64 length = StringLength(str);
    char* copy = TAllocate(length + 1, MEMORY_TAG_STRING);
    TCopyMemory(copy, str, length + 1);
    return copy;
}

// Case-sensitive string comparison. True if the same, otherwise false.
b8 StringsEqual(const char* str0, const char* str1)
{
    return strcmp(str0, str1) == 0;
}

s32 StringFormat(char* dest, const char* format, ...)
{
    if (dest)
    {
        __builtin_va_list argPtr;
        va_start(argPtr, format);
        s32 written = StringFormatV(dest, format, argPtr);
        va_end(argPtr);
        return written;
    }
    return -1;
}

s32 StringFormatV(char* dest, const char* format, void* vaListp)
{
    if (dest)
    {
        // Big, but can fit on the stack.
        char buffer[32000];
        s32 written = vsnprintf(buffer, 32000, format, vaListp);
        buffer[written] = 0;
        TCopyMemory(dest, buffer, written + 1);

        return written;
    }
    return -1;
}