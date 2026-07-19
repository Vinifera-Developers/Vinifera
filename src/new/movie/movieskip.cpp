/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Multiplayer consensus for skipping fullscreen movies.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "movieskip.h"

#include "debughandler.h"
#include "ipxaddr.h"
#include "ipxmgr.h"
#include "movieplayback.h"
#include "scenario.h"
#include "session.h"
#include "tibsun_globals.h"
#include "vinifera_defines.h"
#include "vinifera_globals.h"
#include "wwkeyboard.h"

#include <imgui.h>

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>


namespace
{
    constexpr DWORD LEGACY_OVERLAY_ACTIVITY_WINDOW = 250;
    constexpr float MOVIE_SKIP_OVERLAY_FONT_SCALE = 1.5f;

    struct MovieVoteToken
    {
        std::uint32_t Context = 0;
        std::uint32_t Movie = 0;
        std::uint32_t Instance = 0;

        bool operator==(const MovieVoteToken &other) const
        {
            return Context == other.Context && Movie == other.Movie && Instance == other.Instance;
        }
    };

    struct MovieSkipState
    {
        bool Active = false;
        bool EscWasDown = false;
        bool LegacyBreakoutPrepared = false;
        MovieVoteToken Token;
        std::array<bool, MAX_PLAYERS> Votes = {};
        DWORD LastPlaybackUpdate = 0;
    };

    MovieSkipState State;
    std::uint32_t SequenceContext = 0;
    std::uint32_t NextMovieInstance = 0;
    int LocalSkipScopeDepth = 0;


    void Hash_Bytes(std::uint32_t &hash, const void *data, std::size_t size)
    {
        const auto *bytes = static_cast<const unsigned char *>(data);
        for (std::size_t i = 0; i < size; ++i) {
            hash ^= bytes[i];
            hash *= 16777619u;
        }
    }


    void Hash_String_Case_Insensitive(std::uint32_t &hash, const char *text)
    {
        if (!text) {
            return;
        }

        while (*text) {
            const unsigned char character = static_cast<unsigned char>(*text++);
            const unsigned char upper = character >= 'a' && character <= 'z'
                ? static_cast<unsigned char>(character - ('a' - 'A'))
                : character;
            Hash_Bytes(hash, &upper, sizeof(upper));
        }
    }


    std::uint32_t Build_Context_Hash()
    {
        std::uint32_t hash = 2166136261u;

        Hash_String_Case_Insensitive(hash, Scen ? Scen->ScenarioName : "");

        /**
         *  Session.Players[0] is local on every client, so its vector order is
         *  not shared. Hash players by house ID to obtain the same context on
         *  every peer.
         */
        for (int player_id = 0; player_id < MAX_PLAYERS; ++player_id) {
            for (int i = 0; i < Session.Players.Count(); ++i) {
                const NodeNameType *player = Session.Players[i];
                if (player && static_cast<int>(player->Player.ID) == player_id) {
                    Hash_Bytes(hash, &player_id, sizeof(player_id));
                    Hash_String_Case_Insensitive(hash, player->Name);
                    break;
                }
            }
        }

        return hash;
    }


    std::uint32_t Build_Movie_Hash(const char *movie_name)
    {
        std::uint32_t hash = 2166136261u;
        const std::string normalized = Normalize_Movie_Basename(movie_name);
        Hash_Bytes(hash, normalized.data(), normalized.size());
        return hash;
    }


    int Local_Player_ID()
    {
        if (Session.Players.Count() == 0 || !Session.Players[0]) {
            return -1;
        }

        const int player_id = static_cast<int>(Session.Players[0]->Player.ID);
        return player_id >= 0 && player_id < MAX_PLAYERS ? player_id : -1;
    }


    int Player_ID_From_Address(IPXAddressClass &address)
    {
        for (int i = 1; i < Session.Players.Count(); ++i) {
            NodeNameType *player = Session.Players[i];
            if (player && player->Address == address) {
                const int player_id = static_cast<int>(player->Player.ID);
                return player_id >= 0 && player_id < MAX_PLAYERS ? player_id : -1;
            }
        }

        return -1;
    }


    const char *Player_Name(int player_id)
    {
        for (int i = 0; i < Session.Players.Count(); ++i) {
            const NodeNameType *player = Session.Players[i];
            if (player && static_cast<int>(player->Player.ID) == player_id) {
                return player->Name;
            }
        }

        return "Unknown player";
    }


