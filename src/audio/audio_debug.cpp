/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Audio engine debug logging macros and utilities.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "audio_debug.h"

#include "audio_instance.h"
#include "audio_manager.h"
#include "debughandler.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"

#include <chrono>
#include <d3d11.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <shellscalingapi.h>
#include <tchar.h>
#include <thread>

#ifndef NDEBUG


/**
 *  Forward declare message handler from imgui_impl_win32.cpp
 */
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


namespace AudioImGui
{

// Window handle
static HWND MainWindow = nullptr;

// Data
static ID3D11Device * D3DDevice = nullptr;
static ID3D11DeviceContext * D3DDeviceContext = nullptr;
static IDXGISwapChain * SwapChain = nullptr;
static ID3D11RenderTargetView * MainRenderTargetView = nullptr;

/**
 *  ImGui helper functions
 */
static void CreateRenderTarget()
{
    if (!D3DDevice) {
        return;
    }

    ID3D11Texture2D* pBackBuffer;
    SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    D3DDevice->CreateRenderTargetView(pBackBuffer, NULL, &MainRenderTargetView);
    pBackBuffer->Release();
}

static void CleanupRenderTarget()
{
    if (MainRenderTargetView) {
        MainRenderTargetView->Release();
        MainRenderTargetView = nullptr;
    }
}

static bool CreateDeviceD3D(HWND hWnd)
{
    if (D3DDevice) {
        return false;
    }

    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &SwapChain, &D3DDevice, &featureLevel, &D3DDeviceContext) != S_OK) {
        return false;
    }

    CreateRenderTarget();

    return true;
}

static void CleanupDeviceD3D()
{
    CleanupRenderTarget();

    if (SwapChain) {
        SwapChain->Release();
        SwapChain = nullptr;
    }
    if (D3DDeviceContext) {
        D3DDeviceContext->Release();
        D3DDeviceContext = nullptr;
    }
    if (D3DDevice) {
        D3DDevice->Release();
        D3DDevice = nullptr;
    }
}

/**
 *  ImGui Win32 message handler
 *  You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
 *  - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
 *  - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
 *  Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
 */
static LRESULT WINAPI Main_Window_Procedure(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam)) {
        return true;
    }

    switch (Message)
    {
        case WM_SIZE:
            if (D3DDevice != nullptr && wParam != SIZE_MINIMIZED)
            {
                CleanupRenderTarget();
                SwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                CreateRenderTarget();
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hWnd, Message, wParam, lParam);
}

static void New_Frame()
{
    if (!D3DDeviceContext || !SwapChain) {
        return;
    }

    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

static void Render_Frame()
{
    if (!D3DDeviceContext || !SwapChain) {
        return;
    }

    // Rendering
    ImGui::Render();

    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
    D3DDeviceContext->OMSetRenderTargets(1, &MainRenderTargetView, nullptr);
    D3DDeviceContext->ClearRenderTargetView(MainRenderTargetView, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    UINT sync_interval = 1; // 1 = with vsync, 0 = without vsync
    SwapChain->Present(sync_interval, 0);
}

static void End_Frame()
{
    ImGui::EndFrame();
}

}; // namespace AudioImGui end


namespace AudioUtil
{

/**
 *  Get the DPI scale of the monitor that the requested window is currently on.
 *
 *  @author: 273K @ https://stackoverflow.com/a/70794377
 */
static float Get_Monitor_DPI_Scale(HWND hWnd)
{
    float scMon = 1.0f;
    UINT x, y;

    HMONITOR monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
    if (SUCCEEDED(GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &x, &y)) && (x > 0) && (y > 0)) {
        scMon = 1.0f * x / USER_DEFAULT_SCREEN_DPI;           // 1.25
        // scMon = MulDiv(100, x, USER_DEFAULT_SCREEN_DPI);  // 125
    }

    return scMon;
}

}; // namespace AudioUtil end


/**
 *  State tracking for the audio debug ImGui window thread.
 */
