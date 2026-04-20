/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SESSIONEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended SessionClass class.
 *
 *  @license       Vinifera is free software: you can redistribute it and/or
 *                 modify it under the terms of the GNU General Public License
 *                 as published by the Free Software Foundation, either version
 *                 3 of the License, or (at your option) any later version.
 *
 *                 Vinifera is distributed in the hope that it will be
 *                 useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *                 PURPOSE. See the GNU General Public License for more details.
 *
 *                 You should have received a copy of the GNU General Public
 *                 License along with this program.
 *                 If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#pragma once

#include "extension.h"
#include "session.h"
#include "wolapi.h"


class SessionClassExtension final : public GlobalExtensionClass<SessionClass>
{
    public:
        IFACEMETHOD(Load)(IStream *pStm);
        IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

    public:
        SessionClassExtension(const SessionClass *this_ptr);
        SessionClassExtension(const NoInitClass &noinit);
        virtual ~SessionClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual const char *Name() const override { return "Session"; }
        virtual const char *Full_Name() const override { return "Session"; }

        void Read_MultiPlayer_Settings();
        void Write_MultiPlayer_Settings();
        void Clear_Spawner_State();
        void Apply_Spawner_Runtime_State() const;

    public:
        typedef struct ExtGameOptionsType
        {
            /**
             *  Should the MCV unit auto deploy on game start?
             */
            bool IsAutoDeployMCV;

            /**
             *  Are construction yards pre-placed on the map rather than a MCV given to the player?
             */
            bool IsPrePlacedConYards;

            /**
             *  Can players build their own structures adjacent to structures owned by their allies?
             */
            bool IsBuildOffAlly;

        } ExtGameOptionsType;

        ExtGameOptionsType ExtOptions;
        
        struct SpawnerGameOptionsType {
            int MultiplayerAutoSaveInterval;
            bool QuickMatch;
            bool WriteStatistics;
            bool AutoSurrender;
            bool AttackNeutralUnits;
            bool CoachMode;
            bool ContinueWithoutHumans;
            bool ScrapMetal;
            bool AINamesByDifficulty;
            bool ProtocolZeroEnabled;
            unsigned char ProtocolZeroMaxLatencyLevel;
            int ReconnectTimeout;
            int Tournament;
            unsigned long GameID;
        };

        struct SpawnerSlotInfoType {
            bool IsConfigured;
            bool IsHuman;
            int Color;
            int House;
            int Difficulty;
            bool IsObserver;
            int SpawnLocation;
            int Alliances[MAX_PLAYERS];
        };

        bool IsSpawnerSession;
        SpawnerGameOptionsType SpawnerRuntime;
        SpawnerSlotInfoType SlotInfo[MAX_PLAYERS];

        /**
         *  Is the message we're currently writing meant to be sent to allies only?
         */
        bool IsChatToAllies;

        /**
         *  If we're writing a private message, this is the name of its recipient.
         */
        char MessageRecipientName[32];

        /**
         *  Convenient property to access IsGDI as a HousesType.
         */
        HousesType Get_House() const { return static_cast<HousesType>(reinterpret_cast<unsigned char&>(This()->IsGDI)); }
        void Set_House(HousesType house) { reinterpret_cast<unsigned char&>(This()->IsGDI) = house; }
        __declspec(property(get = Get_House, put = Set_House)) HousesType House;
};
