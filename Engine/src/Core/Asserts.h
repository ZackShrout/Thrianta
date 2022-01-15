#pragma once

#include "Defines.h"

// Disable assertions by commenting out the below line.
#define TASSERTIONS_ENABLED

#ifdef TASSERTIONS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak()
#else
#define debugBreak() __builtin_trap()
#endif

TAPI void ReportAssertionFailure(const char* expression, const char* message, const char* file, s32 line);

#define TASSERT(expr)                                                \
    {                                                                \
        if (expr) {                                                  \
        } else {                                                     \
            ReportAssertionFailure(#expr, "", __FILE__, __LINE__); \
            debugBreak();                                            \
        }                                                            \
    }

#define TASSERT_MSG(expr, message)                                        \
    {                                                                     \
        if (expr) {                                                       \
        } else {                                                          \
            ReportAssertionFailure(#expr, message, __FILE__, __LINE__); \
            debugBreak();                                                 \
        }                                                                 \
    }

#ifdef _DEBUG
#define TASSERT_DEBUG(expr)                                          \
    {                                                                \
        if (expr) {                                                  \
        } else {                                                     \
            ReportAssertionFailure(#expr, "", __FILE__, __LINE__); \
            debugBreak();                                            \
        }                                                            \
    }
#else
#define TASSERT_DEBUG(expr)  // Does nothing at all
#endif

#else
#define TASSERT(expr)               // Does nothing at all
#define TASSERT_MSG(expr, message)  // Does nothing at all
#define TASSERT_DEBUG(expr)         // Does nothing at all
#endif