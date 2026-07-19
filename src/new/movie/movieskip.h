/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Multiplayer consensus for skipping fullscreen movies.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once


class IPXAddressClass;
struct ExtGlobalPacketType;


namespace MovieSkip
{
    /**
     *  Temporarily makes multiplayer movie ESC handling local. End-game
     *  movies use this after gameplay and score synchronization have ended.
     */
    class LocalSkipScope
    {
    public:
        LocalSkipScope();
        ~LocalSkipScope();

        LocalSkipScope(const LocalSkipScope &) = delete;
        LocalSkipScope &operator=(const LocalSkipScope &) = delete;
    };

    /**
     *  Returns whether the active movie may be skipped locally in multiplayer.
     */
    bool Is_Local_Skip_Allowed();

    /**
     *  Starts a new fullscreen movie vote context. Single-player movies are
     *  ignored so their normal immediate ESC handling remains unchanged.
     */
    void Begin(const char *movie_name);

    /**
     *  Ends the current fullscreen movie vote context.
     */
    void End();

    /**
     *  Samples ESC without consuming the game's keyboard queue and broadcasts
     *  a vote when an ESC event or rising edge is observed.
     */
    void Update_Input();

    /**
     *  Returns true once every currently connected human player has voted.
     */
    bool Should_Skip();

    /**
     *  Places a synthetic ESC release in the legacy VQA input queue once the
     *  vote is unanimous, allowing the original breakout path to run.
     */
    void Prepare_Legacy_Breakout();

    /**
     *  Handles an incoming global movie-skip vote packet.
     */
    void Receive_Vote(const ExtGlobalPacketType &packet, IPXAddressClass &address);

    /**
     *  Draws the non-interactive vote status overlay during an ImGui frame.
     */
    void Draw_Overlay();
}
