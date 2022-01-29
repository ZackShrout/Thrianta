#include "Platform/Platform.h"

// Windows platform layer.
#if TPLATFORM_WINDOWS

#include "Core/Logger.h"
#include "Core/Input.h"
#include "Core/Event.h"
#include "Containers/DArray.h"

#include <windows.h>
#include <windowsx.h>  // param input extraction
#include <stdlib.h>

// For surface creation
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include "Renderer/Vulkan/VulkanTypes.inl"

typedef struct platform_state
{
    HINSTANCE hInstance;
    HWND hwnd;
    VkSurfaceKHR surface;
} platform_state;

static platform_state* statePtr;

// Clock
static f64 clockFrequency;
static LARGE_INTEGER startTime;

LRESULT CALLBACK Win32ProcMessage(HWND hwnd, u32 msg, WPARAM wParam, LPARAM lParam);

void ClockSetup()
{
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clockFrequency = 1.0 / (f64)frequency.QuadPart;
    QueryPerformanceCounter(&startTime);
}

b8 PlatformSystemStartup(
    u64* memoryRequirements,
    void* state,
    const char* applicationName,
    s32 x,
    s32 y,
    s32 width,
    s32 height)
{
    *memoryRequirements = sizeof(platform_state);
    if (state == 0) return true;
    
    statePtr = state;
    statePtr->hInstance = GetModuleHandleA(0);

    // Setup and register window class.
    HICON icon = LoadIcon(statePtr->hInstance, IDI_APPLICATION);
    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_DBLCLKS;  // Get double-clicks
    wc.lpfnWndProc = Win32ProcMessage;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = statePtr->hInstance;
    wc.hIcon = icon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);  // NULL; // Manage the cursor manually
    wc.hbrBackground = NULL;                   // Transparent
    wc.lpszClassName = "ThriantaWindowClass";

    if (!RegisterClassA(&wc))
    {
        MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
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
        0, 0, statePtr->hInstance, 0);

    if (handle == 0)
    {
        MessageBoxA(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        TFATAL("Window creation failed!");
        return false;
    }
    else
    {
        statePtr->hwnd = handle;
    }

    // Show the window
    b32 shouldActivate = 1;  // TODO: if the window should not accept input, this should be false.
    s32 showWindowCommandFlags = shouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;
    // If initially minimized, use SW_MINIMIZE : SW_SHOWMINNOACTIVE;
    // If initially maximized, use SW_SHOWMAXIMIZED : SW_MAXIMIZE
    ShowWindow(statePtr->hwnd, showWindowCommandFlags);

    // Clock setup
    ClockSetup();

    return true;
}

void PlatformSystemShutdown(void* state)
{
    if (statePtr && statePtr->hwnd)
    {
        DestroyWindow(statePtr->hwnd);
        statePtr->hwnd = 0;
    }
}

b8 PlatformPumpMessages()
{
    if (statePtr)
    {
        MSG message;
        while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
    }

    return true;
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
    if (!clockFrequency)
        ClockSetup();

    LARGE_INTEGER nowTime;
    QueryPerformanceCounter(&nowTime);
    return (f64)nowTime.QuadPart * clockFrequency;
}

void PlatformSleep(u64 ms)
{
    Sleep(ms);
}

void PlatformGetRequiredExtensionNames(const char*** namesDArray)
{
    DArrayPush(*namesDArray, &"VK_KHR_win32_surface");
}

// Surface creation for Vulkan
b8 PlatformCreateVulkanSurface(vulkan_context* context)
{
    if (!statePtr) return false;

    VkWin32SurfaceCreateInfoKHR createInfo = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    createInfo.hinstance = statePtr->hInstance;
    createInfo.hwnd = statePtr->hwnd;

    VkResult result = vkCreateWin32SurfaceKHR(context->instance, &createInfo, context->allocator, &statePtr->surface);
    if (result != VK_SUCCESS)
    {
        TFATAL("Vulkan surface creation failed.");
        return false;
    }

    context->surface = statePtr->surface;
    return true;
}

LRESULT CALLBACK Win32ProcMessage(HWND hwnd, u32 msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_ERASEBKGND:
            // Notify the OS that erasing will be handled by the application to prevent flicker.
            return 1;
        case WM_CLOSE:
            event_context data = {};
            EventFire(EVENT_CODE_APPLICATION_QUIT, 0, data);
            return true;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
        {
            // Get the updated size.
            RECT r;
            GetClientRect(hwnd, &r);
            u32 width = r.right - r.left;
            u32 height = r.bottom - r.top;

            // Fire the event. The application layer should pick this up, but not handle it
            // as it shouldn be visible to other parts of the application.
            event_context context;
            context.data.u16[0] = (u16)width;
            context.data.u16[1] = (u16)height;
            EventFire(EVENT_CODE_RESIZED, 0, context);
        } break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            // Key pressed/released
            b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
            keys key = (u16)wParam;

            // Check for extended scan code
            b8 isExtended = (HIWORD(lParam) & KF_EXTENDED) == KF_EXTENDED;

            if (wParam == VK_MENU) // Alt key
            {
               key = isExtended ? KEY_RALT : KEY_LALT;
            }
            else if (wParam == VK_SHIFT) // Shift key
            {
                // KF_EXTENDED is not set for shift key
                u32 leftShift = MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC);
                u32 scancode = ((lParam & (0xFF << 16)) >> 16);
                key = scancode == leftShift ? KEY_LSHIFT : KEY_RSHIFT;
            }
            else if (wParam == VK_CONTROL) // Control key
            {
                key = isExtended ? KEY_RCONTROL : KEY_LCONTROL;
            }

            // Pass to the input subsystem for processing
            InputProcessKey(key, pressed);

            // Return 0 to prevent default window behaviour for some keypresses, such as alt.
            return 0;
        }
        case WM_MOUSEMOVE:
        {
            // Mouse move
            s32 xPosition = GET_X_LPARAM(lParam);
            s32 yPosition = GET_Y_LPARAM(lParam);

            // Pass to the input subsystem for processing
            InputProcessMouseMove(xPosition, yPosition);
        } break;
        case WM_MOUSEWHEEL:
        {
            s32 zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            if (zDelta != 0)
            {
               // Flatten the input to an OS-independent (-1, 1)
               zDelta = (zDelta < 0) ? -1 : 1;

               // Pass to the input subsystem for processing
               InputProcessMouseWheel(zDelta);
            }
        } break;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        {
            b8 pressed = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
            buttons mouseButton = BUTTON_MAX_BUTTONS;
            switch (msg)
            {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                    mouseButton = BUTTON_LEFT;
                    break;
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                    mouseButton = BUTTON_MIDDLE;
                    break;
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                    mouseButton = BUTTON_RIGHT;
                    break;
            }

            // Pass to the input subsystem for processing
            if (mouseButton != BUTTON_MAX_BUTTONS)
            {
                InputProcessButton(mouseButton, pressed);
            }
        } break;
    }

    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

#endif // TPLATFORM_WINDOWS