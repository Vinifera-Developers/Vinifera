/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended SessionClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "sessionext.h"

#include "mouse.h"
#include "optionsext.h"
#include "rules.h"
#include "saveload.h"
#include "scenario.h"
#include "textprint.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"

#include <format>


namespace
{
    void Print_Saving_Game_Message()
    {
        const int message_delay = Rule->MessageDelay * TICKS_PER_MINUTE;
        Session.Messages.Add_Message(nullptr, 0, "Saving game...", static_cast<ColorSchemeType>(4), TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, message_delay);

        Map.Flag_To_Redraw(2);
        Map.Render();
    }
}


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
SessionClassExtension::SessionClassExtension(const SessionClass *this_ptr) :
    GlobalExtensionClass(this_ptr)
{
    //if (this_ptr) EXT_DEBUG_TRACE("SessionClassExtension::SessionClassExtension - 0x%08X\n", (uintptr_t)(ThisPtr));
    Init_Clear();
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
SessionClassExtension::~SessionClassExtension()
{
    //EXT_DEBUG_TRACE("SessionClassExtension::~SessionClassExtension - 0x%08X\n", (uintptr_t)(ThisPtr));
}


/**
 *  Loads game options and autosave state from the stream.
 *
 *  @author: ZivDero
 */
HRESULT SessionClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("SessionClassExtension::Load - 0x%08X\n", (uintptr_t)(This()));

    if (!pStm) {
        return E_POINTER;
    }

    HRESULT hr = pStm->Read(&ExtOptions, sizeof(ExtOptions), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    hr = pStm->Read(&AutoSave, sizeof(AutoSave), nullptr);
    return hr;
}


/**
 *  Saves game options and autosave state to the stream.
 *
 *  @author: ZivDero
 */
HRESULT SessionClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("SessionClassExtension::Save - 0x%08X\n", (uintptr_t)(This()));

    static_cast<void>(fClearDirty);

    if (!pStm) {
        return E_POINTER;
    }

    HRESULT hr = pStm->Write(&ExtOptions, sizeof(ExtOptions), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    hr = pStm->Write(&AutoSave, sizeof(AutoSave), nullptr);
    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: CCHyper
 */
int SessionClassExtension::Get_Object_Size() const
{
    //EXT_DEBUG_TRACE("SessionClassExtension::Get_Object_Size - 0x%08X\n", (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void SessionClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("SessionClassExtension::Object_CRC - 0x%08X\n", (uintptr_t)(This()));

    crc(ExtOptions.IsAutoDeployMCV);
    crc(ExtOptions.IsPrePlacedConYards);
    crc(ExtOptions.IsBuildOffAlly);
    crc(ExtOptions.MultiplayerAutoSaveInterval);
    crc(ExtOptions.IsQuickMatch);
    crc(ExtOptions.IsWriteStatistics);
    crc(ExtOptions.IsAutoSurrender);
    crc(ExtOptions.IsAttackNeutralUnits);
    crc(ExtOptions.IsCoachMode);
    crc(ExtOptions.IsContinueWithoutHumans);
    crc(ExtOptions.IsScrapMetal);
    crc(ExtOptions.IsAINamesByDifficulty);
    crc(AutoSave.IsToSave);
    crc(AutoSave.NextAutoSaveFrame);
    crc(AutoSave.NextSPAutoSaveSlot);
    crc(AutoSave.IsMultiplayerAutoSaveSuppressed);
}


/**
 *  Resets all non-option session extension state for a new session.
 *
 *  @author: ZivDero
 */
void SessionClassExtension::Init_Clear()
{
    ExtOptions = ExtGameOptionsType();
    AutoSave = AutoSaveStateType();
    IsSpawnerSession = false;
    ProtocolZeroEnabled = false;
    ProtocolZeroMaxLatencyLevel = 0xFF;
    ConnTimeout = 0;

    for (auto& slot_info : SlotInfo) {
        slot_info = SpawnerSlotInfoType();
    }

    IsChatToAllies = false;
    std::memset(MessageRecipientName, '\0', sizeof(MessageRecipientName));
}


/**
 *  Gets the autosave interval in frames for the current session, if any.
 *
 *  @author: ZivDero
 */
int SessionClassExtension::Get_Autosave_Interval() const
{
    if (Session.Singleplayer_Game() && OptionsExtension->AutoSaveCount > 0 && OptionsExtension->AutoSaveInterval > 0) {
        return OptionsExtension->AutoSaveInterval;
    }

    if ((Session.Type == GAME_IPX || Session.Type == GAME_INTERNET) && !AutoSave.IsMultiplayerAutoSaveSuppressed && ExtOptions.MultiplayerAutoSaveInterval > 0) {
        return ExtOptions.MultiplayerAutoSaveInterval;
    }

    return 0;
}


/**
 *  Schedules the next periodic autosave based on the current frame and session state.
 *
 *  @author: ZivDero
 */
void SessionClassExtension::Schedule_Next_Autosave()
{
    const int interval = Get_Autosave_Interval();
    AutoSave.NextAutoSaveFrame = interval > 0 ? Frame + interval : -1;
}


/**
 *  Queues an autosave to run from the main-loop safe point.
 *
 *  @author: ZivDero
 */
void SessionClassExtension::Flag_To_Save()
{
    AutoSave.IsToSave = true;
}


/**
 *  Suppresses multiplayer autosaves for the remainder of the current session.
 *
 *  @author: ZivDero
 */
void SessionClassExtension::Disable_Multiplayer_Autosaves()
{
    AutoSave.IsMultiplayerAutoSaveSuppressed = true;
    Schedule_Next_Autosave();
}


/**
 *  Sets the next campaign autosave slot using the internal 0-based slot numbering.
 *
 *  @author: ZivDero
 */
void SessionClassExtension::Set_Next_Campaign_Autosave_Slot(int slot)
{
    AutoSave.NextSPAutoSaveSlot = slot >= 0 ? slot : 0;
}


/**
 *  Restores autosave state after loading a savegame.
 *
 *  @author: ZivDero
 */
void SessionClassExtension::Restore_Autosave_After_Load()
{
    AutoSave.IsToSave = false;

    if (Get_Autosave_Interval() <= 0) {
        AutoSave.NextAutoSaveFrame = -1;
        return;
    }

    if (AutoSave.NextAutoSaveFrame <= Frame) {
        Schedule_Next_Autosave();
    }
}


/**
 *  Services autosaves from the post-main-loop safe point.
 *
 *  @author: ZivDero
 */
void SessionClassExtension::Service_Autosave_After_Main_Loop()
{
    if (Frame == AutoSave.NextAutoSaveFrame) {
        Flag_To_Save();
    }

    if (!AutoSave.IsToSave) {
        return;
    }

    Print_Saving_Game_Message();

    if (Session.Singleplayer_Game()) {
        AutoSave.IsToSave = false;
        Schedule_Next_Autosave();

        Pause_Scenario();
        Call_Back();
        Save_Game(Autosave_File_Name().c_str(), Autosave_Description().c_str());
        Resume_Scenario();

        if (OptionsExtension->AutoSaveCount > 0) {
            AutoSave.NextSPAutoSaveSlot = (AutoSave.NextSPAutoSaveSlot + 1) % OptionsExtension->AutoSaveCount;
        }

        return;
    }
    
    if (Session.Type == GAME_IPX || Session.Type == GAME_INTERNET) {
        AutoSave.IsToSave = false;
        Schedule_Next_Autosave();

        Save_Game(Autosave_File_Name().c_str(), Autosave_Description().c_str());
        return;
    }

    AutoSave.IsToSave = false;
    Schedule_Next_Autosave();
}


std::string SessionClassExtension::Autosave_File_Name() const
{
    switch (Session.Type) {
    case GAME_IPX:
    case GAME_INTERNET:
        return NET_SAVE_FILE_NAME;
    case GAME_NORMAL:
    case GAME_SKIRMISH:
        return std::format("AUTOSAVE{}.SAV", AutoSave.NextSPAutoSaveSlot + 1);
    default:
        return "";
    }
}


std::string SessionClassExtension::Autosave_Description() const
{
    switch (Session.Type) {
    case GAME_IPX:
    case GAME_INTERNET:
        return "Multiplayer Game";
    case GAME_NORMAL:
        return std::format("Mission Auto-Save (Slot {})", AutoSave.NextSPAutoSaveSlot + 1);
    case GAME_SKIRMISH:
        return std::format("Skirmish Auto-Save (Slot {})", AutoSave.NextSPAutoSaveSlot + 1);
    default:
        return "";
    }
}
