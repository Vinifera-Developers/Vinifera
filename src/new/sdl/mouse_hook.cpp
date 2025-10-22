// MouseScaler.cpp
#include "mouse_hook.h"
#include <iostream>

namespace
{
// Pointer to the instance so the static hook proc can find it.
// We only allow one instance to run the hook at a time in this simple implementation.
static MouseScaler* g_instance = nullptr;

// Map WM_* mouse messages to SendInput flags for button events
inline WORD MessageToMouseEventFlag(UINT msg)
{
    switch (msg) {
    case WM_LBUTTONDOWN:
        return MOUSEEVENTF_LEFTDOWN;
    case WM_LBUTTONUP:
        return MOUSEEVENTF_LEFTUP;
    case WM_RBUTTONDOWN:
        return MOUSEEVENTF_RIGHTDOWN;
    case WM_RBUTTONUP:
        return MOUSEEVENTF_RIGHTUP;
    case WM_MBUTTONDOWN:
        return MOUSEEVENTF_MIDDLEDOWN;
    case WM_MBUTTONUP:
        return MOUSEEVENTF_MIDDLEUP;
    // mouse move and wheel handled separately
    default:
        return 0;
    }
}

// Low-level mouse hook procedure (static C-style)
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode != HC_ACTION) return CallNextHookEx(nullptr, nCode, wParam, lParam);

    if (!g_instance) return CallNextHookEx(nullptr, nCode, wParam, lParam);

    auto& inst = *g_instance;
    const MSLLHOOKSTRUCT* msh = reinterpret_cast<const MSLLHOOKSTRUCT*>(lParam);

    // If event was injected by us (or another injector), let it pass through.
    // MSLLHOOKSTRUCT::flags contains injected bits:
    // LLMHF_INJECTED = 0x00000001, LLMHF_LOWER_IL_INJECTED = 0x00000002
    if (msh->flags & MouseScaler::LLMHF_INJECTED_MASK) {
        // don't re-inject
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    // Build point
    POINT pt = msh->pt;

    if (!inst.IsPointOverTargetWindow(pt)) {
        // Not over target window: do nothing special.
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }

    // Over our window: swallow original event and synthesize a scaled event instead.
    // We return a non-zero value to prevent further processing of original event.
    // But to preserve expected behaviour we synthesize a corresponding event at scaled coordinates.

    // Copy to avoid accidental mutation.
    MSLLHOOKSTRUCT copy = *msh;
    inst.SynthesizeMouseEvent(static_cast<UINT>(wParam), copy);

    // Swallow original
    return 1;
}
} // namespace

// ------- MouseScaler implementation -------

MouseScaler::MouseScaler() = default;

MouseScaler::~MouseScaler()
{
    Stop();
}

bool MouseScaler::Start()
{
    bool expected = false;
    if (!m_running.compare_exchange_strong(expected, true)) {
        // already running
        return true;
    }

    m_exitRequested.store(false);

    // Only one instance supported by this implementation
    if (g_instance != nullptr) {
        // already have a global instance
        m_running.store(false);
        return false;
    }

    g_instance = this;

    m_thread = std::thread(&MouseScaler::ThreadMain, this);

    // wait a short while for the thread to start and install hook? optional
    return true;
}

void MouseScaler::Stop()
{
    if (!m_running.load()) return;

    m_exitRequested.store(true);

    // Post a quit message to the thread loop so it uninstalls the hook and exits
    if (m_threadId != 0) {
        PostThreadMessage(m_threadId, WM_QUIT, 0, 0);
    }

    if (m_thread.joinable()) m_thread.join();

    m_running.store(false);

    if (g_instance == this) g_instance = nullptr;
}

void MouseScaler::ThreadMain()
{
    // Create a message queue for this thread.
    // A thread-specific message queue exists after the first call to a user32 function that needs it.
    // Call PeekMessage to ensure the queue is created.
    MSG msg;
    PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE);

    m_threadId = GetCurrentThreadId();

    if (!InstallHook()) {
        // hook failed: exit thread
        m_running.store(false);
        return;
    }

    // Message loop - we rely on this to keep the hook alive on this thread.
    while (!m_exitRequested.load()) {
        // Use GetMessage so that WM_QUIT wakes us up.
        BOOL ret = GetMessage(&msg, nullptr, 0, 0);
        if (ret == 0 || ret == -1) // WM_QUIT or error
            break;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UninstallHook();
}

bool MouseScaler::InstallHook()
{
    // Install low-level mouse hook on this thread.
    // WH_MOUSE_LL can be installed from any thread and is global, but we attach the proc here.
    m_hook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, nullptr, 0);
    if (!m_hook) {
        DWORD err = GetLastError();
        // Optionally: log the error.
        return false;
    }
    return true;
}

void MouseScaler::UninstallHook()
{
    if (m_hook) {
        UnhookWindowsHookEx(m_hook);
        m_hook = nullptr;
    }
}

