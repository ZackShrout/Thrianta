#include "TestManager.h"
#include <Containers/DArray.h>
#include <Core/Logger.h>
#include <Core/TString.h>
#include <Core/Clock.h>

typedef struct test_entry {
    PFN_test func;
    char* desc;
} test_entry;

static test_entry* tests;

void TestManagerInit()
{
    tests = DArrayCreate(test_entry);
}

void TestManagerRegisterTest(u8 (*PFN_test)(), char* desc)
{
    test_entry e;
    e.func = PFN_test;
    e.desc = desc;
    DArrayPush(tests, e);
}

void TestManagerRunTests()
{
    u32 passed = 0;
    u32 failed = 0;
    u32 skipped = 0;

    u32 count = DArrayLength(tests);

    clock totalTime;
    ClockStart(&totalTime);

    for (u32 i = 0; i < count; i++)
    {
        clock testTime;
        ClockStart(&testTime);
        u8 result = tests[i].func();
        ClockUpdate(&testTime);

        if (result == true)
        {
            ++passed;
        }
        else if (result == BYPASS)
        {
            TWARN("[SKIPPED]: %s", tests[i].desc);
            ++skipped;
        }
        else
        {
            TERROR("[FAILED]: %s", tests[i].desc);
            ++failed;
        }
        char status[20];
        StringFormat(status, failed ? "*** %d FAILED ***" : "SUCCESS", failed);
        ClockUpdate(&totalTime);
        TINFO("Executed %d of %d (skipped %d) %s (%.6f sec / %.6f sec total", i + 1, count, skipped, status, testTime.elapsed, totalTime.elapsed);
    }

    ClockStop(&totalTime);

    TINFO("Results: %d passed, %d failed, %d skipped.", passed, failed, skipped);
}