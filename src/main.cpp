#include <windows.h>
#include <vector>

struct MonitorInfo {
    HMONITOR monitor;
    RECT rect;
};

static std::vector<HWND> g_windows;
static HHOOK g_keyboardHook = nullptr;

void CloseAllWindows() {
    for (HWND window : g_windows) {
        if (IsWindow(window)) {
            DestroyWindow(window);
        }
    }

    PostQuitMessage(0);
}

LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        const KBDLLHOOKSTRUCT* keyboard = reinterpret_cast<const KBDLLHOOKSTRUCT*>(lParam);

        if ((wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) && keyboard->vkCode == VK_ESCAPE) {
            CloseAllWindows();

            return 1;
        }
    }

    return CallNextHookEx(g_keyboardHook, nCode, wParam, lParam);
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC, LPRECT, LPARAM lParam) {
    auto* monitors = reinterpret_cast<std::vector<MonitorInfo>*>(lParam);

    MONITORINFO info{};
    info.cbSize = sizeof(MONITORINFO);

    if (GetMonitorInfoW(hMonitor, &info)) {
        MonitorInfo monitor{};
        monitor.monitor = hMonitor;
        monitor.rect = info.rcMonitor;

        monitors->push_back(monitor);
    }

    return TRUE;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_ERASEBKGND: {
            return 1;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps{};
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rect{};
            GetClientRect(hwnd, &rect);

            HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
            FillRect(hdc, &rect, blackBrush);
            DeleteObject(blackBrush);
            EndPaint(hwnd, &ps);

            return 0;
        }
        case WM_DESTROY: {
            return 0;
        }
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int) {
    const wchar_t* className = L"PureBlackFullscreenWindow";

    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = nullptr;
    windowClass.lpszClassName = className;
    windowClass.style = CS_HREDRAW | CS_VREDRAW;

    if (!RegisterClassExW(&windowClass)) {
        MessageBoxW(nullptr, L"Failed to register window class.", L"Error", MB_ICONERROR);

        return 1;
    }

    std::vector<MonitorInfo> monitors;
    EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitors));

    if (monitors.empty()) {
        MessageBoxW(nullptr, L"No monitors were found.", L"Error", MB_ICONERROR);

        return 1;
    }

    for (const MonitorInfo& monitor : monitors) {
        const RECT& rect = monitor.rect;

        const int width = rect.right - rect.left;
        const int height = rect.bottom - rect.top;

        HWND hwnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, className, L"Good Night.", WS_POPUP, rect.left, rect.top, width, height, nullptr, nullptr, hInstance, nullptr);

        if (!hwnd) {
            continue;
        }

        g_windows.push_back(hwnd);
    }

    if (g_windows.empty()) {
        MessageBoxW(nullptr, L"Failed to create fullscreen windows.", L"Error", MB_ICONERROR);

        return 1;
    }

    for (HWND hwnd : g_windows) {
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
    }

    SetForegroundWindow(g_windows[0]);
    SetFocus(g_windows[0]);

    g_keyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, KeyboardHookProc, hInstance, 0);

    if (!g_keyboardHook) {
        MessageBoxW(nullptr, L"Failed to install keyboard hook.", L"Error", MB_ICONERROR);

        CloseAllWindows();

        return 1;
    }

    MSG message{};

    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    if (g_keyboardHook) {
        UnhookWindowsHookEx(g_keyboardHook);
        g_keyboardHook = nullptr;
    }

    for (HWND hwnd : g_windows) {
        if (IsWindow(hwnd)) {
            DestroyWindow(hwnd);
        }
    }

    g_windows.clear();

    return 0;
}