    void Broadcast_Local_Vote()
    {
        const int player_id = Local_Player_ID();
        if (!State.Active || player_id < 0 || State.Votes[player_id]) {
            return;
        }

        State.Votes[player_id] = true;

        ExtGlobalPacketType packet {};
        packet.Command = EXT_NET_MOVIE_SKIP_VOTE;
        if (Session.Players.Count() > 0 && Session.Players[0]) {
            std::strncpy(packet.Name, Session.Players[0]->Name, sizeof(packet.Name) - 1);
        }
        packet.MovieSkipVote.Context = State.Token.Context;
        packet.MovieSkipVote.Movie = State.Token.Movie;
        packet.MovieSkipVote.Instance = State.Token.Instance;

        DEBUG_INFO("Movie skip: player {} voted for token {:08x}/{:08x}/{}.\n",
            player_id, State.Token.Context, State.Token.Movie, State.Token.Instance);

        for (int i = 1; i < Session.Players.Count(); ++i) {
            if (Session.Players[i]) {
                Ipx.Send_Global_Message(&packet, sizeof(packet), 1, &Session.Players[i]->Address);
            }
        }
        Ipx.Service();
    }


    int Connected_Player_Count()
    {
        int count = 0;
        for (int i = 0; i < Session.Players.Count(); ++i) {
            const NodeNameType *player = Session.Players[i];
            const int player_id = player ? static_cast<int>(player->Player.ID) : -1;
            if (player_id >= 0 && player_id < MAX_PLAYERS) {
                ++count;
            }
        }
        return count;
    }


    int Vote_Count()
    {
        int count = 0;
        for (int i = 0; i < Session.Players.Count(); ++i) {
            const NodeNameType *player = Session.Players[i];
            const int player_id = player ? static_cast<int>(player->Player.ID) : -1;
            if (player_id >= 0 && player_id < MAX_PLAYERS && State.Votes[player_id]) {
                ++count;
            }
        }
        return count;
    }
}


MovieSkip::LocalSkipScope::LocalSkipScope()
{
    ++LocalSkipScopeDepth;
}


MovieSkip::LocalSkipScope::~LocalSkipScope()
{
    --LocalSkipScopeDepth;
}


bool MovieSkip::Is_Local_Skip_Allowed()
{
    return LocalSkipScopeDepth > 0;
}


void MovieSkip::Begin(const char *movie_name)
{
    State = MovieSkipState {};

    if (Session.Singleplayer_Game() || Is_Local_Skip_Allowed()) {
        return;
    }

    const std::uint32_t context = Build_Context_Hash();
    if (SequenceContext != context) {
        SequenceContext = context;
        NextMovieInstance = 0;
    }

    State.Active = true;
    State.Token.Context = context;
    State.Token.Movie = Build_Movie_Hash(movie_name);
    State.Token.Instance = ++NextMovieInstance;
    State.EscWasDown = Keyboard && Keyboard->Down(KN_ESC);
    State.LastPlaybackUpdate = timeGetTime();

    DEBUG_INFO("Movie skip: begin token {:08x}/{:08x}/{} for \"{}\".\n",
        State.Token.Context, State.Token.Movie, State.Token.Instance, movie_name ? movie_name : "");
}


void MovieSkip::End()
{
    if (State.Active) {
        DEBUG_INFO("Movie skip: end token {:08x}/{:08x}/{}.\n",
            State.Token.Context, State.Token.Movie, State.Token.Instance);
    }

    State = MovieSkipState {};
}


void MovieSkip::Update_Input()
{
    if (!State.Active || Session.Singleplayer_Game() || !Keyboard) {
        return;
    }

    State.LastPlaybackUpdate = timeGetTime();

    const bool esc_down = Keyboard->Down(KN_ESC);
    const unsigned short queued_key = Keyboard->Check();
    const bool esc_queued = (queued_key & 0xFF) == KN_ESC;

    if (esc_queued || (esc_down && !State.EscWasDown)) {
        Broadcast_Local_Vote();
    }
    State.EscWasDown = esc_down;
}