bool MouseScaler::IsPointOverTargetWindow(POINT ptScreen)
{
    if (!m_targetWindow) return false;

    // Get the window rectangle in screen coordinates
    RECT wr;
    if (!GetWindowRect(m_targetWindow, &wr)) return false;

    // If you prefer client area only, convert point to client and test against client rect:
    // POINT clientPt = ptScreen; ScreenToClient(m_targetWindow, &clientPt);
    // GetClientRect and test.

    return (ptScreen.x >= wr.left && ptScreen.x <= wr.right && ptScreen.y >= wr.top && ptScreen.y <= wr.bottom);
}

POINT MouseScaler::ScalePointForWindow(POINT ptScreen)
{
    POINT result = ptScreen;
    if (!m_targetWindow) return result;

    // Convert screen point to client coordinates
    POINT clientPt = ptScreen;
    ScreenToClient(m_targetWindow, &clientPt);

    // Get client rect size
    RECT clientRect;
    GetClientRect(m_targetWindow, &clientRect);
    int w = clientRect.right - clientRect.left;
    int h = clientRect.bottom - clientRect.top;
    if (w <= 0 || h <= 0) return result;

    double factor = m_scaleFactor.load();

    // Compute scaled client coordinates relative to client origin (0,0)
    double scaledClientX = clientPt.x * factor;
    double scaledClientY = clientPt.y * factor;

    // Map back to screen coordinates (client origin to screen)
    POINT clientOrigin = {clientRect.left, clientRect.top};
    ClientToScreen(m_targetWindow, &clientOrigin);

    result.x = static_cast<LONG>(clientOrigin.x + scaledClientX);
    result.y = static_cast<LONG>(clientOrigin.y + scaledClientY);

    return result;
}

void MouseScaler::ScreenPointToAbsolute(long x, long y, DWORD& outX, DWORD& outY)
{
    // Convert screen coordinates to normalized absolute coordinates required by SendInput:
    // 0..65535 corresponds to 0..screenWidth and 0..screenHeight
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    // Prevent divide-by-zero
    if (screenW <= 0) screenW = 1;
    if (screenH <= 0) screenH = 1;

    // Per MSDN, value of 65535 maps to the far right/bottom.
    outX = static_cast<DWORD>((x * 65535) / (screenW - 1));
    outY = static_cast<DWORD>((y * 65535) / (screenH - 1));
}

void MouseScaler::SynthesizeMouseEvent(UINT mouseMsg, const MSLLHOOKSTRUCT& orig)
{
    // Compute scaled screen point
    POINT scaled = ScalePointForWindow(orig.pt);

    // If this is a move message, issue a mouse move to scaled location
    // For button/wheel, we synthesize both a move (to the point) and the corresponding button/wheel event.

    // Build an array of INPUTs
    INPUT inputs[3];
    int inputCount = 0;

    // First: move to scaled absolute position
    DWORD ax = 0, ay = 0;
    ScreenPointToAbsolute(scaled.x, scaled.y, ax, ay);

    ZeroMemory(inputs, sizeof(inputs));

    inputs[inputCount].type = INPUT_MOUSE;
    inputs[inputCount].mi.dx = static_cast<LONG>(ax);
    inputs[inputCount].mi.dy = static_cast<LONG>(ay);
    inputs[inputCount].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    inputs[inputCount].mi.time = 0;
    inputs[inputCount].mi.mouseData = 0;
    ++inputCount;

    // Handle wheel
    if (mouseMsg == WM_MOUSEWHEEL || mouseMsg == WM_MOUSEHWHEEL) {
        INPUT wheelInput = {};
        wheelInput.type = INPUT_MOUSE;
        wheelInput.mi.dx = 0;
        wheelInput.mi.dy = 0;
        wheelInput.mi.time = 0;

        // use same wheel delta as original
        short zDelta = HIWORD(orig.mouseData); // mouseData contains wheel delta in high word

        if (mouseMsg == WM_MOUSEWHEEL) {
            wheelInput.mi.dwFlags = MOUSEEVENTF_WHEEL;
            wheelInput.mi.mouseData = static_cast<DWORD>(zDelta);
        } else // horizontal
        {
            wheelInput.mi.dwFlags = MOUSEEVENTF_HWHEEL;
            wheelInput.mi.mouseData = static_cast<DWORD>(zDelta);
        }

        inputs[inputCount++] = wheelInput;
    } else {
        // Button events: translate WM_* to MOUSEEVENTF_*
        WORD flag = MessageToMouseEventFlag(mouseMsg);
        if (flag != 0) {
            INPUT btn = {};
            btn.type = INPUT_MOUSE;
            btn.mi.dx = 0;
            btn.mi.dy = 0;
            btn.mi.mouseData = 0;
            btn.mi.time = 0;
            btn.mi.dwFlags = flag;
            inputs[inputCount++] = btn;
        } else {
            // If it's a mouse move: we already added the move. Nothing else needed.
            // For other messages (e.g., extra messages), we don't synthesize anything else.
        }
    }

    // Call SendInput to inject the synthetic events.
    // Marking flags with MOUSEEVENTF_MOVE | ABSOLUTE will make events show up as absolute.
    // The events will set MSLLHOOKSTRUCT::flags to injected (LLMHF_INJECTED), so our hook ignores them.
    UINT sent = SendInput(inputCount, inputs, sizeof(INPUT));
    (void)sent; // ignore for now; optionally handle error
}
