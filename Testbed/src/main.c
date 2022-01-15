#include <Core/Logger.h>
#include <Core/Asserts.h>

// TODO: Test
#include <Platform/Platform.h>

int main(void)
{
    TFATAL("A test message: %f", 3.14f);
    TERROR("A test message: %f", 3.14f);
    TWARN("A test message: %f", 3.14f);
    TINFO("A test message: %f", 3.14f);
    TDEBUG("A test message: %f", 3.14f);
    TTRACE("A test message: %f", 3.14f);

    platform_state state;

    if (PlatformStartup(&state, "Thrianta Test Window", 100, 100, 1280, 720))       
    {
        while (TRUE)
        {
            PlatformPumpMessages(&state);
        }
    }

    PlatformShutdown(&state);

    return 0;
}