static std::thread AudioDebugThread;
static std::atomic<bool> AudioDebugThread_Active{false};
static std::atomic<bool> AudioDebugThread_Running{false};
static std::atomic<bool> AudioDebugThread_InLoop{false};

/**
 *  Thread entry point that continuously runs the debug ImGui window loop.
 *
 *  @author: CCHyper
 */
DWORD WINAPI AudioImGuiWindowThread(LPVOID)
{
    static bool _window_created = false;

    DEBUG_INFO("Audio::Debug - Entering thread.\n");

    AudioDebugThread_Running.store(true, std::memory_order_relaxed);

    while (AudioDebugThread_Active.load(std::memory_order_relaxed)) {

        AudioDebugThread_InLoop.store(true, std::memory_order_relaxed);

        AudioManager.Debug_Window_Loop();

        // Sleep the thread to avoid busy-spinning.
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        AudioDebugThread_InLoop.store(false, std::memory_order_relaxed);
    }

    AudioDebugThread_Running.store(false, std::memory_order_relaxed);

    DEBUG_INFO("Audio::Debug - Exiting thread.\n");

    return 0;
}


/**
 *  Creates the debug ImGui window.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Create_Debug_Window()
{
    // Reset error codes
    SetLastError(0);

    DEBUG_INFO("Audio::Debug - Creating window.\n");

    ImGui_ImplWin32_EnableDpiAwareness();

    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = AudioImGui::Main_Window_Procedure;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = ProgramInstance;
    wc.hIcon = nullptr;
    wc.hCursor = nullptr;
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = "Audio Debug Window";
    wc.hIconSm = nullptr;

    BOOL rc = RegisterClassEx(&wc);
    if (!rc) {
        DEBUG_ERROR("Audio::Debug - Failed to register window class!\n");
        return false;
    }

    int win_width = 1000; //Options.ScreenWidth;
    int win_height = 800; //Options.ScreenHeight;

    HWND hwnd = CreateWindowEx(
        0,
        wc.lpszClassName,
        "Audio Debug Window",
        WS_OVERLAPPEDWINDOW|WS_VISIBLE|WS_POPUP,
        0,
        0,
        win_width,
        win_height,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr);

    if (!hwnd) {
        DEBUG_ERROR("Audio::Debug - Failed to create window!\n");
        return false;
    }

    DEBUG_INFO("Audio::Debug - Setting window size.\n");

    // Resposition and resize the window based on the monitor scale.
    float scale = AudioUtil::Get_Monitor_DPI_Scale(hwnd);

    SetWindowPos(hwnd,
        nullptr,
        GetSystemMetrics(SM_CXSCREEN) - win_width,
        GetSystemMetrics(SM_CYSCREEN) - win_height,
        win_width * scale,
        win_height * scale,
        SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

    DEBUG_INFO("Audio::Debug - Creating Direct3D device.\n");

    // Initialize Direct3D
    if (!AudioImGui::CreateDeviceD3D(hwnd)) {
        DEBUG_ERROR("Audio::Debug - Failed to create Direct3D device!\n");
        AudioImGui::CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return false;
    }

    // Store the window handle.
    AudioImGui::MainWindow = hwnd;

    // Show the window
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // The game hides the system cursor, this makes it show only in the ImGui window.
    io.MouseDrawCursor = true;

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    DEBUG_INFO("Audio::Debug - Setting up platform and renderer.\n");

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(AudioImGui::MainWindow);
    ImGui_ImplDX11_Init(AudioImGui::D3DDevice, AudioImGui::D3DDeviceContext);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    //ImGuiIO& io = ImGui::GetIO();
    //io.Fonts->

    DEBUG_INFO("Audio::Debug: Window created.\n");

    /**
     *  Start the ImGui window thread.
     */
    AudioDebugThread_Active.store(true, std::memory_order_relaxed);
    CreateThread(nullptr, 0, AudioImGuiWindowThread, nullptr, 0, nullptr);

    return true;
}


/**
 *  Shuts down the debug ImGui window and releases its D3D resources.
 *
 *  @author: CCHyper
 */
