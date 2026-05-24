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

#include <cstdarg>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <imgui.h>
#include <iterator>
#include <mutex>
#include <string>
#include <vector>

#ifndef NDEBUG


/**
 *  Debug logging handler and queue
 */
typedef struct AudioDebugMessage {
    AudioDebugLogType type;
    AudioDebugLogLevel level;
    std::string message;
    std::string timestamp;
} AudioDebugMessage;

static std::mutex AudioDebugLogMutex;
static std::vector<AudioDebugMessage> AudioDebugLogQueue;

/**
 *  Logs an audio debug message with timestamp to the debug queue.
 *
 *  @author: CCHyper
 */
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
 *  Draws the audio debug ImGui elements.
 *
 *  @author: CCHyper
 */
void AudioManagerClass::Draw_Debug_UI()
{
    // Must match AudioDebugLogType!
    static const char * AudioLogTabNames[] = {
        "Manager",
        "Instance",
        "Sample",
        "Thread",
        "Decoder",
        "IO",
        "Voc",
        "Vox",
        "Theme"
    };

    static ImGuiTextFilter LogFilter;
    static bool AutoScroll = true;

    static float LogScrollY[std::size(AudioLogTabNames)] = { 0.0f };
    static bool LogScrollToBottom[std::size(AudioLogTabNames)] = { true };

    enum SelectedTabEnum {
        TAB_AUDIO_INFO = 0,
        TAB_AUDIO_LOG
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
                        LogScrollY[SelectedDebugLogTab] = ImGui::GetScrollY();
                        SelectedDebugLogTab = i;
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

            ImGui::BeginChild("LogWindow", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

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
            ImGui::Begin("Audio Debug");

            ImGui::BeginChild("Trackers", ImVec2(300, 200), true);

            ImGui::Text("Music.Count = %d", AudioManager.GroupedActiveInstanceMap[AUDIO_GROUP_MUSIC].size());
            ImGui::Text("Speech.Count = %d", AudioManager.GroupedActiveInstanceMap[AUDIO_GROUP_SPEECH].size());
            ImGui::Text("SoundEffect.Count = %d", AudioManager.GroupedActiveInstanceMap[AUDIO_GROUP_SFX].size());
            ImGui::Text("UI.Count = %d", AudioManager.GroupedActiveInstanceMap[AUDIO_GROUP_UI].size());
            ImGui::Text("Streaming.Count = %d", AudioManager.GroupedActiveInstanceMap[AUDIO_GROUP_STREAMING].size());

            ImGui::Separator();

            ImGui::Text("ActiveInstanceMap.Count = %d", static_cast<int>(AudioManager.ActiveInstanceMap.size()));
            ImGui::Text("SamplesMap.Count = %d", static_cast<int>(AudioManager.SamplesMap.size()));

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
            DisplayGroupList("Speech", AudioManager.GroupedActiveInstanceMap[AUDIO_GROUP_SPEECH]);
            DisplayGroupList("Sound Effects", AudioManager.GroupedActiveInstanceMap[AUDIO_GROUP_SFX]);
            DisplayGroupList("UI", AudioManager.GroupedActiveInstanceMap[AUDIO_GROUP_UI]);
            DisplayGroupList("Streaming", AudioManager.GroupedActiveInstanceMap[AUDIO_GROUP_STREAMING]);

            ImGui::EndChild();

            ImGui::End();

            break;
        }
    }

}

#endif // NDEBUG
