/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended SessionClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "extension.h"
#include "session.h"
#include "wolapi.h"

#include <chrono>
#include <optional>
#include <string>


class SessionClassExtension final : public GlobalExtensionClass<SessionClass>
{
    public:
        IFACEMETHOD(Load)(IStream *pStm);
        IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

    public:
        SessionClassExtension(const SessionClass *this_ptr);
        ~SessionClassExtension() override;

        virtual int Get_Object_Size() const override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        void Init_Clear();

        int Get_Autosave_Interval() const;
        void Schedule_Next_Autosave();
        void Flag_To_Save(bool manual);
        void Disable_Multiplayer_Saves();
        void Set_Next_Campaign_Autosave_Slot(int slot);
        void Set_Next_Skirmish_Autosave_Slot(int slot);
        void Restore_Autosave_After_Load();
        void Service_Autosave_After_Main_Loop();
        bool Load_Multiplayer_Save(int slot);
        bool Reconcile_Players();

        bool Is_Out_of_Sync(int id);
        void Clear_Out_Of_Sync_Data();
        void Mark_Player_As_Out_of_Sync(int id);

        void Set_Master(int house_id);
        void Announce_Master();
        void Update_Master_After_Player_Removal();

        bool Are_Statistics_Enabled() const;
        bool Are_Extra_Statistics_Enabled() const;

    private:
        static std::string Multiplayer_Save_File_Name_From_Index(int index);
        void Init_Multiplayer_Saves_For_Session();
        std::string Multiplayer_Save_File_Name() const;
        std::string Autosave_File_Name() const;
        std::string Autosave_Description() const;

    public:

        const char *Name() const override { return "Session"; }
        const char *Full_Name() const override { return "Session"; }

    public:
        struct ExtGameOptionsType {

            /**
             *  Should the MCV unit auto deploy on game start?
             */
            bool IsAutoDeployMCV = false;

            /**
             *  Are construction yards pre-placed on the map rather than a MCV given to the player?
             */
            bool IsPrePlacedConYards = false;

            /**
             *  Can players build their own structures adjacent to structures owned by their allies?
             */
            bool IsBuildOffAlly = true;

            /**
             *  Autosave interval for multiplayer spawner sessions.
             */
            int MultiplayerAutoSaveInterval = 0;

            /**
             *  Should player identity be hidden for quick match?
             */
            bool IsQuickMatch = false;

            /**
             *  Should statistics be written for the current match?
             */
            bool IsWriteStatistics = false;

            /**
             *  Should disconnected players be eliminated instead of handed to the AI?
             */
            bool IsAutoSurrender = false;

            /**
             *  Can armed units attack multiplayer neutral houses?
             */
            bool IsAttackNeutralUnits = false;

            /**
             *  Should defeated players be denied observer vision?
             */
            bool IsCoachMode = false;

            /**
             *  Should the game continue when no human players remain?
             */
            bool IsContinueWithoutHumans = false;

            /**
             *  Should destroyed technos use scrap explosions?
             */
            bool IsScrapMetal = false;

            /**
             *  Should AI players be renamed according to their selected difficulty?
             */
            bool IsAINamesByDifficulty = false;

            /**
             *  Should scenario movies be played in multiplayer?
             */
            bool IsPlayMoviesInMultiplayer = false;

        };

        ExtGameOptionsType ExtOptions;

        struct AutoSaveStateType {

            /**
             *  Has an autosave been queued to run from the main-loop safe point?
             */
            bool IsToSave = false;

            /**
             *  Is the next multiplayer save a manual save?
             */
            bool IsNextMultiplayerSaveManual = false;

            /**
             *  Frame on which the next periodic autosave should trigger.
             */
            int NextAutoSaveFrame = -1;

            /**
             *  Next rotating campaign autosave slot, stored as a 0-based index.
             */
            int NextCampaignAutoSaveSlot = 0;

            /**
             *  Next rotating skirmish autosave slot, stored as a 0-based index.
             */
            int NextSkirmishAutoSaveSlot = 0;

            /**
             *  Have multiplayer saves been suppressed for the current session
             *  (usually due to desync or player disconnnection)?
             */
            bool IsMultiplayerSaveSuppressed = false;
        };

        AutoSaveStateType AutoSave;
        bool IsSpawnerSession = false;
        bool MultiplayerSavesInitializedForThisSession = false;

        /**
         *  Spawner-supplied metadata for the current session. Populated only when
         *  running under the spawner; consumed by the loading-screen override,
         *  the statistics packet, and the in-game difficulty banner.
         */
        struct SpawnerSessionInfoType {

            /**
             *  Map identifier reported in the statistics packet "SCEN" field.
             */
            std::string StatsMapName;

            /**
             *  Map hash reported in the statistics packet "HASH" field.
             */
            std::string StatsMapHash;

            /**
             *  Custom difficulty name shown in the in-game difficulty banner;
             *  empty means fall back to the stock difficulty name.
             */
            std::string DifficultyName;

            /**
             *  Loading-screen filename (including extension) that overrides the
             *  scenario/UIControls pick; empty means no override.
             */
            std::string CustomLoadScreen;

            /**
             *  Optional override for the loading-screen progress-bar position.
             */
            std::optional<Point2D> CustomLoadScreenPos;
        };

        SpawnerSessionInfoType SpawnerInfo;

        struct SpawnerSlotInfoType {
            bool IsConfigured = false;
            bool IsHuman = false;
            int Color = -1;
            int House = -1;
            int Difficulty = -1;
            bool IsObserver = false;
            int SpawnLocation = -1;
            int Alliances[MAX_PLAYERS] = {-1, -1, -1, -1, -1, -1, -1, -1};
        };

        bool ProtocolZeroEnabled = false;
        unsigned char ProtocolZeroMaxLatencyLevel = 0xFF;
        int ConnTimeout = 0;
        SpawnerSlotInfoType SlotInfo[MAX_PLAYERS];

        /**
         *  Are we the original host of the current game session?
         */
        bool IsOriginalHost = false;

        /**
         *  Keeps a record of which players are out of sync with us.
         */
        bool IsOutOfSync[MAX_PLAYERS];

        /**
         *  Frame when the game first went out of sync.
         */
        int OutOfSyncFrame = -1;

        /**
         *  Is the message we're currently writing meant to be sent to allies only?
         */
        bool IsChatToAllies = false;

        /**
         *  If we're writing a private message, this is the name of its recipient.
         */
        char MessageRecipientName[32] = "";

        /**
         *  Convenient property to access IsGDI as a HousesType.
         */
        HousesType Get_House() const { return static_cast<HousesType>(reinterpret_cast<unsigned char&>(This()->IsGDI)); }
        void Set_House(HousesType house) { reinterpret_cast<unsigned char&>(This()->IsGDI) = house; }
        __declspec(property(get = Get_House, put = Set_House)) HousesType House;
};