bool AudioManagerClass::Close_Debug_Window()
{
    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();

    ImGui::DestroyContext();

    AudioImGui::CleanupDeviceD3D();

    DestroyWindow(AudioImGui::MainWindow);
    //UnregisterClass(wc.lpszClassName, wc.hInstance);

    return true;
}


/**
 *  Polls and dispatches Win32 messages for the debug ImGui window.
 *
 *  @author: CCHyper
 */
void AudioManagerClass::Debug_Window_Message_Handler()
{
    if (!AudioImGui::MainWindow) {
        return;
    }

    MSG msg;

    // Poll and handle messages (inputs, window resize, etc.)
    // See the WndProc() function below for our to dispatch events to the Win32 backend.
    while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_QUIT) {
            break;
        }
    }
}


/**
 *  Debug logging handler and queue
 */
// Queue struct
typedef struct AudioDebugMessage {
    AudioDebugLogType type;
    AudioDebugLogLevel level;
    std::string message;
    std::string timestamp;
} AudioDebugMessage;

static std::mutex AudioDebugLogMutex;
static std::vector<AudioDebugMessage> AudioDebugLogQueue;

static void AudioSubmitToDebugLog(AudioDebugLogType type, AudioDebugLogLevel level, const std::string& msg)
{
    std::scoped_lock lock(AudioDebugLogMutex);
    AudioDebugLogQueue.push_back({type, level, msg});
}

void __cdecl Audio_Debug_Log(AudioDebugLogLevel level, AudioDebugLogType type, const char * message, ...)
{
    if (!Vinifera_AudioDebug) {
        return;
    }

    static char buffer[2048]; // Adjust buffer size if needed

    va_list args;
    va_start(args, message);
    vsnprintf(buffer, sizeof(buffer), message, args);
    va_end(args);

    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
    localtime_s(&now_tm, &now_time); // thread-safe
    char timeStr[32];
    std::strftime(timeStr, sizeof(timeStr), "[%H:%M:%S]", &now_tm);

    std::scoped_lock lock(AudioDebugLogMutex);
    AudioDebugLogQueue.push_back({type, level, buffer, timeStr});
}


/**
 *  Main debug window loop which draws the ImGui elements and child windows.
 *
 *  @author: CCHyper
 */
