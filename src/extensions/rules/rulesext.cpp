/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          RULESEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended RulesClass class.
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
#include "rulesext.h"
#include "ccini.h"
#include "rules.h"
#include "tiberium.h"
#include "weapontype.h"
#include "buildingtype.h"
#include "housetype.h"
#include "side.h"
#include "wstring.h"
#include "wwcrc.h"
#include "noinit.h"
#include "swizzle.h"
#include "vinifera_saveload.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "housetypeext.h"
#include "supertypeext.h"
#include "animtypeext.h"
#include "buildingtypeext.h"
#include "aircrafttypeext.h"
#include "unittypeext.h"
#include "infantrytypeext.h"
#include "weapontypeext.h"
#include "bullettypeext.h"
#include "warheadtypeext.h"
#include "terraintypeext.h"
#include "smudgetypeext.h"
#include "overlaytypeext.h"
#include "particletypeext.h"
#include "particlesystypeext.h"
#include "voxelanimtypeext.h"

#include "extension.h"
#include "extension_globals.h"


RulesClassExtension *RulesExtension = nullptr;


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
RulesClassExtension::RulesClassExtension(const RulesClass *this_ptr) :
    GlobalExtensionClass(this_ptr),
    IsMPAutoDeployMCV(false),
    IsMPPrePlacedConYards(false),
    IsBuildOffAlly(true),
    IsShowSuperWeaponTimers(true)
{
    //if (this_ptr) EXT_DEBUG_TRACE("RulesClassExtension::RulesClassExtension - 0x%08X\n", (uintptr_t)(ThisPtr));
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
RulesClassExtension::RulesClassExtension(const NoInitClass &noinit) :
    GlobalExtensionClass(noinit)
{
    //EXT_DEBUG_TRACE("RulesClassExtension::RulesClassExtension(NoInitClass) - 0x%08X\n", (uintptr_t)(ThisPtr));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
RulesClassExtension::~RulesClassExtension()
{
    //EXT_DEBUG_TRACE("RulesClassExtension::~RulesClassExtension - 0x%08X\n", (uintptr_t)(ThisPtr));
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT RulesClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("RulesClassExtension::Load - 0x%08X\n", (uintptr_t)(This()));

    HRESULT hr = GlobalExtensionClass::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) RulesClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT RulesClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("RulesClassExtension::Save - 0x%08X\n", (uintptr_t)(This()));

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
int RulesClassExtension::Size_Of() const
{
    //EXT_DEBUG_TRACE("RulesClassExtension::Size_Of - 0x%08X\n", (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void RulesClassExtension::Detach(TARGET target, bool all)
{
    //EXT_DEBUG_TRACE("RulesClassExtension::Detach - 0x%08X\n", (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void RulesClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("RulesClassExtension::Compute_CRC - 0x%08X\n", (uintptr_t)(This()));

    crc(IsMPAutoDeployMCV);
    crc(IsMPPrePlacedConYards);
    crc(IsBuildOffAlly);
    crc(IsShowSuperWeaponTimers);
}


/**
 *  Fetch the bulk of the rule data from the control file.
 *  
 *  @author: CCHyper
 */
void RulesClassExtension::Process(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("RulesClassExtension::Process - 0x%08X\n", (uintptr_t)(This()));

    /**
     *  This function replaces the original rules process, so we need to duplicate
     *  the its behaviour here first.
     */

    This()->Colors(ini);
    This()->Houses(ini);
    This()->Sides(ini);
    This()->Overlays(ini);

    /**
     *  #issue-117
     * 
     *  Add reading of Weapons list from RULES.INI. This needs to be done before
     *  all weapon
     * 
     *  @author: CCHyper
     */
    Weapons(ini);

    This()->SuperWeapons(ini);
    This()->Warheads(ini);
    This()->Smudges(ini);
    This()->Terrains(ini);
    This()->Buildings(ini);
    This()->Vehicles(ini);
    This()->Aircraft(ini);
    This()->Infantry(ini);
    This()->Animations(ini);
    This()->VoxelAnims(ini);
    This()->Particles(ini);
    This()->ParticleSystems(ini);
    This()->JumpjetControls(ini);
    This()->MPlayer(ini);
    This()->AI(ini);
    This()->Powerups(ini);
    This()->Land_Types(ini);
    This()->IQ(ini);
    This()->General(ini);

    /**
     *  This is a edge case issue we exposed in the original RULES.INI where the
     *  Nod Radar (NARADR) has "IsNewTheater" set to false, and as a result, the
     *  new theater system ends up making this build show in the wrong drawing
     *  palette. To fix this, just before Read_INI() is called on all the
     *  BuildingTypes (see RulesClass::Objects()), we make sure NARADR has the
     *  default value of "IsNewTheater" set to true.
     */
    for (int index = 0; index < BuildingTypes.Count(); ++index) {
        BuildingTypeClass *btype = BuildingTypes[index];
        if (!strcmpi(btype->Name(), "NARADR")) {
            btype->IsNewTheater = true;
        }
    }

    This()->Objects(ini);
    This()->Difficulty(ini);
    This()->CrateRules(ini);
    This()->CombatDamage(ini);
    This()->AudioVisual(ini);
    This()->SpecialWeapons(ini);
    TiberiumClass::Process(ini);

    /**
     *  Process the rules extension.
     * 
     *  #NOTE: These must be performed last!
     */
    General(ini);
    MPlayer(ini);
    AudioVisual(ini);

    /**
     *  Process the objects (extension classes).
     */
    Objects(ini);

    /**
     *  Run some checks to ensure certain values are as expected.
     */
    Check();

    /**
     *  Fixup various inconsistencies in the original INI files.
     */
    Fixups();
}


/**
 *  Process and initialise rule data from the control file.
 *  
 *  @author: CCHyper
 */
void RulesClassExtension::Initialize(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("RulesClassExtension::Initialize - 0x%08X\n", (uintptr_t)(This()));

}


/**
 *  Fetch all the object characteristic values.
 *  
 *  @author: CCHyper
 */
bool RulesClassExtension::Objects(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("RulesClassExtension::Objects - 0x%08X\n", (uintptr_t)(This()));

    /**
     *  Fetch the game object (extension) values from the rules file.
     */

    DEBUG_INFO("Rules: Processing HouseTypeExtensions (Count: %d)...\n", HouseTypeExtensions.Count());
    for (int index = 0; index < HouseTypeExtensions.Count(); ++index) {
        HouseTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing SuperWeaponTypeExtensions (Count: %d)...\n", SuperWeaponTypeExtensions.Count());
    for (int index = 0; index < SuperWeaponTypeExtensions.Count(); ++index) {
        SuperWeaponTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing AnimTypeExtensions (Count: %d)...\n", AnimTypeExtensions.Count());
    for (int index = 0; index < AnimTypeExtensions.Count(); ++index) {
        AnimTypeExtensions[index]->Read_INI(ArtINI); // Animations are loaded explicitly from the ArtINI.
    }
    
    DEBUG_INFO("Rules: Processing BuildingTypeExtensions (Count: %d)...\n", BuildingTypeExtensions.Count());
    for (int index = 0; index < BuildingTypeExtensions.Count(); ++index) {
        BuildingTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing AircraftTypeExtensions (Count: %d)...\n", AircraftTypeExtensions.Count());
    for (int index = 0; index < AircraftTypeExtensions.Count(); ++index) {
        AircraftTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing UnitTypeExtensions (Count: %d)...\n", UnitTypeExtensions.Count());
    for (int index = 0; index < UnitTypeExtensions.Count(); ++index) {
        UnitTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing InfantryTypeExtensions (Count: %d)...\n", InfantryTypeExtensions.Count());
    for (int index = 0; index < InfantryTypeExtensions.Count(); ++index) {
        InfantryTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing WeaponTypeExtensions (Count: %d)...\n", WeaponTypeExtensions.Count());
    for (int index = 0; index < WeaponTypeExtensions.Count(); ++index) {
        WeaponTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing BulletTypeExtensions (Count: %d)...\n", BulletTypeExtensions.Count());
    for (int index = 0; index < BulletTypeExtensions.Count(); ++index) {
        BulletTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing WarheadTypeExtensions (Count: %d)...\n", WarheadTypeExtensions.Count());
    for (int index = 0; index < WarheadTypeExtensions.Count(); ++index) {
        WarheadTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing TerrainTypeExtensions (Count: %d)...\n", TerrainTypeExtensions.Count());
    for (int index = 0; index < TerrainTypeExtensions.Count(); ++index) {
        TerrainTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing SmudgeTypeExtensions (Count: %d)...\n", SmudgeTypeExtensions.Count());
    for (int index = 0; index < SmudgeTypeExtensions.Count(); ++index) {
        SmudgeTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing OverlayTypeExtensions (Count: %d)...\n", OverlayTypeExtensions.Count());
    for (int index = 0; index < OverlayTypeExtensions.Count(); ++index) {
        OverlayTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing ParticleTypeExtensions (Count: %d)...\n", ParticleTypeExtensions.Count());
    for (int index = 0; index < ParticleTypeExtensions.Count(); ++index) {
        ParticleTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing ParticleSystemTypeExtensions (Count: %d)...\n", ParticleSystemTypeExtensions.Count());
    for (int index = 0; index < ParticleSystemTypeExtensions.Count(); ++index) {
        ParticleSystemTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing VoxelAnimTypeExtensions (Count: %d)...\n", VoxelAnimTypeExtensions.Count());
    for (int index = 0; index < VoxelAnimTypeExtensions.Count(); ++index) {
        VoxelAnimTypeExtensions[index]->Read_INI(ini);
    }

    return true;
}


/**
 *  Process the general main game rules.
 *  
 *  @author: CCHyper
 */
bool RulesClassExtension::General(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("RulesClassExtension::General - 0x%08X\n", (uintptr_t)(This()));

    static char const * const GENERAL = "General";

    if (!ini.Is_Present(GENERAL)) {
        return false;
    }

    /**
     *  #issue-632
     *
     *  "EngineerDamage" was incorrectly loaded with "EngineerCaptureLevel", so
     *  the value the value correctly.
     *
     *  @author: CCHyper
     */
    This()->EngineerDamage = ini.Get_Float(GENERAL, "EngineerDamage", This()->EngineerDamage);

    return true;
}


/**
 *  Process the audio/visual game settings.
 *  
 *  @author: CCHyper
 */
bool RulesClassExtension::AudioVisual(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("RulesClassExtension::General - 0x%08X\n", (uintptr_t)(This()));

    static char const * const AUDIOVISUAL = "AudioVisual";

    if (!ini.Is_Present(AUDIOVISUAL)) {
        return false;
    }

    IsShowSuperWeaponTimers = ini.Get_Bool(AUDIOVISUAL, "ShowSuperWeaponTimers", IsShowSuperWeaponTimers);

    return true;
}


/**
 *  Process the general main game rules.
 *  
 *  @author: CCHyper
 */
bool RulesClassExtension::MPlayer(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("RulesClassExtension::MPlayer - 0x%08X\n", (uintptr_t)(This()));

    static char const * const MPLAYER = "MultiplayerDefaults";

    if (!ini.Is_Present(MPLAYER)) {
        return false;
    }

    IsMPAutoDeployMCV = ini.Get_Bool(MPLAYER, "AutoDeployMCV", IsMPAutoDeployMCV);
    IsMPPrePlacedConYards = ini.Get_Bool(MPLAYER, "PrePlacedConYards", IsMPPrePlacedConYards);
    IsBuildOffAlly = ini.Get_Bool(MPLAYER, "BuildOffAlly", IsBuildOffAlly);

    return true;
}


/**
 *  Fetch all the weapon characteristic values.
 * 
 *  @author: CCHyper
 */
bool RulesClassExtension::Weapons(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("RulesClassExtension::Weapons - 0x%08X\n", (uintptr_t)(This()));

    static const char * const WEAPONS = "Weapons";

    char buf[128];
    const WeaponTypeClass *weapontype;

    int counter = ini.Entry_Count(WEAPONS);
    for (int index = 0; index < counter; ++index) {
        const char *entry = ini.Get_Entry(WEAPONS, index);

        /**
         *  Get a weapon entry.
         */
        if (ini.Get_String(WEAPONS, entry, buf, sizeof(buf))) {

            /**
             *  Find or create a weapon of the name specified.
             */
            weapontype = WeaponTypeClass::Find_Or_Make(buf);
            if (weapontype) {
                DEV_DEBUG_INFO("Rules: Found WeaponType \"%s\".\n", buf);
            } else {
                DEV_DEBUG_WARNING("Rules: Error processing WeaponType \"%s\"!\n", buf);
            }

        }

    }

    return counter > 0;
}


/**
 *  Performs checks on rules data to ensure values are as expected.
 *  
 *  @author: CCHyper
 */
void RulesClassExtension::Check()
{
    ASSERT_PRINT(This()->CreditTicks.Count() == 2, "CreditTicks must contain 2 valid entries!");
}


/**
 *  This function is for fixing up any erroneous rules data in the unmodded Tiberian Sun to
 *  ensure the original game works as expected with any new systems we implement.
 *
 *  @author: CCHyper
 */
void RulesClassExtension::Fixups()
{
    HouseTypeClass *housetype = HouseTypes[HOUSE_NOD];
    if (housetype) {

        /**
         *  #issue-903
         * 
         *  Workaround because NOD has Side=GDI in unmodded Tiberian Sun.
         * 
         *  Match criteria;
         *   - The HouseType's name is "Nod"
         *   - HouseType "Nod" is index 1
         *   - Side is GDI (index 0)
         *   - Side GDI (index 0) name is "GDI"
         *   - Side 1 name is "Nod"
         */
        if (Wstring(housetype->Name()) == Wstring("Nod")
            && housetype->Get_Heap_ID() == HOUSE_NOD
            && housetype->Side == SIDE_GDI
            && Wstring(Sides[housetype->Side]->Name()) == Wstring("GDI")
            && Wstring(Sides[SIDE_NOD]->Name()) == Wstring("Nod")) {

            DEBUG_WARNING("Rules: House \"%s\" (%d) has \"Side=GDI\", changing Side to \"Nod\"!\n",
                housetype->Name(), housetype->Get_Heap_ID());

            /**
             *  We are pretty sure this house is NOD, force the Side to SIDE_NOD.
             */
            housetype->Side = SIDE_NOD;

            DEBUG_WARNING("Rules: Please consider changing House \"%s\" to have \"Side=Nod\"!\n",
                housetype->Name());
        }

        /**
         *  #issue-903
         * 
         *  Workaround because NOD has Prefix=B in unmodded Tiberian Sun.
         * 
         *  Match criteria;
         *   - The HouseType's name is "Nod"
         *   - HouseType "Nod" is index 1
         *   - HouseType "Nod" has Prefix=B
         */
        if (Wstring(housetype->Name()) == Wstring("Nod")
            && housetype->Get_Heap_ID() == HOUSE_NOD
            && housetype->Prefix == 'B') {

            DEBUG_WARNING("Rules: House \"%s\" (%d) has \"Prefix=B\", changing Prefix to \"N\"!\n",
                housetype->Name(), housetype->Get_Heap_ID());

            /**
             *  We are pretty sure this house is NOD, force the Prefix to the 'N' character.
             */
            housetype->Prefix = 'N';

            DEBUG_WARNING("Rules: Please consider changing House \"%s\" to have \"Side=Nod\"!\n",
                housetype->Name());
        }

    }

}
