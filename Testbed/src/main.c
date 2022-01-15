#include <Core/Logger.h>
#include <Core/Asserts.h>

int main(void)
{
    TFATAL("A test message: %f", 3.14f);
    TERROR("A test message: %f", 3.14f);
    TWARN("A test message: %f", 3.14f);
    TINFO("A test message: %f", 3.14f);
    TDEBUG("A test message: %f", 3.14f);
    TTRACE("A test message: %f", 3.14f);

    TASSERT(1 == 0);    

    return 0;
}