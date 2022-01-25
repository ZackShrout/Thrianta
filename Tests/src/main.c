#include "TestManager.h"
#include "Memory/LinearAllocatorTests.h"
#include <Core/Logger.h>

int main()
{
    // Always initialize the test manager first
    TestManagerInit();

    // Test registrations here
    LinearAllocatorRegisterTests();

    TDEBUG("Starting tests...");

    // Execute tests
    TestManagerRunTests();

    return 0;
}