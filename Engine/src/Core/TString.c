#include "Core/TString.h"
#include "Core/TMemory.h"

#include <string.h>

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