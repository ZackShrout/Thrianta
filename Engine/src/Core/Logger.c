#include "Logger.h"
#include "Asserts.h"
#include "Platform/Platform.h"
#include "Platform/Filesystem.h"
#include "TString.h"
#include "TMemory.h"

// TODO: temporary
#include <stdarg.h>

typedef struct logger_system_state
{
    file_handle logFileHandle;
} logger_system_state;

static logger_system_state* statePtr;

void AppendToLogFile(const char* message)
{
    if (statePtr && statePtr->logFileHandle.isValid)
    {
        // Since the message already contains a '\n', just write the bytes directly.
        u64 length = StringLength(message);
        u64 written = 0;
        if (!FilesystemWrite(&statePtr->logFileHandle, length, message, &written))
        {
            PlatformConsoleWriteError("ERROR writing to console.log.", LOG_LEVEL_ERROR);
        }
    }
}

b8 LoggingSystemInitialize(u64* memoryRequirement, void* state)
{
    *memoryRequirement = sizeof(logger_system_state);
    if (state == 0) return true;
    
    statePtr = state;

    // Create new/wipe existing log file, then open it.
    if (!FilesystemOpen("console.log", FILE_MODE_WRITE, false, &statePtr->logFileHandle))
    {
        PlatformConsoleWriteError("ERROR: Unable to open console.log for writing.", LOG_LEVEL_ERROR);
        return false;
    }

    // TODO: Remove this
    TFATAL("A test message: %f", 3.14f);
    TERROR("A test message: %f", 3.14f);
    TWARN("A test message: %f", 3.14f);
    TINFO("A test message: %f", 3.14f);
    TDEBUG("A test message: %f", 3.14f);
    TTRACE("A test message: %f", 3.14f); 
    
    return true;
}

void ShutdownLogging(void* state)
{
    // TODO: cleanup logging/write queued entries.
    statePtr = 0;
}

void LogOutput(log_level level, const char* message, ...)
{
    // TODO: These string operations are all pretty slow. This needs to be
    // moved to another thread eventually, along with the file writes, to
    // avoid slowing things down while the engine is trying to run.
    const char* levelStrings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};
    b8 isError = level < LOG_LEVEL_WARN;

    // Technically imposes a 32k character limit on a single log entry, but...
    // DON'T DO THAT!
    char outMessage[32000];
    TZeroMemory(outMessage, sizeof(outMessage));

    // Format original message.
    // NOTE: Oddly enough, MS's headers override the GCC/Clang va_list type with a "typedef char* va_list" in some
    // cases, and as a result throws a strange error here. The workaround for now is to just use __builtin_va_list,
    // which is the type GCC/Clang's va_start expects.
    __builtin_va_list argPtr;
    va_start(argPtr, message);
    StringFormatV(outMessage, message, argPtr);
    va_end(argPtr);

    // Prepend log level to message.
    StringFormat(outMessage, "%s%s\n", levelStrings[level], outMessage);

    // Print message to console
    if (isError)
        PlatformConsoleWriteError(outMessage, level);
    else
        PlatformConsoleWrite(outMessage, level);
    
    // Queue a copy to be written to the log file.
    AppendToLogFile(outMessage);
}

void ReportAssertionFailure(const char* expression, const char* message, const char* file, s32 line)
{
    LogOutput(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: '%s', in file: %s, line: %d\n", expression, message, file, line);
}