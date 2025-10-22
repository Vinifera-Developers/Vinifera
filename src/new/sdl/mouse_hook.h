// MouseScaler.h
#pragma once

#include <Windows.h>
#include <atomic>
#include <cassert>
#include <functional>
#include <thread>

// MouseScaler
// -----------------
// Usage:
//   MouseScaler scaler;
//   scaler.SetTargetWindow(MainWindow); // HWND MainWindow already created
//   scaler.SetScaleFactor(0.5);         // 50% scaling
//   scaler.Start();                     // starts background hook thread
//   ...
//   scaler.Stop();                      // stops hook and thread
//
// The scaler will detect mouse events over MainWindow and scale their coordinates
// relative to MainWindow's client area by the factor, then synthesize equivalent
// mouse events at the scaled screen coordinates. Non-target-window events pass
// through normally.

class MouseScaler
{
public:
    MouseScaler();
    ~MouseScaler();

    // Not copyable
    MouseScaler(const MouseScaler&) = delete;
    MouseScaler& operator=(const MouseScaler&) = delete;

    // Set the window whose input we want to scale.
    void SetTargetWindow(HWND hwnd) { m_targetWindow = hwnd; }

    // Scale factor: 1.0 = no change; 0.5 = half; 2.0 = double
    void SetScaleFactor(double factor) { m_scaleFactor.store(factor); }

    // Start the hook (spawns a background thread). Safe to call multiple times.
    bool Start();

    // Stop and join the background thread. Safe to call multiple times.
    void Stop();

    // Returns whether hook is currently running
    bool IsRunning() const { return m_running.load(); }

public:
    // Background thread entry: installs hook and runs message loop for lifetime of hook.
    void ThreadMain();

    // Installs/uninstalls hook on the background thread.
    bool InstallHook();
    void UninstallHook();

    // Helper: determine if screen point is over target window (including window frame).
    bool IsPointOverTargetWindow(POINT ptScreen);

    // Convert a screen point to the scaled screen point relative to client area.
    POINT ScalePointForWindow(POINT ptScreen);

    // Convert screen coordinates (x,y) to SendInput absolute coords (0..65535).
    static void ScreenPointToAbsolute(long x, long y, DWORD& outX, DWORD& outY);

    // Synthesize mouse events corresponding to the original mouse message but at scaled coords.
    void SynthesizeMouseEvent(UINT mouseMsg, const MSLLHOOKSTRUCT& orig);

public:
    HWND m_targetWindow = nullptr;
    std::atomic<double> m_scaleFactor {1.0};

    std::atomic<bool> m_running {false};
    std::atomic<bool> m_exitRequested {false};

    std::thread m_thread;
    HHOOK m_hook = nullptr;
    DWORD m_threadId = 0;

    // used to ignore events we synthesize ourselves
    static constexpr DWORD LLMHF_INJECTED_MASK = 0x00000001; // injected by SendInput
};
