#include "Logger.h"
#include "Asserts.h"

// TODO: temporary
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

b8 InitializeLogging()
{
    // TODO: create log file.
    return TRUE;
}

void ShutdownLogging()
{
    // TODO: cleanup logging/write queued entries.
}

void LogOutput(log_level level, const char* message, ...)
{
    const char* levelStrings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};
    //b8 isError = level < 2;

    // Technically imposes a 32k character limit on a single log entry, but...
    // DON'T DO THAT!
    char outMessage[32000];
    memset(outMessage, 0, sizeof(outMessage));

    // Format original message.
    // NOTE: Oddly enough, MS's headers override the GCC/Clang va_list type with a "typedef char* va_list" in some
    // cases, and as a result throws a strange error here. The workaround for now is to just use __builtin_va_list,
    // which is the type GCC/Clang's va_start expects.
    __builtin_va_list argPtr;
    va_start(argPtr, message);
    vsnprintf(outMessage, 32000, message, argPtr);
    va_end(argPtr);

    char outMessage2[32000];
    sprintf(outMessage2, "%s%s\n", levelStrings[level], outMessage);

    // TODO: platform-specific output.
    printf("%s", outMessage2);
}

void ReportAssertionFailure(const char* expression, const char* message, const char* file, s32 line)
{
    LogOutput(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: '%s', in file: %s, line: %d\n", expression, message, file, line);
}