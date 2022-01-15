#include "Platform/Platform.h"

// Windows platform layer.
#if TPLATFORM_WINDOWS

#include "Core/Logger.h"

#include <windows.h>
#include <windowsx.h>  // param input extraction
#include <stdlib.h>

typedef struct internal_state
{
    HINSTANCE hInstance;
    HWND hwnd;
} internal_state;

// Clock
static f64 clockFrequency;
static LARGE_INTEGER startTime;

LRESULT CALLBACK Win32ProcMessage(HWND hwnd, u32 msg, WPARAM wParam, LPARAM lParam);

b8 PlatformStartup(
    platform_state* platState,
    const char* applicationName,
    s32 x,
    s32 y,
    s32 width,
    s32 height)
{
    platState->internalState = malloc(sizeof(internal_state));
    internal_state* state = (internal_state*)platState->internalState;

    state->hInstance = GetModuleHandleA(0);

    // Setup and register window class.
    HICON icon = LoadIcon(state->hInstance, IDI_APPLICATION);
    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_DBLCLKS;  // Get double-clicks
    wc.lpfnWndProc = Win32ProcMessage;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = state->hInstance;
    wc.hIcon = icon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);  // NULL; // Manage the cursor manually
    wc.hbrBackground = NULL;                   // Transparent
    wc.lpszClassName = "ThriantaWindowClass";

    if (!RegisterClassA(&wc))
    {
        MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }

    // Create window
    u32 clientX = x;
    u32 clientY = y;
    u32 clientWidth = width;
    u32 clientHeight = height;

    u32 windowX = clientX;
    u32 windowY = clientY;
    u32 windowWidth = clientWidth;
    u32 windowHeight = clientHeight;

    u32 windowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
    u32 windowExStyle = WS_EX_APPWINDOW;

    windowStyle |= WS_MAXIMIZEBOX;
    windowStyle |= WS_MINIMIZEBOX;
    windowStyle |= WS_THICKFRAME;

    // Obtain the size of the border.
    RECT borderRect = {0, 0, 0, 0};
    AdjustWindowRectEx(&borderRect, windowStyle, 0, windowExStyle);

    // In this case, the border rectangle is negative.
    windowX += borderRect.left;
    windowY += borderRect.top;

    // Grow by the size of the OS border.
    windowWidth += borderRect.right - borderRect.left;
    windowHeight += borderRect.bottom - borderRect.top;

    HWND handle = CreateWindowExA(
        windowExStyle, "ThriantaWindowClass", applicationName,
        windowStyle, windowX, windowY, windowWidth, windowHeight,
        0, 0, state->hInstance, 0);

    if (handle == 0)
    {
        MessageBoxA(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        TFATAL("Window creation failed!");
        return FALSE;
    }
    else
    {
        state->hwnd = handle;
    }

    // Show the window
    b32 shouldActivate = 1;  // TODO: if the window should not accept input, this should be false.
    s32 showWindowCommandFlags = shouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;
    // If initially minimized, use SW_MINIMIZE : SW_SHOWMINNOACTIVE;
    // If initially maximized, use SW_SHOWMAXIMIZED : SW_MAXIMIZE
    ShowWindow(state->hwnd, showWindowCommandFlags);

    // Clock setup
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clockFrequency = 1.0 / (f64)frequency.QuadPart;
    QueryPerformanceCounter(&startTime);

    return TRUE;
}

void PlatformShutdown(platform_state* platState)
{
    // Simply cold-cast to the known type.
    internal_state* state = (internal_state*)platState->internalState;

    if (state->hwnd)
    {
        DestroyWindow(state->hwnd);
        state->hwnd = 0;
    }
}

b8 PlatformPumpMessages(platform_state* platState)
{
    MSG message;
    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return TRUE;
}

void* PlatformAllocate(u64 size, b8 aligned)
{
    return malloc(size);
}

void PlatformFree(void* block, b8 aligned)
{
    free(block);
}

void* PlatformZeroMemory(void* block, u64 size)
{
    return memset(block, 0, size);
}

void* PlatformCopyMemory(void* dest, const void* source, u64 size)
{
    return memcpy(dest, source, size);
}

void* PlatformSetMemory(void* dest, s32 value, u64 size)
{
    return memset(dest, value, size);
}

void PlatformConsoleWrite(const char* message, u8 colour)
{
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(consoleHandle, levels[colour]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, number_written, 0);
}

void PlatformConsoleWriteError(const char* message, u8 colour)
{
    HANDLE consoleHandle = GetStdHandle(STD_ERROR_HANDLE);
    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(consoleHandle, levels[colour]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, number_written, 0);
}

f64 PlatformGetAbsoluteTime()
{
    LARGE_INTEGER nowTime;
    QueryPerformanceCounter(&nowTime);
    return (f64)nowTime.QuadPart * clockFrequency;
}

void PlatformSleep(u64 ms)
{
    Sleep(ms);
}

LRESULT CALLBACK Win32ProcMessage(HWND hwnd, u32 msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_ERASEBKGND:
            // Notify the OS that erasing will be handled by the application to prevent flicker.
            return 1;
        case WM_CLOSE:
            // TODO: Fire an event for the application to quit.
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
        {
            // Get the updated size.
            // RECT r;
            // GetClientRect(hwnd, &r);
            // u32 width = r.right - r.left;
            // u32 height = r.bottom - r.top;

            // TODO: Fire an event for window resize.
        } break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            // Key pressed/released
            // b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
            // TODO: input processing

        } break;
        case WM_MOUSEMOVE:
        {
            // Mouse move
            // s32 x_position = GET_X_LPARAM(lParam);
            // s32 y_position = GET_Y_LPARAM(lParam);
            // TODO: input processing.
        } break;
        case WM_MOUSEWHEEL:
        {
            // s32 z_delta = GET_WHEEL_DELTA_WPARAM(wParam);
            // if (z_delta != 0)
            // {
            //    // Flatten the input to an OS-independent (-1, 1)
            //    z_delta = (z_delta < 0) ? -1 : 1;
            //    // TODO: input processing.
            // }
        } break;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        {
            // b8 pressed = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
            // TODO: input processing.
        } break;
    }

    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

#endif // TPLATFORM_WINDOWS