bool MovieSkip::Should_Skip()
{
    if (!State.Active || Session.Singleplayer_Game()) {
        return false;
    }

    bool found_player = false;
    for (int i = 0; i < Session.Players.Count(); ++i) {
        const NodeNameType *player = Session.Players[i];
        const int player_id = player ? static_cast<int>(player->Player.ID) : -1;
        if (player_id < 0 || player_id >= MAX_PLAYERS) {
            continue;
        }

        found_player = true;
        if (!State.Votes[player_id]) {
            return false;
        }
    }

    return found_player;
}


void MovieSkip::Prepare_Legacy_Breakout()
{
    if (!Should_Skip() || State.LegacyBreakoutPrepared || !Keyboard) {
        return;
    }

    /**
     *  The legacy VQA loop still owns the actual cleanup path. Give its normal
     *  ESC check a deterministic key to consume on every client.
     */
    Keyboard->Clear();
    Keyboard->Put(KN_RLSE_BIT | KN_ESC);
    State.LegacyBreakoutPrepared = true;
}


void MovieSkip::Receive_Vote(const ExtGlobalPacketType &packet, IPXAddressClass &address)
{
    const MovieVoteToken token {
        packet.MovieSkipVote.Context,
        packet.MovieSkipVote.Movie,
        packet.MovieSkipVote.Instance
    };

    if (!State.Active || !(State.Token == token)) {
        DEBUG_INFO("Movie skip: ignored an inactive or stale vote for token {:08x}/{:08x}/{}.\n",
            token.Context, token.Movie, token.Instance);
        return;
    }

    const int player_id = Player_ID_From_Address(address);
    if (player_id < 0) {
        DEBUG_WARNING("Movie skip: ignored a vote from an unknown player address.\n");
        return;
    }

    State.Votes[player_id] = true;
    DEBUG_INFO("Movie skip: received player {} vote for token {:08x}/{:08x}/{}.\n",
        player_id, token.Context, token.Movie, token.Instance);
}


void MovieSkip::Draw_Overlay()
{
    if (!State.Active || Vote_Count() == 0) {
        return;
    }

    /**
     *  Modern movies explicitly render an ImGui pass. Legacy VQA movies pass
     *  through the regular screen updater; its recent playback pulse prevents
     *  stale state from appearing after the legacy function returns.
     */
    if (!Vinifera_ModernMoviePlaying && timeGetTime() - State.LastPlaybackUpdate >= LEGACY_OVERLAY_ACTIVITY_WINDOW) {
        return;
    }

    const int player_count = Connected_Player_Count();
    const int vote_count = Vote_Count();
    const int local_player_id = Local_Player_ID();
    const bool local_voted = local_player_id >= 0 && State.Votes[local_player_id];

    if (player_count <= 0) {
        return;
    }

    std::vector<int> remote_voters;
    for (int i = 0; i < Session.Players.Count(); ++i) {
        const NodeNameType *player = Session.Players[i];
        const int player_id = player ? static_cast<int>(player->Player.ID) : -1;
        if (player_id >= 0 && player_id < MAX_PLAYERS && player_id != local_player_id && State.Votes[player_id]) {
            remote_voters.push_back(player_id);
        }
    }

    std::string status;
    if (remote_voters.size() == 1) {
        status = Player_Name(remote_voters.front());
        status += " wants to skip the video.";
    } else if (remote_voters.empty() && local_voted) {
        status = "You want to skip the video.";
    } else {
        status = std::to_string(vote_count) + " players want to skip the video.";
    }

    char prompt[96];
    if (local_voted) {
        std::snprintf(prompt, sizeof(prompt), "Waiting for other players... (%d/%d)", vote_count, player_count);
    } else {
        std::snprintf(prompt, sizeof(prompt), "Press ESC to agree. (%d/%d)", vote_count, player_count);
    }

    ImGui::SetNextWindowPos(ImVec2(12.0f, 12.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.72f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 8.0f));
    ImGui::PushFont(nullptr, ImGui::GetStyle().FontSizeBase * MOVIE_SKIP_OVERLAY_FONT_SCALE);

    constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoInputs
        | ImGuiWindowFlags_NoNav
        | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoFocusOnAppearing;

    if (ImGui::Begin("##MovieSkipVote", nullptr, flags)) {
        ImGui::TextUnformatted(status.c_str());
        ImGui::TextUnformatted(prompt);
    }
    ImGui::End();
    ImGui::PopFont();
    ImGui::PopStyleVar();
}
