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
#include "tibsun_defines.h"
#include "vinifera_saveload.h"
#include "wwcrc.h"
#include "extension.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
AnimTypeClassExtension::AnimTypeClassExtension(const AnimTypeClass* this_ptr) :
    ObjectTypeClassExtension(this_ptr),
    IsHideIfNotTiberium(false),
    IsForceBigCraters(false),
    ZAdjust(0),
    AttachLayer(LAYER_NONE),
    ParticleToSpawn(PARTICLE_NONE),
    NumberOfParticles(0),
    AreaDamage(0),
    AreaDamageRadius(0),
    AreaDamagePercentAtMaxRange(100),
    AreaDamagePercentAgainstUnits(100),
    AreaDamageSmudgeChance(0),
    AreaDamageFlameChance(0),
    // StartAnims(),
    // StartAnimsCount(),
    // StartAnimsMinimum(),
    // StartAnimsMaximum(),
    // MiddleAnims(),
    // MiddleAnimsCount(),
    // MiddleAnimsMinimum(),
    // MiddleAnimsMaximum(),
    // EndAnims(),
    // EndAnimsCount(),
    // EndAnimsMinimum(),
    // EndAnimsMaximum(),
    BiggestFrameWidth(0),
    BiggestFrameHeight(0)
{
    //if (this_ptr) EXT_DEBUG_TRACE("AnimTypeClassExtension::AnimTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    AnimTypeExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
AnimTypeClassExtension::AnimTypeClassExtension(const NoInitClass &noinit) :
    ObjectTypeClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("AnimTypeClassExtension::AnimTypeClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
AnimTypeClassExtension::~AnimTypeClassExtension()
{
    //EXT_DEBUG_TRACE("AnimTypeClassExtension::~AnimTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    AnimTypeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT AnimTypeClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("AnimTypeClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (lpClassID == nullptr) {
        return E_POINTER;
    }

    *lpClassID = __uuidof(this);

    return S_OK;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT AnimTypeClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("AnimTypeClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = ObjectTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    //StartAnims.Clear();
    //StartAnimsCount.Clear();
    //StartAnimsMinimum.Clear();
    //StartAnimsMaximum.Clear();
    //MiddleAnims.Clear();
    //MiddleAnimsCount.Clear();
    //MiddleAnimsMinimum.Clear();
    //MiddleAnimsMaximum.Clear();
    //EndAnims.Clear();
    //EndAnimsCount.Clear();
    //EndAnimsMinimum.Clear();
    //EndAnimsMaximum.Clear();

    new (this) AnimTypeClassExtension(NoInitClass());
    
    //StartAnims.Load(pStm);
    //StartAnimsCount.Load(pStm);
    //StartAnimsMinimum.Load(pStm);
    //StartAnimsMaximum.Load(pStm);
    //MiddleAnims.Load(pStm);
    //MiddleAnimsCount.Load(pStm);
    //MiddleAnimsMinimum.Load(pStm);
    //MiddleAnimsMaximum.Load(pStm);
    //EndAnims.Load(pStm);
    //EndAnimsCount.Load(pStm);
    //EndAnimsMinimum.Load(pStm);
    //EndAnimsMaximum.Load(pStm);

    //VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP_LIST(StartAnims, "StartAnims");
    //VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP_LIST(MiddleAnims, "MiddleAnims");
    //VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP_LIST(EndAnims, "EndAnims");

    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT AnimTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("AnimTypeClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = ObjectTypeClassExtension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    //StartAnims.Save(pStm);
    //StartAnimsCount.Save(pStm);
    //StartAnimsMinimum.Save(pStm);
    //StartAnimsMaximum.Save(pStm);
    //MiddleAnims.Save(pStm);
    //MiddleAnimsCount.Save(pStm);
    //MiddleAnimsMinimum.Save(pStm);
    //MiddleAnimsMaximum.Save(pStm);
    //EndAnims.Save(pStm);
    //EndAnimsCount.Save(pStm);
    //EndAnimsMinimum.Save(pStm);
    //EndAnimsMaximum.Save(pStm);

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: CCHyper
 */
int AnimTypeClassExtension::Size_Of() const
{
    //EXT_DEBUG_TRACE("AnimTypeClassExtension::Size_Of - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void AnimTypeClassExtension::Detach(TARGET target, bool all)
{
    //EXT_DEBUG_TRACE("AnimTypeClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void AnimTypeClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("AnimTypeClassExtension::Compute_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    crc(AttachLayer);
    crc(NumberOfParticles);
    crc(AreaDamage);
    crc(AreaDamageRadius);
    crc(AreaDamagePercentAtMaxRange);
    crc(AreaDamagePercentAgainstUnits);
    crc(AreaDamageSmudgeChance);
    crc(AreaDamageFlameChance);
    // crc(StartAnims.Count());
    // crc(StartAnimsCount.Count());
    // crc(StartAnimsMinimum.Count());
    // crc(StartAnimsMaximum.Count());
    // crc(MiddleAnims.Count());
    // crc(MiddleAnimsCount.Count());
    // crc(MiddleAnimsMinimum.Count());
    // crc(MiddleAnimsMaximum.Count());
    // crc(EndAnims.Count());
    // crc(EndAnimsCount.Count());
    // crc(EndAnimsMinimum.Count());
    // crc(EndAnimsMaximum.Count());
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool AnimTypeClassExtension::Read_INI(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("AnimTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (!ObjectTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    const char *ini_name = This()->Name();

    if (!ini.Is_Present(ini_name)) {
        return false;
    }

    /**
     *  #issue-756
     * 
     *  Reload the RandomRate value and correctly fix up negative and back to front values.
     */
    TPoint2D<int> random_rate = ini.Get_Point(ini_name, "RandomRate", TPoint2D<int>(-1, -1));
    if (random_rate.X != -1) {
        if (random_rate.Y <= 0) {
            DEV_DEBUG_WARNING("Animation \"%s\" has a zero or negative random rate 'Low' value!\n", This()->Name());
        }
        random_rate.X = TICKS_PER_MINUTE / std::abs(random_rate.X);
    }
    if (random_rate.Y != -1) {
        if (random_rate.Y <= 0) {
            DEV_DEBUG_WARNING("Animation \"%s\" has a zero or negative random rate 'High' value!\n", This()->Name());
        }
        random_rate.Y = TICKS_PER_MINUTE / std::abs(random_rate.Y);
    }
    if ((random_rate.X != -1 && random_rate.Y != -1) && random_rate.X > random_rate.Y) {
        std::swap(random_rate.X, random_rate.Y);
    }
    This()->RandomRateMin = std::clamp(random_rate.X, 0, random_rate.X);
    This()->RandomRateMax = std::clamp(random_rate.Y, 0, random_rate.Y);

    /**
     *  #issue-646
     * 
     *  Some animations in the vanilla game have a DetailLevel of 3, which is out
     *  of range and as a result do not play in-game. This makes sure the values
     *  are never outside of the expected range.
     */
    This()->DetailLevel = std::clamp(This()->DetailLevel, 0, 2);

    IsHideIfNotTiberium = ini.Get_Bool(ini_name, "HideIfNoTiberium", IsHideIfNotTiberium);
    IsForceBigCraters = ini.Get_Bool(ini_name, "ForceBigCraters", IsForceBigCraters);
    ZAdjust = ini.Get_Int(ini_name, "ZAdjust", ZAdjust);
    AttachLayer = ini.Get_LayerType(ini_name, "Layer", AttachLayer);
    ParticleToSpawn = ini.Get_ParticleType(ini_name, "SpawnsParticle", ParticleToSpawn);
    NumberOfParticles = ini.Get_Int(ini_name, "NumParticles", NumberOfParticles);

    /**
     *  #issue-520
     *
     *  Implements "RA1 nuke style" area damage logic.
     * 
     *  @author: Rampastring
     */
    AreaDamage = ini.Get_Int(ini_name, "AreaDamage", AreaDamage);
    AreaDamageRadius = ini.Get_Int(ini_name, "AreaDamageRadius", AreaDamageRadius);
    AreaDamagePercentAtMaxRange = ini.Get_Int(ini_name, "AreaDamagePercentAtMaxRange", AreaDamagePercentAtMaxRange);
    AreaDamagePercentAgainstUnits = ini.Get_Int(ini_name, "AreaDamagePercentAgainstUnits", AreaDamagePercentAgainstUnits);
    AreaDamageSmudgeChance = ini.Get_Int(ini_name, "AreaDamageSmudgeChance", AreaDamageSmudgeChance);
    AreaDamageFlameChance = ini.Get_Int(ini_name, "AreaDamageFlameChance", AreaDamageFlameChance);

    // StartAnims = ini.Get_Anims(ini_name, "StartAnims", StartAnims);
    // StartAnimsCount = ini.Get_Integer_List(ini_name, "StartAnimsCount", StartAnimsCount);
    // StartAnimsMinimum = ini.Get_Integer_List(ini_name, "StartAnimsMinimum", StartAnimsMinimum);
    // StartAnimsMaximum = ini.Get_Integer_List(ini_name, "StartAnimsMaximum", StartAnimsMaximum);

    // if (!StartAnimsCount.Count()) {
    //     ASSERT_FATAL(StartAnims.Count() == StartAnimsMinimum.Count());
    //     ASSERT_FATAL(StartAnims.Count() == StartAnimsMaximum.Count());
    // }

    // MiddleAnims = ini.Get_Anims(ini_name, "MiddleAnims", MiddleAnims);
    // MiddleAnimsCount = ini.Get_Integer_List(ini_name, "MiddleAnimsCount", MiddleAnimsCount);
    // MiddleAnimsMinimum = ini.Get_Integer_List(ini_name, "MiddleAnimsMinimum", MiddleAnimsMinimum);
    // MiddleAnimsMaximum = ini.Get_Integer_List(ini_name, "MiddleAnimsMaximum", MiddleAnimsMaximum);

    // if (!MiddleAnimsCount.Count()) {
    //     ASSERT_FATAL(MiddleAnims.Count() == MiddleAnimsMinimum.Count());
    //     ASSERT_FATAL(MiddleAnims.Count() == MiddleAnimsMaximum.Count());
    // }

    // EndAnims = ini.Get_Anims(ini_name, "EndAnims", EndAnims);
    // EndAnimsCount = ini.Get_Integer_List(ini_name, "EndAnimsCount", EndAnimsCount);
    // EndAnimsMinimum = ini.Get_Integer_List(ini_name, "EndAnimsMinimum", EndAnimsMinimum);
    // EndAnimsMaximum = ini.Get_Integer_List(ini_name, "EndAnimsMaximum", EndAnimsMaximum);

    // if (!EndAnimsCount.Count()) {
    //     ASSERT_FATAL(EndAnims.Count() == EndAnimsMinimum.Count());
    //     ASSERT_FATAL(EndAnims.Count() == EndAnimsMaximum.Count());
    // }

    /**
     *  #issue-883
     *
     *  The "biggest" frame of a animation is frame which should hide all cosmetic
     *  changes to the underlaying ground (e.g. craters) that the animation causes,
     *  so these effects are delayed until this frame is reached. TibSun calculates
     *  this by scanning the entire shape file to find the largest visible frame, but
     *  in some cases, this might not be ideal (e.g. the shape has consistent frame
     *  dimensions). This new value allows the frame in which these effects are
     *  spawned be set.
     *
     *  A special value of "-1" will set the biggest frame to the actual middle frame
     *  of the shape file. This behavior was observed in Red Alert 2.
     */
    if (This()->Image != nullptr && This()->Image->Get_Frame_Count() > 0) {
        ShapeFileStruct* image = const_cast<ShapeFileStruct*>(This()->Image);

        int biggest = ini.Get_Int_Clamp(ini_name, "MiddleFrame", -1, image->Get_Frame_Count() - 1, This()->Biggest);

        if (biggest == -1 && image->Get_Frame_Count() >= 2) {

            This()->Biggest = image->Get_Frame_Count() / 2;
            BiggestFrameWidth = image->Get_Frame_Data(This()->Biggest)->FrameWidth;
            BiggestFrameHeight = image->Get_Frame_Data(This()->Biggest)->FrameHeight;

        }
        else if (biggest != This()->Biggest) {

            This()->Biggest = biggest;
            BiggestFrameWidth = image->Get_Frame_Data(This()->Biggest)->FrameWidth;
            BiggestFrameHeight = image->Get_Frame_Data(This()->Biggest)->FrameHeight;
        }
    }

    return true;
}
