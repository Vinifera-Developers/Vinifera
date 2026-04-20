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

#include "tibsun_globals.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
SessionClassExtension::SessionClassExtension(const SessionClass *this_ptr) :
    GlobalExtensionClass(this_ptr)
{
    //if (this_ptr) EXT_DEBUG_TRACE("SessionClassExtension::SessionClassExtension - 0x%08X\n", (uintptr_t)(ThisPtr));
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
 *  Loads game options from the stream.
 *
 *  @note: The Session extension itself is not saved, only the options are!
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
    return hr;
}


/**
 *  Save game options to the stream.
 *
 *  @note: The Session extension itself is not saved, only the options are!
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
    crc(ExtOptions.MultiplayerAutoSaveInterval);
    crc(ExtOptions.IsQuickMatch);
    crc(ExtOptions.IsWriteStatistics);
    crc(ExtOptions.IsAutoSurrender);
    crc(ExtOptions.IsAttackNeutralUnits);
    crc(ExtOptions.IsCoachMode);
    crc(ExtOptions.IsContinueWithoutHumans);
    crc(ExtOptions.IsScrapMetal);
    crc(ExtOptions.IsAINamesByDifficulty);
}
