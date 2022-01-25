#pragma once
#include <Defines.h>

#define BYPASS 2

typedef u8 (*PFN_test)();

void TestManagerInit();
void TestManagerRegisterTest(PFN_test, char* desc);
void TestManagerRunTests();