/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SESSIONEXT.CPP
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

#include "always.h"

#include "sessionext.h"

#include "ccini.h"
#include "noinit.h"
#include "tibsun_globals.h"
#include "vinifera_saveload.h"
#include "voc.h"


namespace
{
    struct QueueAIMPTimings
    {
        int MIXFILE_RESEND_DELTA;
        int FRAMESYNC_DLG_TIME;
        int FRAMESYNC_TIMEOUT;
        int MIXFILE_TIMEOUT;
    };
}


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
SessionClassExtension::SessionClassExtension(const SessionClass *this_ptr) :
    GlobalExtensionClass(this_ptr),
    ExtOptions(),
    IsSpawnerSession(false),
    SpawnerRuntime(),
    IsChatToAllies(false),
    MessageRecipientName("")
{
    //if (this_ptr) EXT_DEBUG_TRACE("SessionClassExtension::SessionClassExtension - 0x%08X\n", (uintptr_t)(ThisPtr));

   /**
     *  Initialises the default game options.
     */
    ExtOptions.IsAutoDeployMCV = false;
    ExtOptions.IsPrePlacedConYards = false;
    ExtOptions.IsBuildOffAlly = true;
    Clear_Spawner_State();
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
SessionClassExtension::SessionClassExtension(const NoInitClass &noinit) :
    GlobalExtensionClass(noinit)
{
    //EXT_DEBUG_TRACE("SessionClassExtension::SessionClassExtension(NoInitClass) - 0x%08X\n", (uintptr_t)(ThisPtr));
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
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT SessionClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("SessionClassExtension::Load - 0x%08X\n", (uintptr_t)(This()));

    HRESULT hr = GlobalExtensionClass::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) SessionClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT SessionClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("SessionClassExtension::Save - 0x%08X\n", (uintptr_t)(This()));

    HRESULT hr = GlobalExtensionClass::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

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
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void SessionClassExtension::Detach(AbstractClass * target, bool all)
{
    //EXT_DEBUG_TRACE("SessionClassExtension::Detach - 0x%08X\n", (uintptr_t)(This()));
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

    crc(IsSpawnerSession);
    crc(SpawnerRuntime.MultiplayerAutoSaveInterval);
    crc(SpawnerRuntime.QuickMatch);
    crc(SpawnerRuntime.WriteStatistics);
    crc(SpawnerRuntime.AutoSurrender);
    crc(SpawnerRuntime.AttackNeutralUnits);
    crc(SpawnerRuntime.CoachMode);
    crc(SpawnerRuntime.ContinueWithoutHumans);
    crc(SpawnerRuntime.ScrapMetal);
    crc(SpawnerRuntime.AINamesByDifficulty);
    crc(SpawnerRuntime.ProtocolZeroEnabled);
    crc(SpawnerRuntime.ProtocolZeroMaxLatencyLevel);
    crc(SpawnerRuntime.ReconnectTimeout);
    crc(SpawnerRuntime.Tournament);
    crc(SpawnerRuntime.GameID);

    for (const SpawnerSlotInfoType& slot_info : SlotInfo) {
        crc(slot_info.IsConfigured);
        crc(slot_info.IsHuman);
        crc(slot_info.Color);
        crc(slot_info.House);
        crc(slot_info.Difficulty);
        crc(slot_info.IsObserver);
        crc(slot_info.SpawnLocation);

        for (int ally_index = 0; ally_index < std::size(slot_info.Alliances); ++ally_index) {
            crc(slot_info.Alliances[ally_index]);
        }
    }

    crc(IsChatToAllies);
    crc(MessageRecipientName);
}


void SessionClassExtension::Clear_Spawner_State()
{
    IsSpawnerSession = false;

    SpawnerRuntime.MultiplayerAutoSaveInterval = 0;
    SpawnerRuntime.QuickMatch = false;
    SpawnerRuntime.WriteStatistics = false;
    SpawnerRuntime.AutoSurrender = false;
    SpawnerRuntime.AttackNeutralUnits = false;
    SpawnerRuntime.CoachMode = false;
    SpawnerRuntime.ContinueWithoutHumans = false;
    SpawnerRuntime.ScrapMetal = false;
    SpawnerRuntime.AINamesByDifficulty = false;
    SpawnerRuntime.ProtocolZeroEnabled = false;
    SpawnerRuntime.ProtocolZeroMaxLatencyLevel = 0xFF;
    SpawnerRuntime.ReconnectTimeout = 0;
    SpawnerRuntime.Tournament = 0;
    SpawnerRuntime.GameID = 0;

    for (SpawnerSlotInfoType& slot_info : SlotInfo) {
        slot_info.IsConfigured = false;
        slot_info.IsHuman = false;
        slot_info.Color = -1;
        slot_info.House = -1;
        slot_info.Difficulty = -1;
        slot_info.IsObserver = false;
        slot_info.SpawnLocation = -1;

        for (int& ally_index : slot_info.Alliances) {
            ally_index = -1;
        }
    }
}


void SessionClassExtension::Apply_Spawner_Runtime_State() const
{
    WestwoodOnline_Tournament = 0;
    WestwoodOnline_GameID = 0;

    if (!IsSpawnerSession) {
        return;
    }

    WestwoodOnline_Tournament = SpawnerRuntime.Tournament;
    WestwoodOnline_GameID = SpawnerRuntime.GameID;

    static QueueAIMPTimings(&Queue_AI_Multiplayer_Timings)[8] = *reinterpret_cast<QueueAIMPTimings(*)[8]>(0x00707F88);
    Queue_AI_Multiplayer_Timings[GAME_IPX].MIXFILE_TIMEOUT = SpawnerRuntime.ReconnectTimeout;
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
void SessionClassExtension::Read_MultiPlayer_Settings()
{
    //EXT_DEBUG_TRACE("SessionClassExtension::Read_MultiPlayer_Settings - 0x%08X\n", (uintptr_t)(This()));
}


/**
 *  Saves the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
void SessionClassExtension::Write_MultiPlayer_Settings()
{
    //EXT_DEBUG_TRACE("SessionClassExtension::Write_MultiPlayer_Settings - 0x%08X\n", (uintptr_t)(This()));
}
