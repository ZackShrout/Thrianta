#pragma once

#include "Defines.h"

// Returns the length of the given string.
TAPI u64 StringLength(const char* str);
TAPI char* StringDuplicate(const char* str);
// Case-sensitive string comparison. True if the same, otherwise false.
TAPI b8 StringsEqual(const char* str0, const char* str1);