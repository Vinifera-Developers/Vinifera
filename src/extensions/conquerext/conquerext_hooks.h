/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for conquer.cpp.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

void ConquerExtension_Hooks();

/**
 *  Processes incoming global network packets (chat, sign-offs, beacons,
 *  load requests, desync coordination...). Called from the IPX callback and,
 *  while the desync dialog is open, from its pump loop.
 */
void Vinifera_Process_Incoming_Global_Packets();

/**
 *  Sends a network chat message to the other players, using the current
 *  Session.MessageAddress and SessionExtension->IsChatToAllies settings.
 */
void Vinifera_Send_Network_Chat(const char* text);