void AudioManagerClass::Debug_Window_Loop()
{
    if (!AudioImGui::MainWindow) {
        return;
    }

    // Must match AudioDebugLogType!
    static const char * AudioLogTabNames[] = {
        "Manager",
        "Instance",
        "Sample",
        "Ambient",
        "Thread",
        "Decoder",
        "IO",
        "Voc",
        "Vox",
        "Theme",
        "Hooks"
    };

    // Must match AudioDebugLogLevel!
    static const char * AudioLogLevelNames[] = {
        "Info",
        "Warning",
        "Error"
    };

    static ImGuiTextFilter LogFilter;
    static bool AutoScroll = true;

    static float LogScrollY[std::size(AudioLogTabNames)] = { 0.0f };
    static bool LogScrollToBottom[std::size(AudioLogTabNames)] = { true };

    /**
     *  Window begin!
     */

    AudioImGui::New_Frame();

    enum SelectedTabEnum {
        TAB_AUDIO_INFO = 0,
        TAB_AUDIO_LOG,
        TAB_GLOBALS,
        TAB_MISC,

        AUDIO_TAB_INFO = 100,
        AUDIO_TAB_TRACKER,
        AUDIO_TAB_MISC,
    };

    static SelectedTabEnum MenuBarSelectedTab = TAB_AUDIO_LOG;
    static int SelectedDebugLogTab = 0;  // TYPE_MANAGER, etc.

    ImGui::Begin("Toolbar", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize   |
        ImGuiWindowFlags_NoMove     |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_AlwaysAutoResize
    );
    ImGui::SetWindowPos(ImVec2(0, 0)); // top-left corner

    float buttonWidth = 100.0f;
    float spacing = ImGui::GetStyle().ItemSpacing.x;

    if (ImGui::Button("Audio Info", ImVec2(buttonWidth, 0))) MenuBarSelectedTab = TAB_AUDIO_INFO;
    ImGui::SameLine(0, spacing);
    if (ImGui::Button("Audio Logs", ImVec2(buttonWidth, 0))) MenuBarSelectedTab = TAB_AUDIO_LOG;
    ImGui::SameLine(0, spacing);
    if (ImGui::Button("Globals", ImVec2(buttonWidth, 0))) MenuBarSelectedTab = TAB_GLOBALS;
    ImGui::SameLine(0, spacing);
    if (ImGui::Button("Misc", ImVec2(buttonWidth, 0))) MenuBarSelectedTab = TAB_MISC;

    ImGui::End();

    switch (MenuBarSelectedTab)
    {
        case TAB_AUDIO_LOG:
        {
            ImGui::Begin("Audio Logs", nullptr, ImGuiWindowFlags_MenuBar);

            // --- Tab Selector ---
            if (ImGui::BeginMenuBar()) {
                for (int i = 0; i < std::size(AudioLogTabNames); ++i) {
                    if (ImGui::Button(AudioLogTabNames[i])) {
                        // Save scroll position of previous tab
                        LogScrollY[SelectedDebugLogTab] = ImGui::GetScrollY();
                        SelectedDebugLogTab = i;
                        // Restore scroll of new tab
                        LogScrollToBottom[i] = false; // Prevent auto-scroll override
                    }
                    ImGui::SameLine();
                }
                ImGui::EndMenuBar();
            }

            ImGui::Separator();
            LogFilter.Draw("Filter");
            ImGui::SameLine();
            ImGui::Checkbox("Auto-scroll", &AutoScroll);
            ImGui::SameLine();
            if (ImGui::Button("Copy to Clipboard")) {
                std::string clipboardText;
                std::scoped_lock lock(AudioDebugLogMutex);
                for (const auto& msg : AudioDebugLogQueue) {
                    if (msg.type != SelectedDebugLogTab) continue;
                    if (!LogFilter.PassFilter(msg.message.c_str())) continue;

                    clipboardText += msg.timestamp + " " + msg.message + "\n";
                }
                ImGui::SetClipboardText(clipboardText.c_str());
            }
            ImGui::Separator();
            
            ImGui::SetNextWindowSize(ImVec2(900, 600), ImGuiCond_Always);

            // --- Log Display Area ---
            ImGui::BeginChild("LogWindow", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

            // --- Log Rendering ---
            std::scoped_lock lock(AudioDebugLogMutex);

            for (const auto& msg : AudioDebugLogQueue) {
                if (msg.type != SelectedDebugLogTab) continue;
                if (!LogFilter.PassFilter(msg.message.c_str())) continue;

                ImVec4 color;
                switch (msg.level) {
                    case 1: color = ImVec4(1.0f, 1.0f, 0.3f, 1.0f); break; // Warning
                    case 2: color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); break; // Error
                    default: color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); break; // Info
                }

                ImGui::TextColored(color, "%s %s", msg.timestamp.c_str(), msg.message.c_str());
            }

            // --- Scroll Management ---
            if (AutoScroll) {
                if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 5.0f) {
                    // Auto-scroll only if already at bottom
                    ImGui::SetScrollHereY(1.0f);
                    LogScrollToBottom[SelectedDebugLogTab] = true;
                } else {
                    LogScrollToBottom[SelectedDebugLogTab] = false;
                }
            } else {
                // Preserve manual scroll position per tab
                LogScrollY[SelectedDebugLogTab] = ImGui::GetScrollY();
                ImGui::SetScrollY(LogScrollY[SelectedDebugLogTab]);
            }

            ImGui::EndChild();
            ImGui::End();
            break;
        }

        case TAB_AUDIO_INFO:
        {
            static SelectedTabEnum AudioMgrSelectedTab = AUDIO_TAB_INFO;

            ImGui::Begin("Audio Debug", nullptr, ImGuiWindowFlags_MenuBar);

            // Taskbar
            if (ImGui::BeginMenuBar()) {
                if (ImGui::Button("Audio")) AudioMgrSelectedTab = AUDIO_TAB_INFO;
                ImGui::SameLine();
                if (ImGui::Button("Globals")) AudioMgrSelectedTab = AUDIO_TAB_TRACKER;
                ImGui::SameLine();
                if (ImGui::Button("Misc")) AudioMgrSelectedTab = AUDIO_TAB_MISC;
                ImGui::EndMenuBar();
            }

            switch (AudioMgrSelectedTab)
            {
                default:
                case TAB_AUDIO_INFO:
                {
                    // Sub window for tracker counts & contents
                    ImGui::BeginChild("Trackers", ImVec2(300, 200), true);

                    ImGui::Text("Music.Count = %d", AudioManager.GroupedActiveInstanceMap[AUDIO_GROUP_MUSIC].size());
                    ImGui::Text("MusicAmbient.Count = %d", AudioManager.GroupedActiveInstanceMap[AUDIO_GROUP_AMBIENT].size());
                    ImGui::Text("Speech.Count = %d", AudioManager.GroupedActiveInstanceMap[AUDIO_GROUP_SPEECH].size());
                    ImGui::Text("SoundEffect.Count = %d", AudioManager.GroupedActiveInstanceMap[AUDIO_GROUP_SFX].size());
                    ImGui::Text("Event.Count = %d", AudioManager.GroupedActiveInstanceMap[AUDIO_GROUP_EVENT].size());

                    ImGui::Separator();

                    ImGui::Text("ActiveInstanceMap.Count = %d", static_cast<int>(AudioManager.ActiveInstanceMap.size()));
                    ImGui::Text("SamplesMap.Count = %d", static_cast<int>(AudioManager.SamplesMap.size()));

                    // lambda for printing the contents of a group list.
                    auto DisplayGroupList = [](const char* label, const std::vector<AudioInstanceClass*>& trackerGroup) {
                        if (ImGui::TreeNode(label)) {
                            for (size_t i = 0; i < trackerGroup.size(); ++i) {
                                const auto* item = trackerGroup[i];
                                if (item) {
                                    ImGui::Text("[%zu] Name: %s", i,
                                        !item->Get_FileName().empty()
                                            ? item->Get_FileName().c_str()
                                            : "<unnamed>");
                                } else {
                                    ImGui::Text("[%zu] <null>", i);
                                }
                            }
                            ImGui::TreePop();
                        }
                    };

                    DisplayGroupList("Music", AudioManager.GroupedActiveInstanceMap[AUDIO_GROUP_MUSIC]);
                    DisplayGroupList("Music Ambient", AudioManager.GroupedActiveInstanceMap[AUDIO_GROUP_AMBIENT]);
                    DisplayGroupList("Speech", AudioManager.GroupedActiveInstanceMap[AUDIO_GROUP_SPEECH]);
                    DisplayGroupList("Sound Effects", AudioManager.GroupedActiveInstanceMap[AUDIO_GROUP_SFX]);
                    DisplayGroupList("Events", AudioManager.GroupedActiveInstanceMap[AUDIO_GROUP_EVENT]);

                    ImGui::EndChild();

                    break;
                }
                case TAB_GLOBALS:
                {
                    // Globals tab content (stub)
                    ImGui::BeginChild("Globals", ImVec2(300, 200), true);

                    ImGui::Text("Globals section - not implemented yet.");

                    ImGui::EndChild();

                    break;
                }
                case TAB_MISC:
                {
                    // Misc tab content (stub)
                    ImGui::BeginChild("Misc", ImVec2(300, 200), true);

                    ImGui::Text("Misc section - not implemented yet.");

                    ImGui::EndChild();

                    break;
                }
            };

            ImGui::End();

            break;
        }

        case TAB_GLOBALS:
            break;

        case TAB_MISC:
            break;
    }

    /**
     *  Window end!
     */

    AudioImGui::Render_Frame();
    AudioImGui::End_Frame();
}

#endif // NDEBUG
