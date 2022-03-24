/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ANIMTYPEEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended AnimTypeClass class.
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
#include "animtypeext.h"
#include "animtype.h"
#include "ccini.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Provides the map for all AnimTypeClass extension instances.
 */
ExtensionMap<AnimTypeClass, AnimTypeClassExtension> AnimTypeClassExtensions;


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
AnimTypeClassExtension::AnimTypeClassExtension(AnimTypeClass *this_ptr) :
    Extension(this_ptr),
    IsHideIfNotTiberium(false),
    IsForceBigCraters(false),
    ZAdjust(0),
    AttachLayer(LAYER_NONE),
    ParticleToSpawn(PARTICLE_NONE),
    NumberOfParticles(0),
    StartAnims(),
    StartAnimsCount(),
    StartAnimsMinimum(),
    StartAnimsMaximum(),
    MiddleAnims(),
    MiddleAnimsCount(),
    MiddleAnimsMinimum(),
    MiddleAnimsMaximum(),
    EndAnims(),
    EndAnimsCount(),
    EndAnimsMinimum(),
    EndAnimsMaximum()
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("AnimTypeClassExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("AnimTypeClassExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    IsInitialized = true;
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
AnimTypeClassExtension::AnimTypeClassExtension(const NoInitClass &noinit) :
    Extension(noinit)
{
    IsInitialized = false;
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
AnimTypeClassExtension::~AnimTypeClassExtension()
{
    //EXT_DEBUG_TRACE("AnimTypeClassExtension destructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("AnimTypeClassExtension destructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    IsInitialized = false;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT AnimTypeClassExtension::Load(IStream *pStm)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("AnimTypeClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    HRESULT hr = Extension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    StartAnims.Clear();
    StartAnimsCount.Clear();
    StartAnimsMinimum.Clear();
    StartAnimsMaximum.Clear();
    MiddleAnims.Clear();
    MiddleAnimsCount.Clear();
    MiddleAnimsMinimum.Clear();
    MiddleAnimsMaximum.Clear();
    EndAnims.Clear();
    EndAnimsCount.Clear();
    EndAnimsMinimum.Clear();
    EndAnimsMaximum.Clear();

    new (this) AnimTypeClassExtension(NoInitClass());

    StartAnims.Load(pStm);
    StartAnimsCount.Load(pStm);
    StartAnimsMinimum.Load(pStm);
    StartAnimsMaximum.Load(pStm);
    MiddleAnims.Load(pStm);
    MiddleAnimsCount.Load(pStm);
    MiddleAnimsMinimum.Load(pStm);
    MiddleAnimsMaximum.Load(pStm);
    EndAnims.Load(pStm);
    EndAnimsCount.Load(pStm);
    EndAnimsMinimum.Load(pStm);
    EndAnimsMaximum.Load(pStm);

    for (int i = 0; i < StartAnims.Count(); ++i) {
        SwizzleManager.Swizzle((void **)&StartAnims[i]);
    }

    for (int i = 0; i < MiddleAnims.Count(); ++i) {
        SwizzleManager.Swizzle((void **)&MiddleAnims[i]);
    }

    for (int i = 0; i < EndAnims.Count(); ++i) {
        SwizzleManager.Swizzle((void **)&EndAnims[i]);
    }
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT AnimTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("AnimTypeClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    HRESULT hr = Extension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    StartAnims.Save(pStm);
    StartAnimsCount.Save(pStm);
    StartAnimsMinimum.Save(pStm);
    StartAnimsMaximum.Save(pStm);
    MiddleAnims.Save(pStm);
    MiddleAnimsCount.Save(pStm);
    MiddleAnimsMinimum.Save(pStm);
    MiddleAnimsMaximum.Save(pStm);
    EndAnims.Save(pStm);
    EndAnimsCount.Save(pStm);
    EndAnimsMinimum.Save(pStm);
    EndAnimsMaximum.Save(pStm);

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: CCHyper
 */
int AnimTypeClassExtension::Size_Of() const
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("AnimTypeClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void AnimTypeClassExtension::Detach(TARGET target, bool all)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("AnimTypeClassExtension::Detach - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void AnimTypeClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("AnimTypeClassExtension::Compute_CRC - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    crc(AttachLayer);
    crc(NumberOfParticles);
    crc(StartAnims.Count());
    crc(StartAnimsCount.Count());
    crc(StartAnimsMinimum.Count());
    crc(StartAnimsMaximum.Count());
    crc(MiddleAnims.Count());
    crc(MiddleAnimsCount.Count());
    crc(MiddleAnimsMinimum.Count());
    crc(MiddleAnimsMaximum.Count());
    crc(EndAnims.Count());
    crc(EndAnimsCount.Count());
    crc(EndAnimsMinimum.Count());
    crc(EndAnimsMaximum.Count());
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool AnimTypeClassExtension::Read_INI(CCINIClass &ini)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("AnimTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    EXT_DEBUG_WARNING("AnimTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    const char *ini_name = ThisPtr->Name();

    if (!ini.Is_Present(ini_name)) {
        return false;
    }

    /**
     *  #issue-646
     * 
     *  Some animations in the vanilla game have a DetailLevel of 3, which is out
     *  of range and as a result do not play in-game. This makes sure the values
     *  are never outside of the expected range.
     */
    ThisPtr->DetailLevel = std::clamp(ThisPtr->DetailLevel, 0, 2);

    IsHideIfNotTiberium = ini.Get_Bool(ini_name, "HideIfNoTiberium", IsHideIfNotTiberium);
    IsForceBigCraters = ini.Get_Bool(ini_name, "ForceBigCraters", IsForceBigCraters);
    ZAdjust = ini.Get_Int(ini_name, "ZAdjust", ZAdjust);
    AttachLayer = ini.Get_LayerType(ini_name, "Layer", AttachLayer);
    ParticleToSpawn = ini.Get_ParticleType(ini_name, "SpawnsParticle", ParticleToSpawn);
    NumberOfParticles = ini.Get_Int(ini_name, "NumParticles", NumberOfParticles);
    
    return true;
}


/**
 *  Fetches the extension data from the INI database. This function is to be
 *  called only after the main rules processing has been done.
 *  
 *  @author: CCHyper
 */
bool AnimTypeClassExtension::Post_Read_INI(CCINIClass &ini)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("AnimTypeClassExtension::Post_Read_INI - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    EXT_DEBUG_WARNING("AnimTypeClassExtension::Post_Read_INI - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    const char *ini_name = ThisPtr->Name();

    if (!ini.Is_Present(ini_name)) {
        return false;
    }

    StartAnims = ini.Get_Anims(ini_name, "StartAnims", StartAnims);
    StartAnimsCount = ini.Get_Integer_List(ini_name, "StartAnimsCount", StartAnimsCount);
    StartAnimsMinimum = ini.Get_Integer_List(ini_name, "StartAnimsMinimum", StartAnimsMinimum);
    StartAnimsMaximum = ini.Get_Integer_List(ini_name, "StartAnimsMaximum", StartAnimsMaximum);

    if (!StartAnimsCount.Count()) {
        ASSERT(StartAnims.Count() == StartAnimsMinimum.Count());
        ASSERT(StartAnims.Count() == StartAnimsMaximum.Count());
    }

    MiddleAnims = ini.Get_Anims(ini_name, "MiddleAnims", MiddleAnims);
    MiddleAnimsCount = ini.Get_Integer_List(ini_name, "MiddleAnimsCount", MiddleAnimsCount);
    MiddleAnimsMinimum = ini.Get_Integer_List(ini_name, "MiddleAnimsMinimum", MiddleAnimsMinimum);
    MiddleAnimsMaximum = ini.Get_Integer_List(ini_name, "MiddleAnimsMaximum", MiddleAnimsMaximum);

    if (!MiddleAnimsCount.Count()) {
        ASSERT(MiddleAnims.Count() == MiddleAnimsMinimum.Count());
        ASSERT(MiddleAnims.Count() == MiddleAnimsMaximum.Count());
    }

    EndAnims = ini.Get_Anims(ini_name, "EndAnims", EndAnims);
    EndAnimsCount = ini.Get_Integer_List(ini_name, "EndAnimsCount", EndAnimsCount);
    EndAnimsMinimum = ini.Get_Integer_List(ini_name, "EndAnimsMinimum", EndAnimsMinimum);
    EndAnimsMaximum = ini.Get_Integer_List(ini_name, "EndAnimsMaximum", EndAnimsMaximum);

    if (!EndAnimsCount.Count()) {
        ASSERT(EndAnims.Count() == EndAnimsMinimum.Count());
        ASSERT(EndAnims.Count() == EndAnimsMaximum.Count());
    }
    
    return true;
}
