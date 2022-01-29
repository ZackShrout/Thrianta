#include "Logger.h"
#include "Asserts.h"
#include "Platform/Platform.h"

// TODO: temporary
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

typedef struct logger_system_state
{
    b8 initialized;
} logger_system_state;

static logger_system_state* statePtr;

b8 LoggingSystemInitialize(u64* memoryRequirement, void* state)
{
    *memoryRequirement = sizeof(logger_system_state);
    if (state == 0) return true;
    
    statePtr = state;
    statePtr->initialized = true;

    // TODO: Remove this
    TFATAL("A test message: %f", 3.14f);
    TERROR("A test message: %f", 3.14f);
    TWARN("A test message: %f", 3.14f);
    TINFO("A test message: %f", 3.14f);
    TDEBUG("A test message: %f", 3.14f);
    TTRACE("A test message: %f", 3.14f); 
    
    // TODO: create log file.
    return true;
}

void ShutdownLogging(void* state)
{
    // TODO: cleanup logging/write queued entries.
    statePtr = 0;
}

void LogOutput(log_level level, const char* message, ...)
{
    const char* levelStrings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};
    b8 isError = level < LOG_LEVEL_WARN;

    // Technically imposes a 32k character limit on a single log entry, but...
    // DON'T DO THAT!
    const s32 msgLength = 32000;
    char outMessage[msgLength];
    memset(outMessage, 0, sizeof(outMessage));

    // Format original message.
    // NOTE: Oddly enough, MS's headers override the GCC/Clang va_list type with a "typedef char* va_list" in some
    // cases, and as a result throws a strange error here. The workaround for now is to just use __builtin_va_list,
    // which is the type GCC/Clang's va_start expects.
    __builtin_va_list argPtr;
    va_start(argPtr, message);
    vsnprintf(outMessage, msgLength, message, argPtr);
    va_end(argPtr);

    char outMessage2[msgLength];
    sprintf(outMessage2, "%s%s\n", levelStrings[level], outMessage);

    // Platform-specific output.
    if (isError)
    {
        PlatformConsoleWriteError(outMessage2, level);
    }
    else
    {
        PlatformConsoleWrite(outMessage2, level);
    }
}

void ReportAssertionFailure(const char* expression, const char* message, const char* file, s32 line)
{
    LogOutput(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: '%s', in file: %s, line: %d\n", expression, message, file, line);
}