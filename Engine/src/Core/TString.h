#pragma once

#include "Defines.h"

// Returns the length of the given string.
TAPI u64 StringLength(const char* str);
TAPI char* StringDuplicate(const char* str);
// Case-sensitive string comparison. True if the same, otherwise false.
TAPI b8 StringsEqual(const char* str0, const char* str1);
// Performs string formatting to dest given format string and parameters.
TAPI s32 StringFormat(char* dest, const char* format, ...);
//
/**
 * Performs variadic string formatting to dest given format string and vaList.
 * @param dest The destination for the formatted string.
 * @param format The string to be formatted.
 * @param vaList The variadic argument list.
 * @returns The size of the data written.
 */
TAPI s32 StringFormatV(char* dest, const char* format, void* vaList);