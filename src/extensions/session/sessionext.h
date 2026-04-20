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
        ~SessionClassExtension() override;

        int Get_Object_Size() const override;
        void Detach(AbstractClass * target, bool all = true) override;
        void Object_CRC(CRCEngine &crc) const override;

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

        };

        ExtGameOptionsType ExtOptions;
        bool IsSpawnerSession = false;

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
