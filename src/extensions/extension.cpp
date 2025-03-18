/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          EXTENSION.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         The file contains the functions required for the extension system.
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
#include "extension.h"
#include "tibsun_functions.h"
#include "vinifera_saveload.h"
#include "vinifera_util.h"
#include "wstring.h"
#include "vector.h"
#include "tclassfactory.h"
#include "swizzle.h"
#include "tracker.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "aircraft.h"
#include "aircrafttype.h"
#include "anim.h"
#include "animtype.h"
#include "building.h"
#include "buildingtype.h"
#include "buildinglight.h"
#include "bullet.h"
#include "bullettype.h"
#include "campaign.h"
#include "empulse.h"
#include "factory.h"
#include "foggedobject.h"
#include "side.h"
#include "house.h"
#include "housetype.h"
#include "infantry.h"
#include "infantrytype.h"
#include "isotile.h"
#include "isotiletype.h"
#include "lightsource.h"
#include "objecttype.h"
#include "overlay.h"
#include "overlaytype.h"
#include "particle.h"
#include "particletype.h"
#include "particlesys.h"
#include "particlesystype.h"
#include "sidebarext.h"
#include "radarevent.h"
#include "script.h"
#include "scripttype.h"
#include "smudge.h"
#include "smudgetype.h"
#include "super.h"
#include "supertype.h"
#include "taskforce.h"
#include "tag.h"
#include "tagtype.h"
#include "team.h"
#include "teamtype.h"
#include "trigger.h"
#include "triggertype.h"
#include "aitrigtype.h"
#include "terrain.h"
#include "terraintype.h"
#include "tiberium.h"
#include "unit.h"
#include "unittype.h"
#include "voxelanim.h"
#include "voxelanimtype.h"
#include "warheadtype.h"
#include "wave.h"
#include "weapontype.h"
#include "veinholemonster.h"
#include "taction.h"
#include "tevent.h"
#include "tube.h"
#include "waypointpath.h"
#include "alphashape.h"
#include "event.h"
#include "options.h"
#include "session.h"
#include "scenario.h"
#include "wsproto.h"
#include "iomap.h"
#include "layer.h"
#include "logic.h"

#include "aircraftext.h"
#include "aircrafttypeext.h"
#include "animext.h"
#include "animtypeext.h"
#include "buildingext.h"
#include "buildingtypeext.h"
#include "bullettypeext.h"
#include "campaignext.h"
#include "factoryext.h"
#include "sideext.h"
#include "houseext.h"
#include "housetypeext.h"
#include "infantryext.h"
#include "infantrytypeext.h"
#include "isotiletypeext.h"
#include "objecttypeext.h"
#include "overlayext.h"
#include "overlaytypeext.h"
#include "particlesystypeext.h"
#include "particletypeext.h"
#include "smudgeext.h"
#include "smudgetypeext.h"
#include "superext.h"
#include "supertypeext.h"
#include "terrainext.h"
#include "terraintypeext.h"
#include "tiberiumext.h"
#include "unitext.h"
#include "unittypeext.h"
#include "voxelanimtypeext.h"
#include "warheadtypeext.h"
#include "waveext.h"
#include "weapontypeext.h"

#include "rulesext.h"
#include "scenarioext.h"
#include "sessionext.h"
#include "tacticalext.h"

#include "themeext.h"

#include "tspp_gitinfo.h"
#include "vinifera_gitinfo.h"

#include <iostream>

#include "aircrafttracker.h"
#include "armortype.h"
#include "kamikazetracker.h"
#include "rockettype.h"
#include "spawnmanager.h"


extern int Execute_Day;
extern int Execute_Month;
extern int Execute_Year;
extern int Execute_Hour;
extern int Execute_Min;
extern int Execute_Sec;


/**
 *  Use this macro when you want to get or set the extension pointer from the abstract object.
 */
#define ABSTRACT_EXTENSION_POINTER_CAST_MACRO(_abstract)    (*(uintptr_t *)(((unsigned char *)_abstract) + 0x10))

/**
 *  Use this macro when fetching the extension pointer from the abstract object
 *  for pointer remapping purposes.
 */
#define ABSTRACT_EXTENSION_POINTER_REMAP_MACRO(_abstract)    (uintptr_t **)(((unsigned char *)_abstract) + 0x10);


/**
 *  Set the extension pointer to the input abstract object.
 * 
 *  @author: CCHyper
 */
static void Extension_Set_Abstract_Pointer(const AbstractClass *abstract, const AbstractClassExtension *abstract_extension)
{
    ASSERT(abstract != nullptr);
    ASSERT(abstract_extension != nullptr);

    ABSTRACT_EXTENSION_POINTER_CAST_MACRO(abstract) = (uintptr_t)abstract_extension;
}


/**
 *  Get the extension pointer from the input abstract object.
 * 
 *  @author: CCHyper
 */
static AbstractClassExtension *Extension_Get_Abstract_Pointer(const AbstractClass *abstract)
{
    uintptr_t abstract_extension_address = ABSTRACT_EXTENSION_POINTER_CAST_MACRO(abstract);
    AbstractClassExtension *abstract_extension = (AbstractClassExtension *)abstract_extension_address;
    return abstract_extension;
}


/**
 *  Invalidate the extension pointer from the input abstract object.
 * 
 *  @author: CCHyper
 */
static void Extension_Clear_Abstract_Pointer(const AbstractClass *abstract)
{
    ABSTRACT_EXTENSION_POINTER_CAST_MACRO(abstract) = (uintptr_t)0x00000000;
}


/**
 *  Creates and attach an instance of the extension class associated with the abstract class.
 * 
 *  @author: CCHyper
 */
template<class BASE_CLASS, class EXT_CLASS>
static EXT_CLASS * Extension_Make(const BASE_CLASS *abstract_ptr)
{
    const BASE_CLASS *abs_ptr = reinterpret_cast<const BASE_CLASS *>(abstract_ptr);
    EXT_CLASS *ext_ptr = reinterpret_cast<EXT_CLASS *>(Extension_Get_Abstract_Pointer(abs_ptr));
    
    /**
     *  Create an instance of the extension linked to the abstract object.
     */
    ext_ptr = new EXT_CLASS(reinterpret_cast<const BASE_CLASS *>(abs_ptr));
    ASSERT(ext_ptr != nullptr);
    if (!ext_ptr) {

        char buffer[256];
        std::snprintf(buffer, sizeof(buffer), "Extension_Make: Failed to make \"%s\" extension!\n", Extension::Utility::Get_TypeID_Name<BASE_CLASS>().c_str());

        EXT_DEBUG_WARNING(buffer);

        ShowCursor(TRUE);
        MessageBoxA(MainWindow, buffer, "Vinifera", MB_OK|MB_ICONEXCLAMATION);

        //Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create WeaponTypeClassExtension instance!\n");

        return nullptr;
    }

    EXT_DEBUG_INFO("Created \"%s\" extension.\n", Extension::Utility::Get_TypeID_Name<BASE_CLASS>().c_str());

    /**
     *  Assign the extension class instance to the abstract class.
     */
    Extension_Set_Abstract_Pointer(abstract_ptr, ext_ptr);

    return ext_ptr;
}


/**
 *  Destroys the attached extension instance for the abstract class.
 * 
 *  @author: CCHyper
 */
template<class BASE_CLASS, class EXT_CLASS>
static bool Extension_Destroy(const BASE_CLASS *abstract_ptr)
{
    /**
     *  Fetch the extension instance from the abstract object.
     */
    EXT_CLASS *ext_ptr = reinterpret_cast<EXT_CLASS *>(Extension_Get_Abstract_Pointer(abstract_ptr));
    if (!ext_ptr) {
        EXT_DEBUG_WARNING("Extension_Destroy: \"%s\" extension pointer is null!\n", Extension::Utility::Get_TypeID_Name<BASE_CLASS>().c_str());
        return false;
    }
    
    /**
     *  Destroy the attached extension class instance.
     */
    delete ext_ptr;

    EXT_DEBUG_INFO("Destroyed \"%s\" extension.\n", Extension::Utility::Get_TypeID_Name<BASE_CLASS>().c_str());

    /**
     *  Clear the extension pointer for the abstract class.
     */
    Extension_Clear_Abstract_Pointer(abstract_ptr);

    return true;
}


/**
 *  Saves all active objects to the data stream.
 * 
 *  @author: CCHyper
 */
template<class BASE_CLASS, class EXT_CLASS>
static bool Extension_Save(IStream *pStm, const DynamicVectorClass<EXT_CLASS *> &list)
{
    /**
     *  Save the number of instances of this class.
     */
    int count = list.Count();
    HRESULT hr = pStm->Write(&count, sizeof(count), nullptr);
    if (FAILED(hr)) {
        return false;
    }

    if (list.Count() <= 0) {
        DEBUG_INFO("List for \"%s\" has a count of zero, skipping save.\n", Extension::Utility::Get_TypeID_Name<EXT_CLASS>().c_str());
        return true;
    }

    DEBUG_INFO("Saving \"%s\" extensions (Count: %d)\n", Extension::Utility::Get_TypeID_Name<BASE_CLASS>().c_str(), list.Count());

    /**
     *  Save each instance of this class.
     */
    for (int index = 0; index < count; ++index) {

        EXT_CLASS *ptr = list[index];

        /**
         *  Tell the extension class to persist itself into the data stream.
         */
        IPersistStream *lpPS = nullptr;
        hr = ptr->QueryInterface(__uuidof(IPersistStream), (LPVOID *)&lpPS);
        if (FAILED(hr)) {
            DEBUG_ERROR("Extension \"%s\" does not support IPersistStream!\n", Extension::Utility::Get_TypeID_Name<EXT_CLASS>().c_str());
            return false;
        }

        /**
         *  Save the object itself.
         */
        hr = OleSaveToStream(lpPS, pStm);
        if (FAILED(hr)) {
            DEBUG_ERROR("OleSaveToStream failed for extension \"%s\" (Index: %d)!\n", Extension::Utility::Get_TypeID_Name<EXT_CLASS>().c_str(), index);
            return false;
        }

        /**
         *  Release the interface.
         */
        hr = lpPS->Release();
        if (FAILED(hr)) {
            DEBUG_ERROR("Failed to release extension \"%s\" stream!\n", Extension::Utility::Get_TypeID_Name<EXT_CLASS>().c_str());
            return false;
        }

        EXT_CLASS * ext_ptr = reinterpret_cast<EXT_CLASS *>(lpPS);

        if (ext_ptr->Fetch_RTTI() != RTTI_WAVE || ext_ptr->Fetch_RTTI() != RTTI_LIGHT) {
            EXT_DEBUG_INFO("  -> %s\n", ext_ptr->Name());
        }
    }

    return true;
}


/**
 *  Loads all active objects form the data stream.
 * 
 *  @author: CCHyper
 */
template<class BASE_CLASS, class EXT_CLASS>
static bool Extension_Load(IStream *pStm, DynamicVectorClass<EXT_CLASS *> &list)
{
    /**
     *  Read the number of instances of this class.
     */
    int count = 0;
    HRESULT hr = pStm->Read(&count, sizeof(count), nullptr);
    if (FAILED(hr)) {
        return false;
    }

    if (count <= 0) {
        DEBUG_INFO("List for \"%s\" has a count of zero, skipping load.\n", Extension::Utility::Get_TypeID_Name<EXT_CLASS>().c_str());
        return true;
    }

    DEBUG_INFO("Loading \"%s\" extensions (Count: %d)\n", Extension::Utility::Get_TypeID_Name<BASE_CLASS>().c_str(), count);
    
    /**
     *  Read each class instance.
     */
    for (int index = 0; index < count; ++index) {
        
        /**
         *  Load the object.
         */
        IUnknown *spUnk = nullptr;
        hr = OleLoadFromStream(pStm, __uuidof(IUnknown), (LPVOID *)&spUnk);
        if (FAILED(hr)) {
            DEBUG_ERROR("OleLoadFromStream failed for extension \"%s\" (Index: %d)!\n", Extension::Utility::Get_TypeID_Name<EXT_CLASS>().c_str(), index);
            return false;
        }

    }

    return true;
}


/**
 *  Request remap of all the extension pointers from all the active abstract objects.
 * 
 *  @author: CCHyper
 */
template<class BASE_CLASS, class EXT_CLASS>
static bool Extension_Request_Pointer_Remap(const DynamicVectorClass<BASE_CLASS *> &list)
{
    if (!list.Count()) {
        DEBUG_INFO("Requested remap of \"%s\" extension pointers, but the list is empty!\n", Extension::Utility::Get_TypeID_Name<BASE_CLASS>().c_str());
        return true;
    }

    DEBUG_INFO("Requesting remap of \"%s\" extension pointers (Count %d)...\n", Extension::Utility::Get_TypeID_Name<BASE_CLASS>().c_str(), list.Count());

    for (int index = 0; index < list.Count(); ++index) {

        const BASE_CLASS *object = list[index];
        if (object) {

            if (!Extension_Get_Abstract_Pointer(object)) {
                DEV_DEBUG_ERROR("Extension_Request_Pointer_Remap: \"%s\" extension pointer (index %d) for is null!\n", Extension::Utility::Get_TypeID_Name<BASE_CLASS>().c_str(), index);
                continue; //return false;
            }

            Wstring extptr_name;
            extptr_name += Extension::Utility::Get_TypeID_Name(object).c_str();
            extptr_name += "::ExtPtr";

            /**
             *  Inform the swizzle manager that we need to remap the pointer.
             */
            uintptr_t **ext_ptr_addr = ABSTRACT_EXTENSION_POINTER_REMAP_MACRO(object);
            VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(*ext_ptr_addr, extptr_name.Peek_Buffer());

            EXT_DEBUG_INFO("  Requested remap of index %d extension pointer complete.\n", index);
        }
    }

    return true;
}


/**
 *  Detaches the object from the list of active object.
 * 
 *  @author: CCHyper
 */
template<class EXT_CLASS>
static void Extension_Detach_This_From_All(DynamicVectorClass<EXT_CLASS *> &list, AbstractClass * target, bool all)
{
    for (int index = 0; index < list.Count(); ++index) {
        list[index]->Detach(target, all);
    }
}


/**
 *  Internal function that performs the creation of the extension object and
 *  associates it with the abstract object.
 * 
 *  @author: CCHyper
 */
AbstractClassExtension *Extension::Private::Make_Internal(const AbstractClass *abstract)
{
    ASSERT(abstract != nullptr);

    AbstractClassExtension *extptr = nullptr;

    switch (const_cast<AbstractClass *>(abstract)->Fetch_RTTI()) {
        case RTTI_UNIT: { extptr = Extension_Make<UnitClass, UnitClassExtension>(reinterpret_cast<const UnitClass *>(abstract)); break; }
        case RTTI_AIRCRAFT: { extptr = Extension_Make<AircraftClass, AircraftClassExtension>(reinterpret_cast<const AircraftClass *>(abstract)); break; }
        case RTTI_AIRCRAFTTYPE: { extptr = Extension_Make<AircraftTypeClass, AircraftTypeClassExtension>(reinterpret_cast<const AircraftTypeClass *>(abstract)); break; }
        case RTTI_ANIM: { extptr = Extension_Make<AnimClass, AnimClassExtension>(reinterpret_cast<const AnimClass *>(abstract)); break; }
        case RTTI_ANIMTYPE: { extptr = Extension_Make<AnimTypeClass, AnimTypeClassExtension>(reinterpret_cast<const AnimTypeClass *>(abstract)); break; }
        case RTTI_BUILDING: { extptr = Extension_Make<BuildingClass, BuildingClassExtension>(reinterpret_cast<const BuildingClass *>(abstract)); break; }
        case RTTI_BUILDINGTYPE: { extptr = Extension_Make<BuildingTypeClass, BuildingTypeClassExtension>(reinterpret_cast<const BuildingTypeClass *>(abstract)); break; }
        //case RTTI_BULLET: { extptr = Extension_Make<BulletClass, BulletClassExtension>(reinterpret_cast<const BulletClass *>(abstract)); break; } // Not yet implemented
        case RTTI_BULLETTYPE: { extptr = Extension_Make<BulletTypeClass, BulletTypeClassExtension>(reinterpret_cast<const BulletTypeClass *>(abstract)); break; }
        case RTTI_CAMPAIGN: { extptr = Extension_Make<CampaignClass, CampaignClassExtension>(reinterpret_cast<const CampaignClass *>(abstract)); break; }
        //case RTTI_CELL: { extptr = Extension_Make<CellClass, CellClassExtension>(reinterpret_cast<const CellClass *>(abstract)); break; } // Not yet implemented
        case RTTI_FACTORY: { extptr = Extension_Make<FactoryClass, FactoryClassExtension>(reinterpret_cast<const FactoryClass *>(abstract)); break; }
        case RTTI_HOUSE: { extptr = Extension_Make<HouseClass, HouseClassExtension>(reinterpret_cast<const HouseClass *>(abstract)); break; }
        case RTTI_HOUSETYPE: { extptr = Extension_Make<HouseTypeClass, HouseTypeClassExtension>(reinterpret_cast<const HouseTypeClass *>(abstract)); break; }
        case RTTI_INFANTRY: {  extptr = Extension_Make<InfantryClass, InfantryClassExtension>(reinterpret_cast<const InfantryClass *>(abstract)); break; }
        case RTTI_INFANTRYTYPE: {  extptr = Extension_Make<InfantryTypeClass, InfantryTypeClassExtension>(reinterpret_cast<const InfantryTypeClass *>(abstract)); break; }
        //case RTTI_ISOTILE: { extptr = Extension_Make<IsometricTileClass, IsometricTileClassExtension>(reinterpret_cast<const IsometricTileClass *>(abstract)); break; } // Not yet implemented
        case RTTI_ISOTILETYPE: { extptr = Extension_Make<IsometricTileTypeClass, IsometricTileTypeClassExtension>(reinterpret_cast<const IsometricTileTypeClass *>(abstract)); break; }
        //case RTTI_LIGHT: { extptr = Extension_Make<BuildingLightClass, BuildingLightClassExtension>(reinterpret_cast<const BuildingLightClass *>(abstract)); break; } // Not yet implemented
        case RTTI_OVERLAY: { extptr = Extension_Make<OverlayClass, OverlayClassExtension>(reinterpret_cast<const OverlayClass *>(abstract)); break; }
        case RTTI_OVERLAYTYPE: { extptr = Extension_Make<OverlayTypeClass, OverlayTypeClassExtension>(reinterpret_cast<const OverlayTypeClass *>(abstract)); break; }
        //case RTTI_PARTICLE: { extptr = Extension_Make<ParticleClass, ParticleClassExtension>(reinterpret_cast<const ParticleClass *>(abstract)); break; } // Not yet implemented
        case RTTI_PARTICLETYPE: { extptr = Extension_Make<ParticleTypeClass, ParticleTypeClassExtension>(reinterpret_cast<const ParticleTypeClass *>(abstract)); break; }
        //case RTTI_PARTICLESYSTEM: { extptr = Extension_Make<ParticleSystemClass, ParticleSystemExtension>(reinterpret_cast<const ParticleSystem *>(abstract)); break; } // Not yet implemented
        case RTTI_PARTICLESYSTEMTYPE: { extptr = Extension_Make<ParticleSystemTypeClass, ParticleSystemTypeClassExtension>(reinterpret_cast<const ParticleSystemTypeClass *>(abstract)); break; }
        //case RTTI_SCRIPT: { extptr = Extension_Make<ScriptClass, ScriptClassExtension>(reinterpret_cast<const ScriptClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_SCRIPTTYPE: { extptr = Extension_Make<ScriptTypeClass, Extension>(reinterpret_cast<const ScriptTypeClass *>(abstract)); break; } // Not yet implemented
        case RTTI_SIDE: { extptr = Extension_Make<SideClass, SideClassExtension>(reinterpret_cast<const SideClass *>(abstract)); break; }
        case RTTI_SMUDGE: { extptr = Extension_Make<SmudgeClass, SmudgeClassExtension>(reinterpret_cast<const SmudgeClass *>(abstract)); break; }
        case RTTI_SMUDGETYPE: { extptr = Extension_Make<SmudgeTypeClass, SmudgeTypeClassExtension>(reinterpret_cast<const SmudgeTypeClass *>(abstract)); break; }
        case RTTI_SUPERWEAPONTYPE: { extptr = Extension_Make<SuperWeaponTypeClass, SuperWeaponTypeClassExtension>(reinterpret_cast<const SuperWeaponTypeClass *>(abstract)); break; }
        //case RTTI_TASKFORCE: { extptr = Extension_Make<TaskForceClass, TaskForceClassExtension>(reinterpret_cast<const TaskForceClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_TEAM: { extptr = Extension_Make<TeamClass, TeamClassExtension>(reinterpret_cast<const TeamClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_TEAMTYPE: { extptr = Extension_Make<TeamTypeClass, TeamTypeClassExtension>(reinterpret_cast<const TeamTypeClass *>(abstract)); break; } // Not yet implemented
        case RTTI_TERRAIN: { extptr = Extension_Make<TerrainClass, TerrainClassExtension>(reinterpret_cast<const TerrainClass *>(abstract)); break; }
        case RTTI_TERRAINTYPE: { extptr = Extension_Make<TerrainTypeClass, TerrainTypeClassExtension>(reinterpret_cast<const TerrainTypeClass *>(abstract)); break; }
        //case RTTI_TRIGGER: { extptr = Extension_Make<TriggerClass, TriggerClassExtension>(reinterpret_cast<const TriggerClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_TRIGGERTYPE: { extptr = Extension_Make<TriggerTypeClass, TriggerTypeClassExtension>(reinterpret_cast<const TriggerTypeClass *>(abstract)); break; } // Not yet implemented
        case RTTI_UNITTYPE: { extptr = Extension_Make<UnitTypeClass, UnitTypeClassExtension>(reinterpret_cast<const UnitTypeClass *>(abstract)); break; }
        //case RTTI_VOXELANIM: { extptr = Extension_Make<VoxelAnimClass, VoxelAnimClassExtension>(reinterpret_cast<const VoxelAnimClass *>(abstract)); break; } // Not yet implemented
        case RTTI_VOXELANIMTYPE: { extptr = Extension_Make<VoxelAnimTypeClass, VoxelAnimTypeClassExtension>(reinterpret_cast<const VoxelAnimTypeClass *>(abstract)); break; }
        case RTTI_WAVE: { extptr = Extension_Make<WaveClass, WaveClassExtension>(reinterpret_cast<const WaveClass *>(abstract)); break; }
        //case RTTI_TAG: { extptr = Extension_Make<TagClass, TagClassExtension>(reinterpret_cast<const TagClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_TAGTYPE: { extptr = Extension_Make<TagTypeClass, Extension>(reinterpret_cast<const TagTypeClass *>(abstract)); break; } // Not yet implemented
        case RTTI_TIBERIUM: { extptr = Extension_Make<TiberiumClass, TiberiumClassExtension>(reinterpret_cast<const TiberiumClass *>(abstract)); break; }
        //case RTTI_ACTION: { extptr = Extension_Make<TActionClass, TActionClassExtension>(reinterpret_cast<const TActionClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_EVENT: { extptr = Extension_Make<TEventClass, TEventClassExtension>(reinterpret_cast<const TEventClass *>(abstract)); break; } // Not yet implemented
        case RTTI_WEAPONTYPE: { extptr = Extension_Make<WeaponTypeClass, WeaponTypeClassExtension>(reinterpret_cast<const WeaponTypeClass *>(abstract)); break; }
        case RTTI_WARHEADTYPE: { extptr = Extension_Make<WarheadTypeClass, WarheadTypeClassExtension>(reinterpret_cast<const WarheadTypeClass *>(abstract)); break; }
        //case RTTI_WAYPOINT: { extptr = Extension_Make<WaypointClass, WaypointClassExtension>(reinterpret_cast<const WaypointClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_TUBE: { extptr = Extension_Make<TubeClass, TubeClassExtension>(reinterpret_cast<const TubeClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_LIGHTSOURCE: { extptr = Extension_Make<LightSourceClass, LightSourceClassExtension>(reinterpret_cast<const LightSourceClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_EMPULSE: { extptr = Extension_Make<EMPulseClass, EMPulseClassExtension>(reinterpret_cast<const EMPulseClass *>(abstract)); break; } // Not yet implemented
        case RTTI_SUPERWEAPON: { extptr = Extension_Make<SuperClass, SuperClassExtension>(reinterpret_cast<const SuperClass *>(abstract)); break; }
        //case RTTI_AITRIGGER: { extptr = Extension_Make<AITriggerClass, AITriggerClassExtension>(reinterpret_cast<const AITriggerClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_AITRIGGERTYPE: { extptr = Extension_Make<AITriggerTypeClass, AITriggerTypeClassExtension>(reinterpret_cast<const AITriggerTypeClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_NEURON: { extptr = Extension_Make<NeuronClass, NeuronClassExtension>(reinterpret_cast<const NeuronClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_FOGGEDOBJECT: { extptr = Extension_Make<FoggedObjectClass, FoggedObjectClassExtension>(reinterpret_cast<const FoggedObjectClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_ALPHASHAPE: { extptr = Extension_Make<AlphaShapeClass, AlphaShapeClassExtension>(reinterpret_cast<const AlphaShapeClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_VEINHOLEMONSTER: { extptr = Extension_Make<VeinholeMonsterClass, VeinholeMonsterClassExtension>(reinterpret_cast<const VeinholeMonsterClass *>(abstract)); break; } // Not yet implemented
        default: { DEBUG_ERROR("Extension::Make: No extension support for \"%s\" implemented!\n", Name_From_RTTI(abstract->Fetch_RTTI())); break; }
    };

    return extptr;
}


/**
 *  Internal function that performs the destruction of the extension object
 *  associated with the abstract object.
 * 
 *  @author: CCHyper
 */
bool Extension::Private::Destroy_Internal(const AbstractClass *abstract)
{
    ASSERT(abstract != nullptr);
    
    bool removed = false;

    switch (abstract->Fetch_RTTI()) {
        case RTTI_UNIT: { removed = Extension_Destroy<UnitClass, UnitClassExtension>(reinterpret_cast<const UnitClass *>(abstract)); break; }
        case RTTI_AIRCRAFT: { removed = Extension_Destroy<AircraftClass, AircraftClassExtension>(reinterpret_cast<const AircraftClass *>(abstract)); break; }
        case RTTI_AIRCRAFTTYPE: { removed = Extension_Destroy<AircraftTypeClass, AircraftTypeClassExtension>(reinterpret_cast<const AircraftTypeClass *>(abstract)); break; }
        case RTTI_ANIM: { removed = Extension_Destroy<AnimClass, AnimClassExtension>(reinterpret_cast<const AnimClass *>(abstract)); break; }
        case RTTI_ANIMTYPE: { removed = Extension_Destroy<AnimTypeClass, AnimTypeClassExtension>(reinterpret_cast<const AnimTypeClass *>(abstract)); break; }
        case RTTI_BUILDING: { removed = Extension_Destroy<BuildingClass, BuildingClassExtension>(reinterpret_cast<const BuildingClass *>(abstract)); break; }
        case RTTI_BUILDINGTYPE: { removed = Extension_Destroy<BuildingTypeClass, BuildingTypeClassExtension>(reinterpret_cast<const BuildingTypeClass *>(abstract)); break; }
        //case RTTI_BULLET: { removed = Extension_Destroy<BulletClass, BulletClassExtension>(reinterpret_cast<const BulletClass *>(abstract)); break; } // Not yet implemented
        case RTTI_BULLETTYPE: { removed = Extension_Destroy<BulletTypeClass, BulletTypeClassExtension>(reinterpret_cast<const BulletTypeClass *>(abstract)); break; }
        case RTTI_CAMPAIGN: { removed = Extension_Destroy<CampaignClass, CampaignClassExtension>(reinterpret_cast<const CampaignClass *>(abstract)); break; }
        //case RTTI_CELL: { removed = Extension_Destroy<CellClass, CellClassExtension>(reinterpret_cast<const CellClass *>(abstract)); break; } // Not yet implemented
        case RTTI_FACTORY: { removed = Extension_Destroy<FactoryClass, FactoryClassExtension>(reinterpret_cast<const FactoryClass *>(abstract)); break; }
        case RTTI_HOUSE: { removed = Extension_Destroy<HouseClass, HouseClassExtension>(reinterpret_cast<const HouseClass *>(abstract)); break; }
        case RTTI_HOUSETYPE: { removed = Extension_Destroy<HouseTypeClass, HouseTypeClassExtension>(reinterpret_cast<const HouseTypeClass *>(abstract)); break; }
        case RTTI_INFANTRY: {  removed = Extension_Destroy<InfantryClass, InfantryClassExtension>(reinterpret_cast<const InfantryClass *>(abstract)); break; }
        case RTTI_INFANTRYTYPE: {  removed = Extension_Destroy<InfantryTypeClass, InfantryTypeClassExtension>(reinterpret_cast<const InfantryTypeClass *>(abstract)); break; }
        //case RTTI_ISOTILE: { removed = Extension_Destroy<IsometricTileClass, IsometricTileClassExtension>(reinterpret_cast<const IsometricTileClass *>(abstract)); break; } // Not yet implemented
        case RTTI_ISOTILETYPE: { removed = Extension_Destroy<IsometricTileTypeClass, IsometricTileTypeClassExtension>(reinterpret_cast<const IsometricTileTypeClass *>(abstract)); break; }
        //case RTTI_LIGHT: { removed = Extension_Destroy<BuildingLightClass, BuildingLightClassExtension>(reinterpret_cast<const BuildingLightClass *>(abstract)); break; } // Not yet implemented
        case RTTI_OVERLAY: { removed = Extension_Destroy<OverlayClass, OverlayClassExtension>(reinterpret_cast<const OverlayClass *>(abstract)); break; }
        case RTTI_OVERLAYTYPE: { removed = Extension_Destroy<OverlayTypeClass, OverlayTypeClassExtension>(reinterpret_cast<const OverlayTypeClass *>(abstract)); break; }
        //case RTTI_PARTICLE: { removed = Extension_Destroy<ParticleClass, ParticleClassExtension>(reinterpret_cast<const ParticleClass *>(abstract)); break; } // Not yet implemented
        case RTTI_PARTICLETYPE: { removed = Extension_Destroy<ParticleTypeClass, ParticleTypeClassExtension>(reinterpret_cast<const ParticleTypeClass *>(abstract)); break; }
        //case RTTI_PARTICLESYSTEM: { removed = Extension_Destroy<ParticleSystemClass, ParticleSystemExtension>(reinterpret_cast<const ParticleSystem *>(abstract)); break; } // Not yet implemented
        case RTTI_PARTICLESYSTEMTYPE: { removed = Extension_Destroy<ParticleSystemTypeClass, ParticleSystemTypeClassExtension>(reinterpret_cast<const ParticleSystemTypeClass *>(abstract)); break; }
        //case RTTI_SCRIPT: { removed = Extension_Destroy<ScriptClass, ScriptClassExtension>(reinterpret_cast<const ScriptClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_SCRIPTTYPE: { removed = Extension_Destroy<ScriptTypeClass, Extension>(reinterpret_cast<const ScriptTypeClass *>(abstract)); break; } // Not yet implemented
        case RTTI_SIDE: { removed = Extension_Destroy<SideClass, SideClassExtension>(reinterpret_cast<const SideClass *>(abstract)); break; }
        case RTTI_SMUDGE: { removed = Extension_Destroy<SmudgeClass, SmudgeClassExtension>(reinterpret_cast<const SmudgeClass *>(abstract)); break; }
        case RTTI_SMUDGETYPE: { removed = Extension_Destroy<SmudgeTypeClass, SmudgeTypeClassExtension>(reinterpret_cast<const SmudgeTypeClass *>(abstract)); break; }
        case RTTI_SUPERWEAPONTYPE: { removed = Extension_Destroy<SuperWeaponTypeClass, SuperWeaponTypeClassExtension>(reinterpret_cast<const SuperWeaponTypeClass *>(abstract)); break; }
        //case RTTI_TASKFORCE: { removed = Extension_Destroy<TaskForceClass, TaskForceClassExtension>(reinterpret_cast<const TaskForceClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_TEAM: { removed = Extension_Destroy<TeamClass, TeamClassExtension>(reinterpret_cast<const TeamClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_TEAMTYPE: { removed = Extension_Destroy<TeamTypeClass, TeamTypeClassExtension>(reinterpret_cast<const TeamTypeClass *>(abstract)); break; } // Not yet implemented
        case RTTI_TERRAIN: { removed = Extension_Destroy<TerrainClass, TerrainClassExtension>(reinterpret_cast<const TerrainClass *>(abstract)); break; }
        case RTTI_TERRAINTYPE: { removed = Extension_Destroy<TerrainTypeClass, TerrainTypeClassExtension>(reinterpret_cast<const TerrainTypeClass *>(abstract)); break; }
        //case RTTI_TRIGGER: { removed = Extension_Destroy<TriggerClass, TriggerClassExtension>(reinterpret_cast<const TriggerClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_TRIGGERTYPE: { removed = Extension_Destroy<TriggerTypeClass, TriggerTypeClassExtension>(reinterpret_cast<const TriggerTypeClass *>(abstract)); break; } // Not yet implemented
        case RTTI_UNITTYPE: { removed = Extension_Destroy<UnitTypeClass, UnitTypeClassExtension>(reinterpret_cast<const UnitTypeClass *>(abstract)); break; }
        //case RTTI_VOXELANIM: { removed = Extension_Destroy<VoxelAnimClass, VoxelAnimClassExtension>(reinterpret_cast<const VoxelAnimClass *>(abstract)); break; } // Not yet implemented
        case RTTI_VOXELANIMTYPE: { removed = Extension_Destroy<VoxelAnimTypeClass, VoxelAnimTypeClassExtension>(reinterpret_cast<const VoxelAnimTypeClass *>(abstract)); break; }
        case RTTI_WAVE: { removed = Extension_Destroy<WaveClass, WaveClassExtension>(reinterpret_cast<const WaveClass *>(abstract)); break; }
        //case RTTI_TAG: { removed = Extension_Destroy<TagClass, TagClassExtension>(reinterpret_cast<const TagClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_TAGTYPE: { removed = Extension_Destroy<TagTypeClass, Extension>(reinterpret_cast<const TagTypeClass *>(abstract)); break; } // Not yet implemented
        case RTTI_TIBERIUM: { removed = Extension_Destroy<TiberiumClass, TiberiumClassExtension>(reinterpret_cast<const TiberiumClass *>(abstract)); break; }
        //case RTTI_ACTION: { removed = Extension_Destroy<TActionClass, TActionClassExtension>(reinterpret_cast<const TActionClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_EVENT: { removed = Extension_Destroy<TEventClass, TEventClassExtension>(reinterpret_cast<const TEventClass *>(abstract)); break; } // Not yet implemented
        case RTTI_WEAPONTYPE: { removed = Extension_Destroy<WeaponTypeClass, WeaponTypeClassExtension>(reinterpret_cast<const WeaponTypeClass *>(abstract)); break; }
        case RTTI_WARHEADTYPE: { removed = Extension_Destroy<WarheadTypeClass, WarheadTypeClassExtension>(reinterpret_cast<const WarheadTypeClass *>(abstract)); break; }
        //case RTTI_WAYPOINT: { removed = Extension_Destroy<WaypointClass, WaypointClassExtension>(reinterpret_cast<const WaypointClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_TUBE: { removed = Extension_Destroy<TubeClass, TubeClassExtension>(reinterpret_cast<const TubeClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_LIGHTSOURCE: { removed = Extension_Destroy<LightSourceClass, LightSourceClassExtension>(reinterpret_cast<const LightSourceClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_EMPULSE: { removed = Extension_Destroy<EMPulseClass, EMPulseClassExtension>(reinterpret_cast<const EMPulseClass *>(abstract)); break; } // Not yet implemented
        case RTTI_SUPERWEAPON: { removed = Extension_Destroy<SuperClass, SuperClassExtension>(reinterpret_cast<const SuperClass *>(abstract)); break; }
        //case RTTI_AITRIGGER: { removed = Extension_Destroy<AITriggerClass, AITriggerClassExtension>(reinterpret_cast<const AITriggerClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_AITRIGGERTYPE: { removed = Extension_Destroy<AITriggerTypeClass, AITriggerTypeClassExtension>(reinterpret_cast<const AITriggerTypeClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_NEURON: { removed = Extension_Destroy<NeuronClass, NeuronClassExtension>(reinterpret_cast<const NeuronClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_FOGGEDOBJECT: { removed = Extension_Destroy<FoggedObjectClass, FoggedObjectClassExtension>(reinterpret_cast<const FoggedObjectClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_ALPHASHAPE: { removed = Extension_Destroy<AlphaShapeClass, AlphaShapeClassExtension>(reinterpret_cast<const AlphaShapeClass *>(abstract)); break; } // Not yet implemented
        //case RTTI_VEINHOLEMONSTER: { removed = Extension_Destroy<VeinholeMonsterClass, VeinholeMonsterClassExtension>(reinterpret_cast<const VeinholeMonsterClass *>(abstract)); break; } // Not yet implemented
        default: { DEBUG_ERROR("Extension::Destroy: No extension support for \"%s\" implemented!\n", Name_From_RTTI(abstract->Fetch_RTTI())); break; }
    };

    ASSERT(removed);

    return true;
}


/**
 *  Internal function for fetching the extension object associated with the abstract object.
 * 
 *  @author: CCHyper
 */
AbstractClassExtension *Extension::Private::Fetch_Internal(const AbstractClass *abstract)
{
    ASSERT(abstract != nullptr);

    AbstractClassExtension *ext_ptr = Extension_Get_Abstract_Pointer(abstract);

    if (!ext_ptr) {
        DEBUG_ERROR("Extension::Fetch: Extension for \"%s\" is null!\n", Extension::Utility::Get_TypeID_Name(abstract).c_str());
        return nullptr;
    }

    /**
     *  Its possible the pointer could be invalid, so perform a check.
     */
    if (ext_ptr->Fetch_RTTI() <= RTTI_NONE || ext_ptr->Fetch_RTTI() >= RTTI_COUNT) {
        DEBUG_ERROR("Extension::Fetch: Invalid extension RTTI type for \"%s\"!\n", Extension::Utility::Get_TypeID_Name(abstract).c_str());
        return nullptr;
    }

    //EXT_DEBUG_INFO("Extension::Fetch: Abstract \"%s\", got extension \"%s\".\n", Extension::Utility::Get_TypeID_Name(abstract).c_str(), ext_ptr->Name());

    return ext_ptr;
}


/**
 *  Save all the extension class data to the stream.
 * 
 *  @author: CCHyper
 */
bool Extension::Save(IStream *pStm)
{
    ASSERT(pStm != nullptr);

    if (!pStm) {
        return false;
    }

    DEV_DEBUG_INFO("Extension::Save(enter)\n");
    
    /**
     *  #NOTE: The order of these calls must match the relevant RTTIType order!
     */
    if (!Extension_Save<UnitClass, UnitClassExtension>(pStm, UnitExtensions)) { return false; }
    if (!Extension_Save<AircraftClass, AircraftClassExtension>(pStm, AircraftExtensions)) { return false; }
    if (!Extension_Save<AircraftTypeClass, AircraftTypeClassExtension>(pStm, AircraftTypeExtensions)) { return false; }
    if (!Extension_Save<AnimClass, AnimClassExtension>(pStm, AnimExtensions)) { return false; }
    if (!Extension_Save<AnimTypeClass, AnimTypeClassExtension>(pStm, AnimTypeExtensions)) { return false; }
    if (!Extension_Save<BuildingClass, BuildingClassExtension>(pStm, BuildingExtensions)) { return false; }
    if (!Extension_Save<BuildingTypeClass, BuildingTypeClassExtension>(pStm, BuildingTypeExtensions)) { return false; }
    //if (!Extension_Save<BulletClass, BulletClassExtension>(pStm, BulletExtensions)) { return false; }                 // Not yet implemented
    if (!Extension_Save<BulletTypeClass, BulletTypeClassExtension>(pStm, BulletTypeExtensions)) { return false; }
    //if (!Extension_Save<CampaignClass, CampaignClassExtension>(pStm, CampaignExtensions)) { return false; }           // Supported, but Campaign's are not saved to file.
    //if (!Extension_Save<CellClass, CellClassExtension>(pStm, CellExtensions)) { return false; }                       // Not yet implemented
    if (!Extension_Save<FactoryClass, FactoryClassExtension>(pStm, FactoryExtensions)) { return false; }
    if (!Extension_Save<HouseClass, HouseClassExtension>(pStm, HouseExtensions)) { return false; }
    if (!Extension_Save<HouseTypeClass, HouseTypeClassExtension>(pStm, HouseTypeExtensions)) { return false; }
    if (!Extension_Save<InfantryClass, InfantryClassExtension>(pStm, InfantryExtensions)) { return false; }
    if (!Extension_Save<InfantryTypeClass, InfantryTypeClassExtension>(pStm, InfantryTypeExtensions)) { return false; }
    //if (!Extension_Save<IsometricTileClass, IsometricTileClassExtension>(pStm, IsometricTileExtensions)) { return false; } // Not yet implemented
    //if (!Extension_Save<IsometricTileTypeClass, IsometricTileTypeClassExtension>(pStm, IsometricTileTypeExtensions)) { return false; } // Supported, but IsoTileTypes's are not saved to file.
    //if (!Extension_Save<BuildingLightClass, BuildingLightClassExtension>(pStm, BuildingLightExtensions)) { return false; } // Not yet implemented
    if (!Extension_Save<OverlayClass, OverlayClassExtension>(pStm, OverlayExtensions)) { return false; }
    if (!Extension_Save<OverlayTypeClass, OverlayTypeClassExtension>(pStm, OverlayTypeExtensions)) { return false; }
    //if (!Extension_Save<ParticleClass, ParticleClassExtension>(pStm, ParticleClassExtensions)) { return false; }      // Not yet implemented
    if (!Extension_Save<ParticleTypeClass, ParticleTypeClassExtension>(pStm, ParticleTypeExtensions)) { return false; }
    //if (!Extension_Save<ParticleSystemClass, ParticleSystemClassExtension>(pStm, ParticleSystemExtensions)) { return false; } // Not yet implemented
    if (!Extension_Save<ParticleSystemTypeClass, ParticleSystemTypeClassExtension>(pStm, ParticleSystemTypeExtensions)) { return false; }
    //if (!Extension_Save<ScriptClass, ScriptClassExtension>(pStm, ScriptExtensions)) { return false; }                 // Not yet implemented
    //if (!Extension_Save<ScriptTypeClass, ScriptTypeClassExtension>(pStm, ScriptTypeExtensions)) { return false; }     // Not yet implemented
    if (!Extension_Save<SideClass, SideClassExtension>(pStm, SideExtensions)) { return false; }
    if (!Extension_Save<SmudgeClass, SmudgeClassExtension>(pStm, SmudgeExtensions)) { return false; }
    if (!Extension_Save<SmudgeTypeClass, SmudgeTypeClassExtension>(pStm, SmudgeTypeExtensions)) { return false; }
    if (!Extension_Save<SuperWeaponTypeClass, SuperWeaponTypeClassExtension>(pStm, SuperWeaponTypeExtensions)) { return false; }
    //if (!Extension_Save<TaskForceClass, TaskForceClassExtension>(pStm, TaskForceExtensions)) { return false; }        // Not yet implemented
    //if (!Extension_Save<TeamClass, TeamClassExtension>(pStm, TeamExtensions)) { return false; }                       // Not yet implemented
    //if (!Extension_Save<TeamTypeClass, TeamTypeClassExtension>(pStm, TeamTypeExtensions)) { return false; }           // Not yet implemented
    if (!Extension_Save<TerrainClass, TerrainClassExtension>(pStm, TerrainExtensions)) { return false; }
    if (!Extension_Save<TerrainTypeClass, TerrainTypeClassExtension>(pStm, TerrainTypeExtensions)) { return false; }
    //if (!Extension_Save<TriggerClass, TriggerClassExtension>(pStm, TriggerExtensions)) { return false; }              // Not yet implemented
    //if (!Extension_Save<TriggerTypeClass, TriggerTypeClassExtension>(pStm, TriggerTypeExtensions)) { return false; }  // Not yet implemented
    if (!Extension_Save<UnitTypeClass, UnitTypeClassExtension>(pStm, UnitTypeExtensions)) { return false; }
    //if (!Extension_Save<VoxelAnimClass, VoxelAnimClassExtension>(pStm, VoxelAnimExtensions)) { return false; }        // Not yet implemented
    if (!Extension_Save<VoxelAnimTypeClass, VoxelAnimTypeClassExtension>(pStm, VoxelAnimTypeExtensions)) { return false; }
    if (!Extension_Save<WaveClass, WaveClassExtension>(pStm, WaveExtensions)) { return false; }
    //if (!Extension_Save<TagClass, TagClassExtension>(pStm, TagExtensions)) { return false; }                          // Not yet implemented
    //if (!Extension_Save<TagTypeClass, TagTypeClassExtension>(pStm, TagTypeExtensions)) { return false; }              // Not yet implemented
    if (!Extension_Save<TiberiumClass, TiberiumClassExtension>(pStm, TiberiumExtensions)) { return false; }
    //if (!Extension_Save<TActionClass, TActionClassExtension>(pStm, TActionExtensions)) { return false; }              // Not yet implemented
    //if (!Extension_Save<TEventClass, TEventClassExtension>(pStm, TEventExtensions)) { return false; }                 // Not yet implemented
    if (!Extension_Save<WeaponTypeClass, WeaponTypeClassExtension>(pStm, WeaponTypeExtensions)) { return false; }
    if (!Extension_Save<WarheadTypeClass, WarheadTypeClassExtension>(pStm, WarheadTypeExtensions)) { return false; }
    //if (!Extension_Save<WaypointClass, WaypointClassExtension>(pStm, WaypointExtensions)) { return false; }           // Not yet implemented
    //if (!Extension_Save<TubeClass, TubeClassExtension>(pStm, TubeExtensions)) { return false; }                       // Not yet implemented
    //if (!Extension_Save<LightSourceClass, LightSourceClassExtension>(pStm, LightSourceExtensions)) { return false; }  // Not yet implemented
    //if (!Extension_Save<EMPulseClass, EMPulseClassExtension>(pStm, EMPulseExtensions)) { return false; }              // Not yet implemented
    if (!Extension_Save<SuperClass, SuperClassExtension>(pStm, SuperExtensions)) { return false; }
    //if (!Extension_Save<AITriggerClass, AITriggerClassExtension>(pStm, AITriggerExtensions)) { return false; }        // Not yet implemented
    //if (!Extension_Save<AITriggerTypeClass, AITriggerTypeClassExtension>(pStm, AITriggerExtensions)) { return false; } // Not yet implemented
    //if (!Extension_Save<NeuronClass, NeuronClassExtension>(pStm, NeuronExtensions)) { return false; }                 // Not yet implemented
    //if (!Extension_Save<FoggedObjectClass, FoggedObjectClassExtension>(pStm, FoggedObjectExtensions)) { return false; } // Not yet implemented
    //if (!Extension_Save<AlphaShapeClass, AlphaShapeClassExtension>(pStm, AlphaShapeExtensions)) { return false; }     // Not yet implemented
    //if (!Extension_Save<VeinholeMonsterClass, VeinholeMonsterClassExtension>(pStm, VeinholeMonsterExtensions)) { return false; } // Not yet implemented

    if (FAILED(TacticalMapExtension->Save(pStm, true))) { return false; }
    DEBUG_INFO("Saved \"%s\" extension.\n", TacticalMapExtension->Name());

    if (FAILED(RuleExtension->Save(pStm, true))) { return false; }
    DEBUG_INFO("Saved \"%s\" extension\n", RuleExtension->Name());

    if (FAILED(ScenExtension->Save(pStm, true))) { return false; }
    DEBUG_INFO("Saved \"%s\" extension\n", ScenExtension->Name());

    if (FAILED(SidebarExtension->Save(pStm, true))) { return false; }
    DEBUG_INFO("Saved \"%s\" extension\n", SidebarExtension->Name());

    if (FAILED(SessionExtension->Save(pStm, true))) { return false; }
    DEBUG_INFO("Saved \"%s\" extension\n", SessionExtension->Name());

    DEV_DEBUG_INFO("Extension::Save(exit)\n");

    return true;
}


/**
 *  Load all the extension class data from the stream.
 * 
 *  @author: CCHyper
 */
bool Extension::Load(IStream *pStm)
{
    ASSERT(pStm != nullptr);

    if (!pStm) {
        return false;
    }

    DEV_DEBUG_INFO("Extension::Load(enter)\n");

    /**
     *  #NOTE: The order of these calls must match the relevant RTTIType order!
     */
    if (!Extension_Load<UnitClass, UnitClassExtension>(pStm, UnitExtensions)) { return false; }
    if (!Extension_Load<AircraftClass, AircraftClassExtension>(pStm, AircraftExtensions)) { return false; }
    if (!Extension_Load<AircraftTypeClass, AircraftTypeClassExtension>(pStm, AircraftTypeExtensions)) { return false; }
    if (!Extension_Load<AnimClass, AnimClassExtension>(pStm, AnimExtensions)) { return false; }
    if (!Extension_Load<AnimTypeClass, AnimTypeClassExtension>(pStm, AnimTypeExtensions)) { return false; }
    if (!Extension_Load<BuildingClass, BuildingClassExtension>(pStm, BuildingExtensions)) { return false; }
    if (!Extension_Load<BuildingTypeClass, BuildingTypeClassExtension>(pStm, BuildingTypeExtensions)) { return false; }
    //if (!Extension_Load<BulletClass, BulletClassExtension>(pStm, BulletExtensions)) { return false; }                 // Not yet implemented
    if (!Extension_Load<BulletTypeClass, BulletTypeClassExtension>(pStm, BulletTypeExtensions)) { return false; }
    //if (!Extension_Load<CampaignClass, CampaignClassExtension>(pStm, CampaignExtensions)) { return false; }           // Supported, but Campaign's are not saved to file.
    //if (!Extension_Load<CellClass, CellClassExtension>(pStm, CellExtensions)) { return false; }                       // Not yet implemented
    if (!Extension_Load<FactoryClass, FactoryClassExtension>(pStm, FactoryExtensions)) { return false; }
    if (!Extension_Load<HouseClass, HouseClassExtension>(pStm, HouseExtensions)) { return false; }
    if (!Extension_Load<HouseTypeClass, HouseTypeClassExtension>(pStm, HouseTypeExtensions)) { return false; }
    if (!Extension_Load<InfantryClass, InfantryClassExtension>(pStm, InfantryExtensions)) { return false; }
    if (!Extension_Load<InfantryTypeClass, InfantryTypeClassExtension>(pStm, InfantryTypeExtensions)) { return false; }
    //if (!Extension_Load<IsometricTileClass, IsometricTileClassExtension>(pStm, IsometricTileExtensions)) { return false; } // Not yet implemented
    //if (!Extension_Load<IsometricTileTypeClass, IsometricTileTypeClassExtension>(pStm, IsometricTileTypeExtensions)) { return false; } // Supported, but IsoTileTypes's are not saved to file.
    //if (!Extension_Load<BuildingLightClass, BuildingLightClassExtension>(pStm, BuildingLightExtensions)) { return false; } // Not yet implemented
    if (!Extension_Load<OverlayClass, OverlayClassExtension>(pStm, OverlayExtensions)) { return false; }
    if (!Extension_Load<OverlayTypeClass, OverlayTypeClassExtension>(pStm, OverlayTypeExtensions)) { return false; }
    //if (!Extension_Load<ParticleClass, ParticleClassExtension>(pStm, ParticleClassExtensions)) { return false; }      // Not yet implemented
    if (!Extension_Load<ParticleTypeClass, ParticleTypeClassExtension>(pStm, ParticleTypeExtensions)) { return false; }
    //if (!Extension_Load<ParticleSystemClass, ParticleSystemClassExtension>(pStm, ParticleSystemExtensions)) { return false; } // Not yet implemented
    if (!Extension_Load<ParticleSystemTypeClass, ParticleSystemTypeClassExtension>(pStm, ParticleSystemTypeExtensions)) { return false; }
    //if (!Extension_Load<ScriptClass, ScriptClassExtension>(pStm, ScriptExtensions)) { return false; }                 // Not yet implemented
    //if (!Extension_Load<ScriptTypeClass, ScriptTypeClassExtension>(pStm, ScriptTypeExtensions)) { return false; }     // Not yet implemented
    if (!Extension_Load<SideClass, SideClassExtension>(pStm, SideExtensions)) { return false; }
    if (!Extension_Load<SmudgeClass, SmudgeClassExtension>(pStm, SmudgeExtensions)) { return false; }
    if (!Extension_Load<SmudgeTypeClass, SmudgeTypeClassExtension>(pStm, SmudgeTypeExtensions)) { return false; }
    if (!Extension_Load<SuperWeaponTypeClass, SuperWeaponTypeClassExtension>(pStm, SuperWeaponTypeExtensions)) { return false; }
    //if (!Extension_Load<TaskForceClass, TaskForceClassExtension>(pStm, TaskForceExtensions)) { return false; }        // Not yet implemented
    //if (!Extension_Load<TeamClass, TeamClassExtension>(pStm, TeamExtensions)) { return false; }                       // Not yet implemented
    //if (!Extension_Load<TeamTypeClass, TeamTypeClassExtension>(pStm, TeamTypeExtensions)) { return false; }           // Not yet implemented
    if (!Extension_Load<TerrainClass, TerrainClassExtension>(pStm, TerrainExtensions)) { return false; }
    if (!Extension_Load<TerrainTypeClass, TerrainTypeClassExtension>(pStm, TerrainTypeExtensions)) { return false; }
    //if (!Extension_Load<TriggerClass, TriggerClassExtension>(pStm, TriggerExtensions)) { return false; }              // Not yet implemented
    //if (!Extension_Load<TriggerTypeClass, TriggerTypeClassExtension>(pStm, TriggerTypeExtensions)) { return false; }  // Not yet implemented
    if (!Extension_Load<UnitTypeClass, UnitTypeClassExtension>(pStm, UnitTypeExtensions)) { return false; }
    //if (!Extension_Load<VoxelAnimClass, VoxelAnimClassExtension>(pStm, VoxelAnimExtensions)) { return false; }        // Not yet implemented
    if (!Extension_Load<VoxelAnimTypeClass, VoxelAnimTypeClassExtension>(pStm, VoxelAnimTypeExtensions)) { return false; }
    if (!Extension_Load<WaveClass, WaveClassExtension>(pStm, WaveExtensions)) { return false; }
    //if (!Extension_Load<TagClass, TagClassExtension>(pStm, TagExtensions)) { return false; }                          // Not yet implemented
    //if (!Extension_Load<TagTypeClass, TagTypeClassExtension>(pStm, TagTypeExtensions)) { return false; }              // Not yet implemented
    if (!Extension_Load<TiberiumClass, TiberiumClassExtension>(pStm, TiberiumExtensions)) { return false; }
    //if (!Extension_Load<TActionClass, TActionClassExtension>(pStm, TActionExtensions)) { return false; }              // Not yet implemented
    //if (!Extension_Load<TEventClass, TEventClassExtension>(pStm, TEventExtensions)) { return false; }                 // Not yet implemented
    if (!Extension_Load<WeaponTypeClass, WeaponTypeClassExtension>(pStm, WeaponTypeExtensions)) { return false; }
    if (!Extension_Load<WarheadTypeClass, WarheadTypeClassExtension>(pStm, WarheadTypeExtensions)) { return false; }
    //if (!Extension_Load<WaypointClass, WaypointClassExtension>(pStm, WaypointExtensions)) { return false; }           // Not yet implemented
    //if (!Extension_Load<TubeClass, TubeClassExtension>(pStm, TubeExtensions)) { return false; }                       // Not yet implemented
    //if (!Extension_Load<LightSourceClass, LightSourceClassExtension>(pStm, LightSourceExtensions)) { return false; }  // Not yet implemented
    //if (!Extension_Load<EMPulseClass, EMPulseClassExtension>(pStm, EMPulseExtensions)) { return false; }              // Not yet implemented
    if (!Extension_Load<SuperClass, SuperClassExtension>(pStm, SuperExtensions)) { return false; }
    //if (!Extension_Load<AITriggerClass, AITriggerClassExtension>(pStm, AITriggerExtensions)) { return false; }        // Not yet implemented
    //if (!Extension_Load<AITriggerTypeClass, AITriggerTypeClassExtension>(pStm, AITriggerExtensions)) { return false; } // Not yet implemented
    //if (!Extension_Load<NeuronClass, NeuronClassExtension>(pStm, NeuronExtensions)) { return false; }                 // Not yet implemented
    //if (!Extension_Load<FoggedObjectClass, FoggedObjectClassExtension>(pStm, FoggedObjectExtensions)) { return false; } // Not yet implemented
    //if (!Extension_Load<AlphaShapeClass, AlphaShapeClassExtension>(pStm, AlphaShapeExtensions)) { return false; }     // Not yet implemented
    //if (!Extension_Load<VeinholeMonsterClass, VeinholeMonsterClassExtension>(pStm, VeinholeMonsterExtensions)) { return false; } // Not yet implemented

    if (FAILED(TacticalMapExtension->Load(pStm))) { return false; }
    DEBUG_INFO("Loaded \"%s\" extension.\n", TacticalMapExtension->Name());
    TacticalMapExtension->Assign_This(TacticalMap);

    if (FAILED(RuleExtension->Load(pStm))) { return false; }
    DEBUG_INFO("Loaded \"%s\" extension.\n", RuleExtension->Name());
    RuleExtension->Assign_This(Rule);

    if (FAILED(ScenExtension->Load(pStm))) { return false; }
    DEBUG_INFO("Loaded \"%s\" extension.\n", ScenExtension->Name());
    ScenExtension->Assign_This(Scen);

    if (FAILED(SidebarExtension->Load(pStm))) { return false; }
    DEBUG_INFO("Loaded \"%s\" extension.\n", SidebarExtension->Name());
    SidebarExtension->Assign_This(&Map);

    if (FAILED(SessionExtension->Load(pStm))) { return false; }
    DEBUG_INFO("Loaded \"%s\" extension.\n", SessionExtension->Name());
    SessionExtension->Assign_This(&Session);

    /**
     *  Now we have sucessfully loaded the class data, request the remapping
     *  of all the abstract extension pointers.
     */
    if (!Extension::Request_Pointer_Remap()) { return false; }

    DEV_DEBUG_INFO("Extension::Load(exit)\n");

    return true;
}


/**
 *  Request pointer remap on all extension pointers.
 * 
 *  @author: CCHyper
 */
bool Extension::Request_Pointer_Remap()
{
    DEBUG_INFO("Extension::Request_Pointer_Remap(enter)\n");

    /**
     *  #NOTE: The order of these calls must match the relevant RTTIType order!
     */
    if (!Extension_Request_Pointer_Remap<UnitClass, UnitClassExtension>(Units)) { return false; }
    if (!Extension_Request_Pointer_Remap<AircraftClass, AircraftClassExtension>(Aircrafts)) { return false; }
    if (!Extension_Request_Pointer_Remap<AircraftTypeClass, AircraftTypeClassExtension>(AircraftTypes)) { return false; }
    if (!Extension_Request_Pointer_Remap<AnimClass, AnimClassExtension>(Anims)) { return false; }
    if (!Extension_Request_Pointer_Remap<AnimTypeClass, AnimTypeClassExtension>(AnimTypes)) { return false; }
    if (!Extension_Request_Pointer_Remap<BuildingClass, BuildingClassExtension>(Buildings)) { return false; }
    if (!Extension_Request_Pointer_Remap<BuildingTypeClass, BuildingTypeClassExtension>(BuildingTypes)) { return false; }
    //if (!Extension_Request_Pointer_Remap<BulletClass, BulletClassExtension>(Bullets)) { return false; }               // Not yet implemented
    if (!Extension_Request_Pointer_Remap<BulletTypeClass, BulletTypeClassExtension>(BulletTypes)) { return false; }
    //if (!Extension_Request_Pointer_Remap<CampaignClass, CampaignClassExtension>(Campaigns)) { return false; }         // Does not need to be processed for pointer remapping.
    //if (!Extension_Request_Pointer_Remap<CellClass, CellClassExtension>()) { return false; }                          // Not yet implemented
    if (!Extension_Request_Pointer_Remap<FactoryClass, FactoryClassExtension>(Factories)) { return false; }
    if (!Extension_Request_Pointer_Remap<HouseClass, HouseClassExtension>(Houses)) { return false; }
    if (!Extension_Request_Pointer_Remap<HouseTypeClass, HouseTypeClassExtension>(HouseTypes)) { return false; }
    if (!Extension_Request_Pointer_Remap<InfantryClass, InfantryClassExtension>(Infantry)) { return false; }
    if (!Extension_Request_Pointer_Remap<InfantryTypeClass, InfantryTypeClassExtension>(InfantryTypes)) { return false; }
    //if (!Extension_Request_Pointer_Remap<IsometricTileClass, IsometricTileClassExtension>(IsoTiles)) { return false; } // Not yet implemented
    //if (!Extension_Request_Pointer_Remap<IsometricTileTypeClass, IsometricTileTypeClassExtension>(IsoTileTypes)) { return false; } // Does not need to be processed.
    //if (!Extension_Request_Pointer_Remap<BuildingLightClass, BuildingLightClassExtension>(BuildingLights)) { return false; } // Not yet implemented
    if (!Extension_Request_Pointer_Remap<OverlayClass, OverlayClassExtension>(Overlays)) { return false; }
    if (!Extension_Request_Pointer_Remap<OverlayTypeClass, OverlayTypeClassExtension>(OverlayTypes)) { return false; }
    //if (!Extension_Request_Pointer_Remap<ParticleClass, ParticleClassExtension>(Particles)) { return false; }         // Not yet implemented
    if (!Extension_Request_Pointer_Remap<ParticleTypeClass, ParticleTypeClassExtension>(ParticleTypes)) { return false; }
    //if (!Extension_Request_Pointer_Remap<ParticleSystemClass, ParticleSystemClassExtension>(ParticleSystems)) { return false; } // Not yet implemented
    if (!Extension_Request_Pointer_Remap<ParticleSystemTypeClass, ParticleSystemTypeClassExtension>(ParticleSystemTypes)) { return false; }
    //if (!Extension_Request_Pointer_Remap<ScriptClass, ScriptClassExtension>(Scripts)) { return false; }               // Not yet implemented
    //if (!Extension_Request_Pointer_Remap<ScriptTypeClass, ScriptTypeClassExtension>(ScriptTypes)) { return false; }   // Not yet implemented
    if (!Extension_Request_Pointer_Remap<SideClass, SideClassExtension>(Sides)) { return false; }
    if (!Extension_Request_Pointer_Remap<SmudgeClass, SmudgeClassExtension>(Smudges)) { return false; }
    if (!Extension_Request_Pointer_Remap<SmudgeTypeClass, SmudgeTypeClassExtension>(SmudgeTypes)) { return false; }
    if (!Extension_Request_Pointer_Remap<SuperWeaponTypeClass, SuperWeaponTypeClassExtension>(SuperWeaponTypes)) { return false; }
    //if (!Extension_Request_Pointer_Remap<TaskForceClass, TaskForceClassExtension>(TaskForces)) { return false; }      // Not yet implemented
    //if (!Extension_Request_Pointer_Remap<TeamClass, TeamClassExtension>(Teams)) { return false; }                     // Not yet implemented
    //if (!Extension_Request_Pointer_Remap<TeamTypeClass, TeamTypeClassExtension>(TeamTypes)) { return false; }         // Not yet implemented
    if (!Extension_Request_Pointer_Remap<TerrainClass, TerrainClassExtension>(Terrains)) { return false; }
    if (!Extension_Request_Pointer_Remap<TerrainTypeClass, TerrainTypeClassExtension>(TerrainTypes)) { return false; }
    //if (!Extension_Request_Pointer_Remap<TriggerClass, TriggerClassExtension>(Triggers)) { return false; }            // Not yet implemented
    //if (!Extension_Request_Pointer_Remap<TriggerTypeClass, TriggerTypeClassExtension>(TriggerTypes)) { return false; } // Not yet implemented
    if (!Extension_Request_Pointer_Remap<UnitTypeClass, UnitTypeClassExtension>(UnitTypes)) { return false; }
    //if (!Extension_Request_Pointer_Remap<VoxelAnimClass, VoxelAnimClassExtension>(VoxelAnims)) { return false; }      // Not yet implemented
    if (!Extension_Request_Pointer_Remap<VoxelAnimTypeClass, VoxelAnimTypeClassExtension>(VoxelAnimTypes)) { return false; }
    if (!Extension_Request_Pointer_Remap<WaveClass, WaveClassExtension>(Waves)) { return false; }
    //if (!Extension_Request_Pointer_Remap<TagClass, TagClassExtension>(Tags)) { return false; }                        // Not yet implemented
    //if (!Extension_Request_Pointer_Remap<TagTypeClass, TagTypeClassExtension>(TagTypes)) { return false; }            // Not yet implemented
    if (!Extension_Request_Pointer_Remap<TiberiumClass, TiberiumClassExtension>(Tiberiums)) { return false; }
    //if (!Extension_Request_Pointer_Remap<TActionClass, TActionClassExtension>(TActions)) { return false; }            // Not yet implemented
    //if (!Extension_Request_Pointer_Remap<TEventClass, TEventClassExtension>(TEvents)) { return false; }               // Not yet implemented
    if (!Extension_Request_Pointer_Remap<WeaponTypeClass, WeaponTypeClassExtension>(WeaponTypes)) { return false; }
    if (!Extension_Request_Pointer_Remap<WarheadTypeClass, WarheadTypeClassExtension>(WarheadTypes)) { return false; }
    //if (!Extension_Request_Pointer_Remap<WaypointClass, WaypointClassExtension>(Waypoints)) { return false; }         // Not yet implemented
    //if (!Extension_Request_Pointer_Remap<TubeClass, TubeClassExtension>(Tubes)) { return false; }                     // Not yet implemented
    //if (!Extension_Request_Pointer_Remap<LightSourceClass, LightSourceClassExtension>(LightSources)) { return false; } // Not yet implemented
    //if (!Extension_Request_Pointer_Remap<EMPulseClass, EMPulseClassExtension>(EMPulses)) { return false; }            // Not yet implemented
    if (!Extension_Request_Pointer_Remap<SuperClass, SuperClassExtension>(Supers)) { return false; }
    //if (!Extension_Request_Pointer_Remap<AITriggerClass, AITriggerClassExtension>(AITriggers)) { return false; }      // Not yet implemented
    //if (!Extension_Request_Pointer_Remap<AITriggerTypeClass, AITriggerTypeExtension>(AITriggerTypes)) { return false; } // Not yet implemented
    //if (!Extension_Request_Pointer_Remap<NeuronClass, NeuronClassExtension>(Neurons)) { return false; }               // Not yet implemented
    //if (!Extension_Request_Pointer_Remap<FoggedObjectClass, FoggedObjectClassExtension>(FoggedObjects)) { return false; } // Not yet implemented
    //if (!Extension_Request_Pointer_Remap<AlphaShapeClass, AlphaShapeClassExtension>(AlphaShapes)) { return false; }   // Not yet implemented
    //if (!Extension_Request_Pointer_Remap<VeinholeMonsterClass, VeinholeMonsterClassExtension>(VeinholeMonsters)) { return false; } // Not yet implemented

    DEBUG_INFO("Extension::Request_Pointer_Remap(exit)\n");

    return true;
}


/**
 *  Register the extension class COM factories.
 * 
 *  @author: CCHyper
 */
bool Extension::Register_Class_Factories()
{
    DEV_DEBUG_INFO("Extension::Register_Class_Factories(enter)\n");

    /**
     *  #NOTE: The order of these calls must match the relevant RTTIType order!
     */
    REGISTER_CLASS(UnitClassExtension);
    REGISTER_CLASS(AircraftClassExtension);
    REGISTER_CLASS(AircraftTypeClassExtension);
    REGISTER_CLASS(AnimClassExtension);
    REGISTER_CLASS(AnimTypeClassExtension);
    REGISTER_CLASS(BuildingClassExtension);
    REGISTER_CLASS(BuildingTypeClassExtension);
    //REGISTER_CLASS(BulletClassExtension);                                     // Not yet implemented
    REGISTER_CLASS(BulletTypeClassExtension);
    REGISTER_CLASS(CampaignClassExtension);
    //REGISTER_CLASS(CellClassExtension);                                       // Not yet implemented
    REGISTER_CLASS(FactoryClassExtension);
    REGISTER_CLASS(HouseClassExtension);
    REGISTER_CLASS(HouseTypeClassExtension);
    REGISTER_CLASS(InfantryClassExtension);
    REGISTER_CLASS(InfantryTypeClassExtension);
    //REGISTER_CLASS(IsometricTileClassExtension);                              // Not yet implemented
    REGISTER_CLASS(IsometricTileTypeClassExtension);
    //REGISTER_CLASS(BuildingLightClassExtension);                              // Not yet implemented
    REGISTER_CLASS(OverlayClassExtension);
    REGISTER_CLASS(OverlayTypeClassExtension);
    //REGISTER_CLASS(ParticleClassExtension);                                   // Not yet implemented
    REGISTER_CLASS(ParticleTypeClassExtension);
    //REGISTER_CLASS(ParticleSystemClassExtension);                             // Not yet implemented
    REGISTER_CLASS(ParticleSystemTypeClassExtension);
    //REGISTER_CLASS(ScriptClassExtension);                                     // Not yet implemented
    //REGISTER_CLASS(ScriptTypeClassExtension);                                 // Not yet implemented
    REGISTER_CLASS(SideClassExtension);
    REGISTER_CLASS(SmudgeClassExtension);
    REGISTER_CLASS(SmudgeTypeClassExtension);
    REGISTER_CLASS(SuperWeaponTypeClassExtension);
    //REGISTER_CLASS(TaskForceClassExtension);                                  // Not yet implemented
    //REGISTER_CLASS(TeamClassExtension);                                       // Not yet implemented
    //REGISTER_CLASS(TeamTypeClassExtension);                                   // Not yet implemented
    REGISTER_CLASS(TerrainClassExtension);
    REGISTER_CLASS(TerrainTypeClassExtension);
    //REGISTER_CLASS(TriggerClassExtension);                                    // Not yet implemented
    //REGISTER_CLASS(TriggerTypeClassExtension);                                // Not yet implemented
    REGISTER_CLASS(UnitTypeClassExtension);
    //REGISTER_CLASS(VoxelAnimClassExtension);                                  // Not yet implemented
    REGISTER_CLASS(VoxelAnimTypeClassExtension);
    REGISTER_CLASS(WaveClassExtension);
    //REGISTER_CLASS(TagClassExtension);                                        // Not yet implemented
    //REGISTER_CLASS(TagTypeClassExtension);                                    // Not yet implemented
    REGISTER_CLASS(TiberiumClassExtension);
    //REGISTER_CLASS(TActionClassExtension);                                    // Not yet implemented
    //REGISTER_CLASS(TEventClassExtension);                                     // Not yet implemented
    REGISTER_CLASS(WeaponTypeClassExtension);
    REGISTER_CLASS(WarheadTypeClassExtension);
    //REGISTER_CLASS(WaypointClassExtension);                                   // Not yet implemented
    //REGISTER_CLASS(TubeClassExtension);                                       // Not yet implemented
    //REGISTER_CLASS(LightSourceClassExtension);                                // Not yet implemented
    //REGISTER_CLASS(EMPulseClassExtension);                                    // Not yet implemented
    //REGISTER_CLASS(TacticalExtension);                                        // Tactical extension is now a global class and no longer uses COM, so we don't need to register it.
    REGISTER_CLASS(SuperClassExtension);
    //REGISTER_CLASS(AITriggerClassExtension);                                  // Not yet implemented
    //REGISTER_CLASS(AITriggerTypeClassExtension);                              // Not yet implemented
    //REGISTER_CLASS(NeuronClassExtension);                                     // Not yet implemented
    //REGISTER_CLASS(FoggedObjectClassExtension);                               // Not yet implemented
    //REGISTER_CLASS(AlphaShapeClassExtension);                                 // Not yet implemented
    //REGISTER_CLASS(VeinholeMonsterClassExtension););                          // Not yet implemented
    
    DEV_DEBUG_INFO("Extension::Register_Class_Factories(exit)\n");

    return true;
}


/**
 *  Clear out the extension class heaps.
 * 
 *  @author: CCHyper
 */
void Extension::Free_Heaps()
{
    DEV_DEBUG_INFO("Extension::Free_Heaps(enter)\n");

    ++ScenarioInit;

    /**
     *  #NOTE: The order of these calls must match the relevant RTTIType order!
     */
    UnitExtensions.Clear();
    AircraftExtensions.Clear();
    AircraftTypeExtensions.Clear();
    AnimExtensions.Clear();
    AnimTypeExtensions.Clear();
    BuildingExtensions.Clear();
    BuildingTypeExtensions.Clear();
    //BulletExtensions.Clear();                                                 // Not yet implemented
    BulletTypeExtensions.Clear();
    //CampaignExtensions.Clear();                                               // Campaign's do not need to be processed.
    //CellExtensions.Clear();                                                   // Not yet implemented
    FactoryExtensions.Clear();
    HouseExtensions.Clear();
    HouseTypeExtensions.Clear();
    InfantryExtensions.Clear();
    InfantryTypeExtensions.Clear();
    //IsometricTileExtensions.Clear();                                          // Not yet implemented
    //IsometricTileTypeExtensions.Clear();                                      // IsoTileType's not need to be processed.
    //BuildingLightExtensions.Clear();                                          // Not yet implemented
    OverlayExtensions.Clear();
    OverlayTypeExtensions.Clear();
    //ParticleExtensions.Clear();                                               // Not yet implemented
    ParticleTypeExtensions.Clear();
    //ParticleSystemExtensions.Clear();                                         // Not yet implemented
    ParticleSystemTypeExtensions.Clear();
    //ScriptExtensions.Clear();                                                 // Not yet implemented
    //ScriptTypeExtensions.Clear();                                             // Not yet implemented
    SideExtensions.Clear();
    SmudgeExtensions.Clear();
    SmudgeTypeExtensions.Clear();
    SuperWeaponTypeExtensions.Clear();
    //TaskForceExtensions.Clear();                                              // Not yet implemented
    //TeamExtensions.Clear();                                                   // Not yet implemented
    //TeamTypeExtensions.Clear();                                               // Not yet implemented
    TerrainExtensions.Clear();
    TerrainTypeExtensions.Clear();
    //TriggerExtensions.Clear();                                                // Not yet implemented
    //TriggerTypeExtensions.Clear();                                            // Not yet implemented
    UnitTypeExtensions.Clear();
    //VoxelAnimExtensions.Clear();                                              // Not yet implemented
    VoxelAnimTypeExtensions.Clear();
    WaveExtensions.Clear();
    //TagExtensions.Clear();                                                    // Not yet implemented
    //TagTypeExtensions.Clear();                                                // Not yet implemented
    TiberiumExtensions.Clear();
    //TActionExtensions.Clear();                                                // Not yet implemented
    //TEventExtensions.Clear();                                                 // Not yet implemented
     WeaponTypeExtensions.Clear();
    WarheadTypeExtensions.Clear();
    //WaypointExtensions.Clear();                                               // Not yet implemented
    //TubeExtensions.Clear();                                                   // Not yet implemented
    //LightSourceExtensions.Clear();                                            // Not yet implemented
    //EMPulseExtensions.Clear();                                                // Not yet implemented
    //delete TacticalMapExtension;                                              // Does not need to be processed here, class has been promoted to a global.
    SuperExtensions.Clear();
    //AITriggerExtensions.Clear();                                              // Not yet implemented
    //AITriggerTypeExtensions.Clear();                                          // Not yet implemented
    //NeuronExtensions.Clear();                                                 // Not yet implemented
    //FoggedObjectExtensions.Clear();                                           // Not yet implemented
    //AlphaShapeExtensions.Clear();                                             // Not yet implemented
    //VeinholeMonsterExtensions.Clear();                                        // Not yet implemented

    --ScenarioInit;

    DEV_DEBUG_INFO("Extension::Free_Heaps(exit)\n");
}


/**
 *  Prints all the events from the queue list.
 *
 *  @author: CCHyper
 */
template<class T, int I>
static bool Print_Event_List(FILE *fp, QueueClass<T, I> &list)
{
    for (int index = 0; index < list.Count; ++index) {
        EventClass *ev = &list[index];
        if (ev) {
            char ev_byte_format[4];
            Wstring ev_data_buffer;
            int ev_size = EventClass::Event_Length(ev->Type);
            const char *ev_name = EventClass::Event_Name(ev->Type);
            for (int i = 0; i < ev_size; ++i) {
                std::snprintf(ev_byte_format, sizeof(ev_byte_format), "%02X", (unsigned char)ev->Data.Array.Byte[i]); // We use this union member so we can do array access.
                ev_data_buffer += ev_byte_format;
                if (i < ev_size-1) ev_data_buffer += " ";
            }

            std::fprintf(fp, "%04d  %s  Frame: %d  ID: %d  Data: %s\n", index, EventClass::Event_Name(ev->Type), ev->Frame, ev->ID, ev_data_buffer);
        }
    }
    return true;
}


/**
 *  Prints the unique crc for each object in the list.
 *
 *  @author: CCHyper
 */
template<class T>
static void Print_Heap_CRC_Lists(FILE *fp, DynamicVectorClass<T *> &list)
{
    CRCEngine *crc = new CRCEngine;

    std::fprintf(fp, "\n\n********* %s CRCs ********\n\n", Extension::Utility::Get_TypeID_Name<T>().c_str());
    std::fprintf(fp, "Index    CRC\n");
    std::fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    for (int index = 0; index < list.Count(); ++index) {
        T *ptr = list[index];
        ptr->Object_CRC(*crc);
        std::fprintf(fp, "%05d    %08x\n", index, crc->CRC_Value());
        EXT_DEBUG_INFO("%05d %08x\n", index, crc->CRC_Value());
    }

    delete crc;
}


/**
 *  Prints a data file for finding Sync Bugs.
 *
 *  @author: CCHyper
 */
void Extension::Print_CRCs(EventClass *ev)
{
    /**
     *  Create a unique filename for the sync log based on the time of execution and the player name.
     */
    char filename_buffer[512];
    std::snprintf(filename_buffer, sizeof(filename_buffer), "%s\\SYNC_%s-%02d_%02u-%02u-%04u_%02u-%02u-%02u-%d.LOG",
        Vinifera_DebugDirectory,
        PlayerPtr->IniName,
        PlayerPtr->HeapID,
        Execute_Day, Execute_Month, Execute_Year, Execute_Hour, Execute_Min, Execute_Sec, Frame);

    /**
     *  Open the sync log.
     */
    FILE *fp = std::fopen(filename_buffer, "w+");
    if (fp == nullptr) {
        DEBUG_ERROR("Failed to open sync log file for writing!\n");
        return;
    }

    DEBUG_INFO("Writing sync log to file %s.\n", filename_buffer);

    Extension::Print_CRCs(fp, ev);

    std::fclose(fp);
}


const char* Facing_To_String(FacingType facing) 
{
    if (facing == FACING_NONE) {
        return "";
    }

    static const char* facing_names[8] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" };

    return facing_names[(int)facing];
}


void Print_Path(FILE* fp, FootClass *foot)
{
    // Print path
    FacingType facing = foot->Path[0];
    int pathindex = 0;

    while (facing != FACING_NONE) {
        std::fprintf(fp, Facing_To_String(facing));
        std::fprintf(fp, " ");
        pathindex++;
        facing = foot->Path[pathindex];
    }

    std::fprintf(fp, "\n");
}


/**
 *  Prints a data file for finding Sync Bugs.
 * 
 *  @author: CCHyper
 */
void Extension::Print_CRCs(FILE *fp, EventClass *ev)
{
    DEV_DEBUG_INFO("Extension::Print_CRCs(enter)\n");

    std::fprintf(fp, "--------------------------------------------------------------------------------\n");
    std::fprintf(fp, "---------------------  V I N I F E R A   S Y N C   L O G  ----------------------\n");
    std::fprintf(fp, "--------------------------------------------------------------------------------\n");
    std::fprintf(fp, "\n");
    std::fprintf(fp, "Build Type : %s\n", Vinifera_Build_Type_String());
    std::fprintf(fp, "TS++ commit author: %s\n", TSPP_Git_Author());
    std::fprintf(fp, "TS++ commit date: %s\n", TSPP_Git_DateTime());
    std::fprintf(fp, "TS++ commit branch: %s\n", "master"); // TSPP_Git_Branch());
    std::fprintf(fp, "TS++ commit hash: %s\n", TSPP_Git_Hash_Short());
    std::fprintf(fp, "TS++ local changes: %s\n", TSPP_Git_Uncommitted_Changes() ? "YES" : "NO");
    std::fprintf(fp, "Vinifera commit author: %s\n", Vinifera_Git_Author());
    std::fprintf(fp, "Vinifera commit date: %s\n", Vinifera_Git_DateTime());
    std::fprintf(fp, "Vinifera commit branch: %s\n", Vinifera_Git_Branch());
    std::fprintf(fp, "Vinifera commit hash: %s\n", Vinifera_Git_Hash_Short());
    std::fprintf(fp, "Vinifera local changes: %s\n", Vinifera_Git_Uncommitted_Changes() ? "YES" : "NO");
    std::fprintf(fp, "\n");
    std::fprintf(fp, "--------------------------------------------------------------------------------\n");
    std::fprintf(fp, "\n");

    std::fprintf(fp, "Frames: %d\n", Frame);
    std::fprintf(fp, "Player ID: %02d\n", PlayerPtr->HeapID);
    std::fprintf(fp, "Player Name: %s\n", PlayerPtr->IniName);
    //std::fprintf(fp, "Average FPS: %d\n", total_cycles_or_iterations_ > 0 ? total_fps_ / total_cycles_or_iterations_ : 0);
    std::fprintf(fp, "Max MaxAhead: %d\n", Session.MaxMaxAhead);
    std::fprintf(fp, "FrameSendRate: %d\n", Session.FrameSendRate);
    std::fprintf(fp, "Latency setting: %d\n", Session.LatencyFudge);
    std::fprintf(fp, "Game speed setting: %d\n", Options.GameSpeed);
    std::fprintf(fp, "Scenario random: %d\n", Scen->RandomNumber());
    std::fprintf(fp, "Random seed: %d\n", Seed);
    std::fprintf(fp, "\n");

    /**
     *  Print the local addresses.
     */
    if (PacketTransport) {
        for (int index = 0; index < PacketTransport->Local_Addresses_Count(); ++index) {
            unsigned char *addr = PacketTransport->Get_Local_Address(index);
            if (addr) {
                std::fprintf(fp, "Local address: %d.%d.%d.%d\n", addr[0], addr[1], addr[2], addr[3]);
            }
        }
    }
    std::fprintf(fp, "\n");

    /**
     *  Print information on each of the players in the game.
     */
    for (int index = 0; index < MAX_MULTI_NAMES; ++index) {
        MPStatsType &player_stats = Session.Stats[index];
        if (std::strlen(player_stats.Name) > 0) {
            std::fprintf(fp, "Name: %s\n", player_stats.Name);
            std::fprintf(fp, "Address: %s\n", player_stats.Address.As_String());
            std::fprintf(fp, "Max avg round trip: %d\n", player_stats.MaxAvgRoundTrip);
            std::fprintf(fp, "Max round trip: %d\n", player_stats.MaxRoundTrip);
            std::fprintf(fp, "Resends: %d\n", player_stats.Resends);
            std::fprintf(fp, "Frame sync stalls: %d\n", player_stats.FrameSyncStalls);
            std::fprintf(fp, "Command count stalls: %d\n", player_stats.CommandCountStalls);
            std::fprintf(fp, "Lost: %d\n", player_stats.Lost);
            std::fprintf(fp, "Percent lost: %d\n", player_stats.PercentLost);
            std::fprintf(fp, "\n");
        }
    }
    //std::fprintf(fp, "\n");

    /**
     *  Print the most recent CRC values.
     * 
     *  Rampastring: print all of 'em
     */
    for (int i = 0; i < 256; ++i) {
        std::fprintf(fp, "CRC[%d]=%x\n", i, CRC[i]);
    }
    std::fprintf(fp, "\n");

    /**
     *  Log the extension heap sizes.
     */
    std::fprintf(fp, "-------------------------- Heap Sizes -------------------------\n");
    std::fprintf(fp, "Units.Count = %d\n", Units.Count());
    std::fprintf(fp, "Aircrafts.Count = %d\n", Aircrafts.Count());
    std::fprintf(fp, "AircraftTypes.Count = %d\n", AircraftTypes.Count());
    std::fprintf(fp, "Anims.Count = %d\n", Anims.Count());
    std::fprintf(fp, "AnimTypes.Count = %d\n", AnimTypes.Count());
    std::fprintf(fp, "Buildings.Count = %d\n", Buildings.Count());
    std::fprintf(fp, "BuildingTypes.Count = %d\n", BuildingTypes.Count());
    std::fprintf(fp, "Bullets.Count = %d\n", Bullets.Count());
    std::fprintf(fp, "BulletTypes.Count = %d\n", BulletTypes.Count());
    //std::fprintf(fp, "Campaigns.Count = %d\n", Campaigns.Count());            // Does not need to be processed as these have no impact on networking.
    std::fprintf(fp, "Factories.Count = %d\n", Factories.Count());
    std::fprintf(fp, "Houses.Count = %d\n", Houses.Count());
    std::fprintf(fp, "HouseTypes.Count = %d\n", HouseTypes.Count());
    std::fprintf(fp, "Infantry.Count = %d\n", Infantry.Count());
    std::fprintf(fp, "InfantryTypes.Count = %d\n", InfantryTypes.Count());
    std::fprintf(fp, "IsometricTiles.Count = %d\n", IsoTiles.Count());
    std::fprintf(fp, "IsometricTileTypes.Count = %d\n", IsoTileTypes.Count());
    std::fprintf(fp, "BuildingLights.Count = %d\n", BuildingLights.Count());
    std::fprintf(fp, "Overlays.Count = %d\n", Overlays.Count());
    std::fprintf(fp, "OverlayTypes.Count = %d\n", OverlayTypes.Count());
    std::fprintf(fp, "Particles.Count = %d\n", Particles.Count());
    std::fprintf(fp, "ParticleTypes.Count = %d\n", ParticleTypes.Count());
    std::fprintf(fp, "ParticleSystems.Count = %d\n", ParticleSystems.Count());
    std::fprintf(fp, "ParticleSystemTypes.Count = %d\n", ParticleSystemTypes.Count());
    std::fprintf(fp, "Scripts.Count = %d\n", Scripts.Count());
    std::fprintf(fp, "ScriptTypes.Count = %d\n", ScriptTypes.Count());
    std::fprintf(fp, "Sides.Count = %d\n", Sides.Count());
    std::fprintf(fp, "Smudges.Count = %d\n", Smudges.Count());
    std::fprintf(fp, "SmudgeTypes.Count = %d\n", SmudgeTypes.Count());
    std::fprintf(fp, "SuperWeaponTypes.Count = %d\n", SuperWeaponTypes.Count());
    std::fprintf(fp, "TaskForces.Count = %d\n", TaskForces.Count());
    std::fprintf(fp, "Teams.Count = %d\n", Teams.Count());
    std::fprintf(fp, "TeamTypes.Count = %d\n", TeamTypes.Count());
    std::fprintf(fp, "Terrains.Count = %d\n", Terrains.Count());
    std::fprintf(fp, "TerrainTypes.Count = %d\n", TerrainTypes.Count());
    std::fprintf(fp, "Triggers.Count = %d\n", Triggers.Count());
    std::fprintf(fp, "TriggerTypes.Count = %d\n", TriggerTypes.Count());
    std::fprintf(fp, "UnitTypes.Count = %d\n", UnitTypes.Count());
    std::fprintf(fp, "VoxelAnims.Count = %d\n", VoxelAnims.Count());
    std::fprintf(fp, "VoxelAnimTypes.Count = %d\n", VoxelAnimTypes.Count());
    std::fprintf(fp, "Waves.Count = %d\n", Waves.Count());
    std::fprintf(fp, "Tags.Count = %d\n", Tags.Count());
    std::fprintf(fp, "TagTypes.Count = %d\n", TagTypes.Count());
    std::fprintf(fp, "Tiberiums.Count = %d\n", Tiberiums.Count());
    std::fprintf(fp, "TActions.Count = %d\n", TActions.Count());
    std::fprintf(fp, "TEvents.Count = %d\n", TEvents.Count());
    std::fprintf(fp, "WeaponTypes.Count = %d\n", WeaponTypes.Count());
    std::fprintf(fp, "WarheadTypes.Count = %d\n", WarheadTypes.Count());
    std::fprintf(fp, "WaypointPaths.Count = %d\n", WaypointPaths.Count());
    std::fprintf(fp, "Tubes.Count = %d\n", Tubes.Count());
    std::fprintf(fp, "LightSources.Count = %d\n", LightSources.Count());
    std::fprintf(fp, "Empulses.Count = %d\n", Empulses.Count());
    std::fprintf(fp, "Supers.Count = %d\n", Supers.Count());
    std::fprintf(fp, "AITriggerTypes.Count = %d\n", AITriggerTypes.Count());
    std::fprintf(fp, "FoggedObjects.Count = %d\n", FoggedObjects.Count());
    std::fprintf(fp, "AlphaShapes.Count = %d\n", AlphaShapes.Count());
    std::fprintf(fp, "VeinholeMonsters.Count = %d\n", VeinholeMonsters.Count());
    std::fprintf(fp, "\n");

    std::fprintf(fp, "UnitExtensions.Count = %d\n", UnitExtensions.Count());
    std::fprintf(fp, "AircraftExtensions.Count = %d\n", AircraftExtensions.Count());
    std::fprintf(fp, "AircraftTypeExtensions.Count = %d\n", AircraftTypeExtensions.Count());
    std::fprintf(fp, "AnimExtensions.Count = %d\n", AnimExtensions.Count());
    std::fprintf(fp, "AnimTypeExtensions.Count = %d\n", AnimTypeExtensions.Count());
    std::fprintf(fp, "BuildingExtensions.Count = %d\n", BuildingExtensions.Count());
    std::fprintf(fp, "BuildingTypeExtensions.Count = %d\n", BuildingTypeExtensions.Count());
    //std::fprintf(fp, "BulletExtensions.Count = %d\n", BulletExtensions.Count());                                      // Not yet implemented
    std::fprintf(fp, "BulletTypeExtensions.Count = %d\n", BulletTypeExtensions.Count());
    //std::fprintf(fp, "CampaignExtensions.Count = %d\n", CampaignExtensions.Count());                                  // Does not need to be logged as these have no impact on networking.
    std::fprintf(fp, "FactoryExtensions.Count = %d\n", FactoryExtensions.Count());
    std::fprintf(fp, "HouseExtensions.Count = %d\n", HouseExtensions.Count());
    std::fprintf(fp, "HouseTypeExtensions.Count = %d\n", HouseTypeExtensions.Count());
    std::fprintf(fp, "InfantryExtensions.Count = %d\n", InfantryExtensions.Count());
    std::fprintf(fp, "InfantryTypeExtensions.Count = %d\n", InfantryTypeExtensions.Count());
    //std::fprintf(fp, "IsometricTileExtensions.Count = %d\n", IsometricTileExtensions.Count());                        // Not yet implemented
    std::fprintf(fp, "IsometricTileTypeExtensions.Count = %d\n", IsometricTileTypeExtensions.Count());
    //std::fprintf(fp, "BuildingLightExtensions.Count = %d\n", BuildingLightExtensions.Count());                        // Not yet implemented
    std::fprintf(fp, "OverlayExtensions.Count = %d\n", OverlayExtensions.Count());
    std::fprintf(fp, "OverlayTypeExtensions.Count = %d\n", OverlayTypeExtensions.Count());
    //std::fprintf(fp, "ParticleExtensions.Count = %d\n", ParticleExtensions.Count());                                  // Not yet implemented
    std::fprintf(fp, "ParticleTypeExtensions.Count = %d\n", ParticleTypeExtensions.Count());
    //std::fprintf(fp, "ParticleSystemExtensions.Count = %d\n", ParticleSystemExtensions.Count());                      // Not yet implemented
    std::fprintf(fp, "ParticleSystemTypeExtensions.Count = %d\n", ParticleSystemTypeExtensions.Count());
    //std::fprintf(fp, "ScriptExtensions.Count = %d\n", ScriptExtensions.Count());                                      // Not yet implemented
    //std::fprintf(fp, "ScriptTypeExtensions.Count = %d\n", ScriptTypeExtensions.Count());                              // Not yet implemented
    std::fprintf(fp, "SideExtensions.Count = %d\n", SideExtensions.Count());
    std::fprintf(fp, "SmudgeExtensions.Count = %d\n", SmudgeExtensions.Count());
    std::fprintf(fp, "SmudgeTypeExtensions.Count = %d\n", SmudgeTypeExtensions.Count());
    std::fprintf(fp, "SuperWeaponTypeExtensions.Count = %d\n", SuperWeaponTypeExtensions.Count());
    //std::fprintf(fp, "TaskForceExtensions.Count = %d\n", TaskForceExtensions.Count());                                // Not yet implemented
    //std::fprintf(fp, "TeamExtensions.Count = %d\n", TeamExtensions.Count());                                          // Not yet implemented
    //std::fprintf(fp, "TeamTypeExtensions.Count = %d\n", TeamTypeExtensions.Count());                                  // Not yet implemented
    std::fprintf(fp, "TerrainExtensions.Count = %d\n", TerrainExtensions.Count());
    std::fprintf(fp, "TerrainTypeExtensions.Count = %d\n", TerrainTypeExtensions.Count());
    //std::fprintf(fp, "TriggerExtensions.Count = %d\n", TriggerExtensions.Count());                                    // Not yet implemented
    //std::fprintf(fp, "TriggerTypeExtensions.Count = %d\n", TriggerTypeExtensions.Count());                            // Not yet implemented
    std::fprintf(fp, "UnitTypeExtensions.Count = %d\n", UnitTypeExtensions.Count());
    //std::fprintf(fp, "VoxelAnimExtensions.Count = %d\n", VoxelAnimExtensions.Count());                                // Not yet implemented
    std::fprintf(fp, "VoxelAnimTypeExtensions.Count = %d\n", VoxelAnimTypeExtensions.Count());
    std::fprintf(fp, "WaveExtensions.Count = %d\n", WaveExtensions.Count());
    //std::fprintf(fp, "TagExtensions.Count = %d\n", TagExtensions.Count());                                            // Not yet implemented
    //std::fprintf(fp, "TagTypeExtensions.Count = %d\n", TagTypeExtensions.Count());                                    // Not yet implemented
    std::fprintf(fp, "TiberiumExtensions.Count = %d\n", TiberiumExtensions.Count());
    //std::fprintf(fp, "TActionExtensions.Count = %d\n", TActionExtensions.Count());                                    // Not yet implemented
    //std::fprintf(fp, "TEventExtensions.Count = %d\n", TEventExtensions.Count());                                      // Not yet implemented
    std::fprintf(fp, "WeaponTypeExtensions.Count = %d\n", WeaponTypeExtensions.Count());
    std::fprintf(fp, "WarheadTypeExtensions.Count = %d\n", WarheadTypeExtensions.Count());
    //std::fprintf(fp, "WaypointExtensions.Count = %d\n", WaypointExtensions.Count());                                  // Not yet implemented
    //std::fprintf(fp, "TubeExtensions.Count = %d\n", TubeExtensions.Count());                                          // Not yet implemented
    //std::fprintf(fp, "LightSourceExtensions.Count = %d\n", LightSourceExtensions.Count());                            // Not yet implemented
    //std::fprintf(fp, "EMPulseExtensions.Count = %d\n", EMPulseExtensions.Count());                                    // Not yet implemented
    std::fprintf(fp, "SuperExtensions.Count = %d\n", SuperExtensions.Count());
    //std::fprintf(fp, "AITriggerExtensions.Count = %d\n", AITriggerExtensions.Count());                                // Not yet implemented
    //std::fprintf(fp, "AITriggerTypeExtensions.Count = %d\n", AITriggerTypeExtensions.Count());                        // Not yet implemented
    //std::fprintf(fp, "NeuronExtensions.Count = %d\n", NeuronExtensions.Count());                                      // Not yet implemented
    //std::fprintf(fp, "FoggedObjectExtensions.Count = %d\n", FoggedObjectExtensions.Count());                          // Not yet implemented
    //std::fprintf(fp, "AlphaShapeExtensions.Count = %d\n", AlphaShapeExtensions.Count());                              // Not yet implemented
    //std::fprintf(fp, "VeinholeMonsterExtensions.Count = %d\n", VeinholeMonsterExtensions.Count());                    // Not yet implemented
    std::fprintf(fp, "\n");

    std::fprintf(fp, "Map.Array.Length = %d\n", Map.Array.Length());
    //std::fprintf(fp, "CellExtensions.Count = %d\n", CellExtensions.Count());                                          // Not yet implemented
    std::fprintf(fp, "\n");

    /**
     *  Houses
     */
    for (int house = 0; house < Houses.Count(); ++house) {
        GameCRC = 0;
        HouseClass *housep = Houses[house];
        if (housep) {
            //const char *a = HouseTypes[housep->HeapID]->Name();
            //const char *b = housep->ActLike != HOUSE_NONE ? HouseTypes[housep->ActLike]->Name() : "<none>";
            std::fprintf(fp, "%s: IsHuman:%d  Color:%s  HeapID:%d  Credits:%d  Power:%d  Drain:%d  HouseType:%s  ActLike:%s\n",
                housep->IniName,
                housep->IsHuman,
                ColorSchemes[housep->RemapColor]->Name,
                housep->HeapID,
                housep->Credits,
                housep->Power,
                housep->Drain,
                housep->Class->Name(),
                housep->ActLike != HOUSE_NONE ? HouseTypes[housep->ActLike]->Name() : "<none>");
            Add_CRC(&GameCRC, (int)housep->Credits + (int)housep->Power + (int)housep->Drain);
            EXT_DEBUG_INFO("House %s:%x\n", housep->Class->Name(), GameCRC);
        }
    }
    std::fprintf(fp, "\n");

    /**
     *  Infantry
     */
    for (int house = 0; house < Houses.Count(); ++house) {
        HouseClass *housep = Houses[house];
        if (housep) {
            GameCRC = 0;
            std::fprintf(fp, "------------- %s (%s %d) %s ------------\n", housep->Class->Name(), housep->IniName, housep->HeapID, Extension::Utility::Get_TypeID_Name<InfantryClass>().c_str());
            for (int index = 0; index < Infantry.Count(); ++index) {
                InfantryClass *ptr = Infantry[index];
                if (ptr->Owner() == house) {
                    Add_CRC(&GameCRC, (int)((ptr->Get_Coord().X / 10) << 16) + (int)(ptr->Get_Coord().Y / 10) + (int)ptr->PrimaryFacing.Current().Get_Dir());

                    const char *tarcom_name = "None";
                    Coordinate tarcom_coord;

                    const char *navcom_name = "None";
                    Coordinate navcom_coord;

                    if (ptr->TarCom) {
                        tarcom_name = Name_From_RTTI(ptr->TarCom->Fetch_RTTI());
                        tarcom_coord = ptr->TarCom->Center_Coord();
                    }

                    if (ptr->NavCom) {
                        navcom_name = Name_From_RTTI(ptr->NavCom->Fetch_RTTI());
                        navcom_coord = ptr->NavCom->Center_Coord();
                    }

                    std::fprintf(fp, "COORD:%d,%d,%d  Facing:%d  Mission:%s  HeapID:%s(%d)  Speed:%d  TarCom:%s(%d,%d,%d)  NavCom:%s(%d,%d,%d)  Doing:%d  Path: ",
                                ptr->Center_Coord().X, ptr->Center_Coord().Y, ptr->Center_Coord().Z,
                                (int)ptr->PrimaryFacing.Current().Get_Dir(), MissionClass::Mission_Name(ptr->Get_Mission()),
                                ptr->Class->Name(), ptr->Class->HeapID,
                                (int)(ptr->Speed * 256.0),
                                tarcom_name, tarcom_coord.X, tarcom_coord.Y, tarcom_coord.Z,
                                navcom_name, navcom_coord.X, navcom_coord.Y, navcom_coord.Z,
                                ptr->Doing);

                    Print_Path(fp, ptr);
                }
            }
            EXT_DEBUG_INFO("%s %s:%x\n", housep->Class->Name(), Extension::Utility::Get_TypeID_Name<InfantryClassExtension>().c_str(), GameCRC);
        }
        std::fprintf(fp, "\n");
    }

    /**
     *  Units
     */
    for (int house = 0; house < Houses.Count(); ++house) {
        HouseClass *housep = Houses[house];
        if (housep) {
            GameCRC = 0;
            std::fprintf(fp, "------------- %s (%s %d) %s ------------\n", housep->Class->Name(), housep->IniName, housep->HeapID, Extension::Utility::Get_TypeID_Name<UnitClass>().c_str());
            for (int index = 0; index < Units.Count(); ++index) {
                UnitClass *ptr = Units[index];
                if (ptr->Owner() == house) {
                    Add_CRC(&GameCRC, (int)((ptr->Get_Coord().X / 10) << 16) + (int)(ptr->Get_Coord().Y / 10) + (int)ptr->PrimaryFacing.Current().Get_Dir());

                    const char *tarcom_name = "None";
                    Coordinate tarcom_coord;

                    const char *navcom_name = "None";
                    Coordinate navcom_coord;

                    if (ptr->TarCom) {
                        tarcom_name = Name_From_RTTI(ptr->TarCom->Fetch_RTTI());
                        tarcom_coord = ptr->TarCom->Center_Coord();
                    }

                    if (ptr->NavCom) {
                        navcom_name = Name_From_RTTI(ptr->NavCom->Fetch_RTTI());
                        navcom_coord = ptr->NavCom->Center_Coord();
                    }

                    std::fprintf(fp, "COORD:%d,%d,%d  Facing:%d  Facing2:%d  Mission:%s  HeapID:%s(%d)  TarCom:%s(%d,%d,%d)  NavCom:%s(%d,%d,%d)  TrkNum:%d  TrkInd:%d  SpdAcc:%d  Path:",
                                ptr->Center_Coord().X, ptr->Center_Coord().Y, ptr->Center_Coord().Z,
                                (int)ptr->PrimaryFacing.Current().Get_Dir(), (int)ptr->SecondaryFacing.Current().Get_Dir(), MissionClass::Mission_Name(ptr->Get_Mission()),
                                ptr->Class->Name(), ptr->Class->HeapID,
                                tarcom_name, tarcom_coord.X, tarcom_coord.Y, tarcom_coord.Z,
                                navcom_name, navcom_coord.X, navcom_coord.Y, navcom_coord.Z,
                                ptr->Locomotor_Ptr()->Get_Track_Number(), ptr->Locomotor_Ptr()->Get_Track_Number(), ptr->Locomotor_Ptr()->Get_Speed_Accum());

                    Print_Path(fp, ptr);
                }
            }
            EXT_DEBUG_INFO("%s %s:%x\n", housep->Class->Name(), Extension::Utility::Get_TypeID_Name<UnitClass>().c_str(), GameCRC);
        }
        std::fprintf(fp, "\n");
    }

    /**
     *  Buildings
     */
    for (int house = 0; house < Houses.Count(); ++house) {
        HouseClass *housep = Houses[house];
        if (housep) {
            GameCRC = 0;
            std::fprintf(fp, "------------- %s (%s %d) %s ------------\n", housep->Class->Name(), housep->IniName, housep->HeapID, Extension::Utility::Get_TypeID_Name<BuildingClass>().c_str());
            for (int index = 0; index < Buildings.Count(); ++index) {
                BuildingClass *ptr = Buildings[index];
                if (ptr->Owner() == house) {
                    Add_CRC(&GameCRC, (int)((ptr->Get_Coord().X / 10) << 16) + (int)(ptr->Get_Coord().Y / 10) + (int)ptr->PrimaryFacing.Current().Get_Dir());

                    const char *tarcom_name = "None";
                    Coordinate tarcom_coord;

                    if (ptr->TarCom) {
                        tarcom_name = Name_From_RTTI(ptr->TarCom->Fetch_RTTI());
                        tarcom_coord = ptr->TarCom->Center_Coord();
                    }

                    std::fprintf(fp, "COORD:%d,%d,%d  Facing:%d  Mission:%s  HeapID:%s(%d)  TarCom:%s(%d,%d,%d)\n",
                                ptr->Center_Coord().X, ptr->Center_Coord().Y, ptr->Center_Coord().Z,
                                (int)ptr->PrimaryFacing.Current().Get_Dir(), MissionClass::Mission_Name(ptr->Get_Mission()),
                                ptr->Class->Name(), ptr->Class->HeapID,
                                tarcom_name, tarcom_coord.X, tarcom_coord.Y, tarcom_coord.Z);
                }
            }
            EXT_DEBUG_INFO("%s %s:%x\n", housep->Class->Name(), Extension::Utility::Get_TypeID_Name<BuildingClass>().c_str(), GameCRC);
        }
        std::fprintf(fp, "\n");
    }

    /**
     *  Aircraft
     */
    for (int house = 0; house < Houses.Count(); ++house) {
        HouseClass *housep = Houses[house];
        if (housep) {
            GameCRC = 0;
            std::fprintf(fp, "------------- %s (%s %d) %s ------------\n", housep->Class->Name(), housep->IniName, housep->HeapID, Extension::Utility::Get_TypeID_Name<AircraftClass>().c_str());
            for (int index = 0; index < Aircrafts.Count(); ++index) {
                AircraftClass *ptr = Aircrafts[index];
                if (ptr->Owner() == house) {
                    Add_CRC(&GameCRC, (int)((ptr->Get_Coord().X / 10) << 16) + (int)(ptr->Get_Coord().Y / 10) + (int)ptr->PrimaryFacing.Current().Get_Dir());

                    const char *tarcom_name = "None";
                    Coordinate tarcom_coord;

                    const char *navcom_name = "None";
                    Coordinate navcom_coord;

                    if (ptr->TarCom) {
                        tarcom_name = Name_From_RTTI(ptr->TarCom->Fetch_RTTI());
                        tarcom_coord = ptr->TarCom->Center_Coord();
                    }

                    if (ptr->NavCom) {
                        navcom_name = Name_From_RTTI(ptr->NavCom->Fetch_RTTI());
                        navcom_coord = ptr->NavCom->Center_Coord();
                    }

                    std::fprintf(fp, "COORD:%d,%d,%d  Facing:%d  Mission:%s  HeapID:%s(%d) TarCom:%s(%d,%d,%d)  NavCom:%s(%d,%d,%d)  Path:",
                                ptr->Center_Coord().X, ptr->Center_Coord().Y, ptr->Center_Coord().Z,
                                (int)ptr->PrimaryFacing.Current().Get_Dir(), MissionClass::Mission_Name(ptr->Get_Mission()),
                                ptr->Class->Name(), ptr->Class->HeapID,
                                tarcom_name, tarcom_coord.X, tarcom_coord.Y, tarcom_coord.Z,
                                navcom_name, navcom_coord.X, navcom_coord.Y, navcom_coord.Z);

                    Print_Path(fp, ptr);
                }
            }
            EXT_DEBUG_INFO("%s %s:%x\n", housep->Class->Name(), Extension::Utility::Get_TypeID_Name<AircraftClass>().c_str(), GameCRC);
        }
        std::fprintf(fp, "\n");
    }

    /**
     *  Projectiles
     */
    std::fprintf(fp, "-------------------- Projectiles / Bullets ------------------ - \n");
    for (int index = 0; index < Bullets.Count(); ++index) {
        BulletClass *bullet = Bullets[index];

        const char *bullet_name = bullet->Full_Name();

        const char* payback = "None";
        const char* payback_owner = "None";
        int owner_id = -1;

        if (bullet->Payback) {
            payback = bullet->Payback->Full_Name();

            payback_owner = bullet->Payback->Owner_HouseClass()->IniName;
            owner_id = bullet->Payback->Owner();
        }

        std::fprintf(fp, "Coord:%d,%d,%d  TargetCoord:(%d,%d,%d)  Payback:%s  Owner:%s  OwnerID:%d  HeapID:%s\n",
            bullet->Center_Coord().X, bullet->Center_Coord().Y, bullet->Center_Coord().Z,
            bullet->Target_Coord().X, bullet->Target_Coord().Y, bullet->Target_Coord().Z,
            payback, payback_owner, owner_id, bullet_name);
    }
    std::fprintf(fp, "\n");

    /**
     *  Animations
     */
    std::fprintf(fp, "-------------------- Animations -------------------\n");
    for (int index = 0; index < Anims.Count(); ++index) {
        AnimClass *animp = Anims[index];
        const char *xobject_name = "None";
        Coordinate xobject_coord;

        if (animp->xObject) {
            xobject_name = Name_From_RTTI(animp->xObject->Fetch_RTTI());
            xobject_coord = animp->xObject->Center_Coord();
        }

        const char *anim_name = animp->Full_Name();

        std::fprintf(fp, "Coord:%d,%d,%d  Target:%s(%d,%d,%d)  OwnerHouse:%d  Loops:%d  HeapID:%s  \n",
            animp->Center_Coord().X, animp->Center_Coord().Y, animp->Center_Coord().Z,
            xobject_name, xobject_coord.X, xobject_coord.Y, xobject_coord.Z,
            animp->OwnerHouse,
            animp->Loops,
            anim_name);
    }
    std::fprintf(fp, "\n");

    /**
     *  Map Layers
     */
    GameCRC = 0;
    for (LayerType layer = LAYER_FIRST; layer < LAYER_COUNT; ++layer) {
        std::fprintf(fp, ">>>> MAP LAYER %s (%d) <<<<\n", Name_From_Layer(layer), layer);
        for (int index = 0; index < Map.Layer[layer].Count(); ++index) {
            ObjectClass *objp = Map.Layer[layer][index];
            Add_CRC(&GameCRC, (int)((objp->Get_Coord().X / 10) << 16) + (int)(objp->Get_Coord().Y / 10));
            std::fprintf(fp, "Object %d: %s ", index, objp->Coord.As_String());
            switch (objp->Fetch_RTTI()) {
                case RTTI_AIRCRAFT:
                    std::fprintf(fp, "Aircraft  (HeapID:%s (%d)) ", objp->Name(), objp->Fetch_Heap_ID());
                    break;
                case RTTI_ANIM:
                    std::fprintf(fp, "Anim      (HeapID:%s (%d)) ", objp->Name(), objp->Fetch_Heap_ID());
                        break;
                case RTTI_BUILDING:
                    std::fprintf(fp, "Building  (HeapID:%s (%d)) ", objp->Name(), objp->Fetch_Heap_ID());
                    break;
                case RTTI_BULLET:
                    std::fprintf(fp, "Bullet    (HeapID:%s (%d)) ", objp->Name(), objp->Fetch_Heap_ID());
                    break;
                case RTTI_INFANTRY:
                    std::fprintf(fp, "Infantry  (HeapID:%s (%d)) ", objp->Name(), objp->Fetch_Heap_ID());
                    break;
                case RTTI_OVERLAY:
                    std::fprintf(fp, "Overlay   (HeapID:%s (%d)) ", objp->Name(), objp->Fetch_Heap_ID());
                    break;
                case RTTI_SMUDGE:
                    std::fprintf(fp, "Smudge    (HeapID:%s (%d)) ", objp->Name(), objp->Fetch_Heap_ID());
                    break;
                case RTTI_TERRAIN:
                    std::fprintf(fp, "Terrain   (HeapID:%s (%d)) ", objp->Name(), objp->Fetch_Heap_ID());
                    break;
                case RTTI_UNIT:
                    std::fprintf(fp, "Unit      (HeapID:%s (%d)) ", objp->Name(), objp->Fetch_Heap_ID());
                    break;
                case RTTI_PARTICLE:
                    std::fprintf(fp, "Particle  (HeapID:%s (%d)) ", objp->Name(), objp->Fetch_Heap_ID());
                    break;
            };
            HouseClass *housep = objp->Owner_HouseClass();
            if (housep) {
                std::fprintf(fp, "Owner: %s\n", housep->Class->IniName);
            } else {
                std::fprintf(fp, "Owner: NONE\n");
            }
        }
        std::fprintf(fp, "\n");
    }
    std::fprintf(fp, "\n");
    EXT_DEBUG_INFO("Map Layers: %x\n", GameCRC);

    /**
     *  Logic Layers
     */
    GameCRC = 0;
    std::fprintf(fp, ">>>> LOGIC LAYER <<<<\n");
    for (int index = 0; index < Logic.Count(); ++index) {
        ObjectClass *objp = Logic[index];
        Add_CRC(&GameCRC, (int)((objp->Get_Coord().X / 10) << 16) + (int)(objp->Get_Coord().Y / 10));
        std::fprintf(fp, "Object %d: %s ", index, objp->Coord.As_String());
        switch (objp->Fetch_RTTI()) {
            case RTTI_AIRCRAFT:
                std::fprintf(fp, "Aircraft  (HeapID:%s (%d)) ", objp->Name(), objp->Fetch_Heap_ID());
                break;
            case RTTI_ANIM:
                std::fprintf(fp, "Anim      (HeapID:%s (%d)) ", objp->Name(), objp->Fetch_Heap_ID());
                break;
            case RTTI_BUILDING:
                std::fprintf(fp, "Building  (HeapID:%s (%d)) ", objp->Name(), objp->Fetch_Heap_ID());
                break;
            case RTTI_BULLET:
                std::fprintf(fp, "Bullet    (HeapID:%s (%d)) ", objp->Name(), objp->Fetch_Heap_ID());
                break;
            case RTTI_INFANTRY:
                std::fprintf(fp, "Infantry  (HeapID:%s (%d)) ", objp->Name(), objp->Fetch_Heap_ID());
                break;
            case RTTI_OVERLAY:
                std::fprintf(fp, "Overlay   (HeapID:%s (%d)) ", objp->Name(), objp->Fetch_Heap_ID());
                break;
            case RTTI_SMUDGE:
                std::fprintf(fp, "Smudge    (HeapID:%s (%d)) ", objp->Name(), objp->Fetch_Heap_ID());
                break;
            case RTTI_TERRAIN:
                std::fprintf(fp, "Terrain   (HeapID:%s (%d)) ", objp->Name(), objp->Fetch_Heap_ID());
                break;
            case RTTI_UNIT:
                std::fprintf(fp, "Unit      (HeapID:%s (%d)) ", objp->Name(), objp->Fetch_Heap_ID());
                break;
            case RTTI_PARTICLE:
                std::fprintf(fp, "Particle  (HeapID:%s (%d)) ", objp->Name(), objp->Fetch_Heap_ID());
                break;
        };
        HouseClass *housep = objp->Owner_HouseClass();
        if (housep) {
            std::fprintf(fp, "Owner: %s\n", housep->Class->IniName);
        } else {
            std::fprintf(fp, "Owner: NONE\n");
        }
    }
    std::fprintf(fp, "\n");
    EXT_DEBUG_INFO("Logic: %x\n", GameCRC);

    /**
     *  Random # generator, frame #.
     */
    std::fprintf(fp, "\nRandom Number:%x\n", Scen->RandomNumber());
    EXT_DEBUG_INFO("Random Number: %x\n", Scen->RandomNumber());

    std::fprintf(fp, "My Frame:%d\n", Frame);
    EXT_DEBUG_INFO("My Frame: %d\n", Frame);

    if (ev) {
        std::fprintf(fp, "\n");
        std::fprintf(fp, "Offending event:\n");
        std::fprintf(fp, "  Type:         %s\n", EventClass::Event_Name(ev->Type));
        std::fprintf(fp, "  Frame:        %d\n", ev->Frame);
        std::fprintf(fp, "  ID:           %x\n", ev->ID);
        std::fprintf(fp, "  CRC:          %x\n", ev->Data.FrameInfo.CRC);
        std::fprintf(fp, "  CommandCount: %d\n", ev->Data.FrameInfo.CommandCount);
        std::fprintf(fp, "  Delay:        %d\n", ev->Data.FrameInfo.Delay);
    }
    std::fprintf(fp, "\n");

    /**
     *  Event queues.
     *  Rampastring: printing these causes a crash atm
     */
#if 0
    std::fprintf(fp, "-------------------- DoList Events -------------------\n");
    Print_Event_List(fp, DoList);
    std::fprintf(fp, "\n");

    std::fprintf(fp, "-------------------- OutList Events -------------------\n");
    Print_Event_List(fp, OutList);
    std::fprintf(fp, "\n");
#endif

    /**
     *  Print heap CRC's.
     */
    Print_Heap_CRC_Lists(fp, Units);
    Print_Heap_CRC_Lists(fp, Aircrafts);
    Print_Heap_CRC_Lists(fp, AircraftTypes);
    Print_Heap_CRC_Lists(fp, Anims);
    Print_Heap_CRC_Lists(fp, AnimTypes);
    Print_Heap_CRC_Lists(fp, Buildings);
    Print_Heap_CRC_Lists(fp, BuildingTypes);
    Print_Heap_CRC_Lists(fp, Bullets);
    Print_Heap_CRC_Lists(fp, BulletTypes);
    //Print_Heap_CRC_Lists(fp, Campaigns);                                      // Does not need to be processed as these have no impact on networking.
    Print_Heap_CRC_Lists(fp, Factories);
    Print_Heap_CRC_Lists(fp, Houses);
    Print_Heap_CRC_Lists(fp, HouseTypes);
    Print_Heap_CRC_Lists(fp, Infantry);
    Print_Heap_CRC_Lists(fp, InfantryTypes);
    Print_Heap_CRC_Lists(fp, IsoTiles);
    Print_Heap_CRC_Lists(fp, IsoTileTypes);
    Print_Heap_CRC_Lists(fp, BuildingLights);
    Print_Heap_CRC_Lists(fp, Overlays);
    Print_Heap_CRC_Lists(fp, OverlayTypes);
    Print_Heap_CRC_Lists(fp, Particles);
    Print_Heap_CRC_Lists(fp, ParticleTypes);
    Print_Heap_CRC_Lists(fp, ParticleSystems);
    Print_Heap_CRC_Lists(fp, ParticleSystemTypes);
    Print_Heap_CRC_Lists(fp, Scripts);
    Print_Heap_CRC_Lists(fp, ScriptTypes);
    Print_Heap_CRC_Lists(fp, Sides);
    Print_Heap_CRC_Lists(fp, Smudges);
    Print_Heap_CRC_Lists(fp, SmudgeTypes);
    Print_Heap_CRC_Lists(fp, SuperWeaponTypes);
    Print_Heap_CRC_Lists(fp, TaskForces);
    Print_Heap_CRC_Lists(fp, Teams);
    Print_Heap_CRC_Lists(fp, TeamTypes);
    Print_Heap_CRC_Lists(fp, Terrains);
    Print_Heap_CRC_Lists(fp, TerrainTypes);
    Print_Heap_CRC_Lists(fp, Triggers);
    Print_Heap_CRC_Lists(fp, TriggerTypes);
    Print_Heap_CRC_Lists(fp, UnitTypes);
    Print_Heap_CRC_Lists(fp, VoxelAnims);
    Print_Heap_CRC_Lists(fp, VoxelAnimTypes);
    Print_Heap_CRC_Lists(fp, Waves);
    Print_Heap_CRC_Lists(fp, Tags);
    Print_Heap_CRC_Lists(fp, TagTypes);
    Print_Heap_CRC_Lists(fp, Tiberiums);
    Print_Heap_CRC_Lists(fp, TActions);
    Print_Heap_CRC_Lists(fp, TEvents);
    Print_Heap_CRC_Lists(fp, WeaponTypes);
    Print_Heap_CRC_Lists(fp, WarheadTypes);
    Print_Heap_CRC_Lists(fp, WaypointPaths);
    Print_Heap_CRC_Lists(fp, Tubes);
    Print_Heap_CRC_Lists(fp, LightSources);
    Print_Heap_CRC_Lists(fp, Empulses);
    Print_Heap_CRC_Lists(fp, Supers);
    Print_Heap_CRC_Lists(fp, AITriggerTypes);
    Print_Heap_CRC_Lists(fp, FoggedObjects);
    Print_Heap_CRC_Lists(fp, AlphaShapes);
    Print_Heap_CRC_Lists(fp, VeinholeMonsters);

    /**
     *  Print extension heap CRC's.
     */
    Print_Heap_CRC_Lists(fp, UnitExtensions);
    Print_Heap_CRC_Lists(fp, AircraftExtensions);
    Print_Heap_CRC_Lists(fp, AircraftTypeExtensions);
    Print_Heap_CRC_Lists(fp, AnimExtensions);
    Print_Heap_CRC_Lists(fp, AnimTypeExtensions);
    Print_Heap_CRC_Lists(fp, BuildingExtensions);
    Print_Heap_CRC_Lists(fp, BuildingTypeExtensions);
    //Print_Heap_CRC_Lists(fp, BulletExtensions);                               // Not yet implemented
    Print_Heap_CRC_Lists(fp, BulletTypeExtensions);
    //Print_Heap_CRC_Lists(fp, CampaignExtensions);                             // Does not need to be processed as these have no impact on networking.
    //Print_Heap_CRC_Lists(fp, CellExtensions);                                 // Not yet implemented
    Print_Heap_CRC_Lists(fp, FactoryExtensions);
    Print_Heap_CRC_Lists(fp, HouseExtensions);
    Print_Heap_CRC_Lists(fp, HouseTypeExtensions);
    Print_Heap_CRC_Lists(fp, InfantryExtensions);
    Print_Heap_CRC_Lists(fp, InfantryTypeExtensions);
    //Print_Heap_CRC_Lists(fp, IsometricTileExtensions);                        // Not yet implemented
    Print_Heap_CRC_Lists(fp, IsometricTileTypeExtensions);
    //Print_Heap_CRC_Lists(fp, BuildingLightExtensions);                        // Not yet implemented
    Print_Heap_CRC_Lists(fp, OverlayExtensions);
    Print_Heap_CRC_Lists(fp, OverlayTypeExtensions);
    //Print_Heap_CRC_Lists(fp, ParticleExtensions);                             // Not yet implemented
    Print_Heap_CRC_Lists(fp, ParticleTypeExtensions);
    //Print_Heap_CRC_Lists(fp, ParticleSystemExtensions);                       // Not yet implemented
    Print_Heap_CRC_Lists(fp, ParticleSystemTypeExtensions);
    //Print_Heap_CRC_Lists(fp, ScriptExtensions);                               // Not yet implemented
    //Print_Heap_CRC_Lists(fp, ScriptTypeExtensions);                           // Not yet implemented
    Print_Heap_CRC_Lists(fp, SideExtensions);
    Print_Heap_CRC_Lists(fp, SmudgeExtensions);
    Print_Heap_CRC_Lists(fp, SmudgeTypeExtensions);
    Print_Heap_CRC_Lists(fp, SuperWeaponTypeExtensions);
    //Print_Heap_CRC_Lists(fp, TaskForceExtensions);                            // Not yet implemented
    //Print_Heap_CRC_Lists(fp, TeamExtensions);                                 // Not yet implemented
    //Print_Heap_CRC_Lists(fp, TeamTypeExtensions);                             // Not yet implemented
    Print_Heap_CRC_Lists(fp, TerrainExtensions);
    Print_Heap_CRC_Lists(fp, TerrainTypeExtensions);
    //Print_Heap_CRC_Lists(fp, TriggerExtensions);                              // Not yet implemented
    //Print_Heap_CRC_Lists(fp, TriggerTypeExtensions);                          // Not yet implemented
    Print_Heap_CRC_Lists(fp, UnitTypeExtensions);
    //Print_Heap_CRC_Lists(fp, VoxelAnimExtensions);                            // Not yet implemented
    Print_Heap_CRC_Lists(fp, VoxelAnimTypeExtensions);
    Print_Heap_CRC_Lists(fp, WaveExtensions);
    //Print_Heap_CRC_Lists(fp, TagExtensions);                                  // Not yet implemented
    //Print_Heap_CRC_Lists(fp, TagTypeExtensions);                              // Not yet implemented
    Print_Heap_CRC_Lists(fp, TiberiumExtensions);
    //Print_Heap_CRC_Lists(fp, TActionExtensions);                              // Not yet implemented
    //Print_Heap_CRC_Lists(fp, TEventExtensions);                               // Not yet implemented
    Print_Heap_CRC_Lists(fp, WeaponTypeExtensions);
    Print_Heap_CRC_Lists(fp, WarheadTypeExtensions);
    //Print_Heap_CRC_Lists(fp, WaypointExtensions);                             // Not yet implemented
    //Print_Heap_CRC_Lists(fp, TubeExtensions);                                 // Not yet implemented
    //Print_Heap_CRC_Lists(fp, LightSourceExtensions);                          // Not yet implemented
    //Print_Heap_CRC_Lists(fp, EMPulseExtensions);                              // Not yet implemented
    Print_Heap_CRC_Lists(fp, SuperExtensions);
    //Print_Heap_CRC_Lists(fp, AITriggerExtensions);                            // Not yet implemented
    //Print_Heap_CRC_Lists(fp, AITriggerTypeExtensions);                        // Not yet implemented
    //Print_Heap_CRC_Lists(fp, NeuronExtensions);                               // Not yet implemented
    //Print_Heap_CRC_Lists(fp, FoggedObjectExtensions);                         // Not yet implemented
    //Print_Heap_CRC_Lists(fp, AlphaShapeExtensions);                           // Not yet implemented
    //Print_Heap_CRC_Lists(fp, VeinholeMonsterExtensions);                      // Not yet implemented

    DEV_DEBUG_INFO("Extension::Print_CRCs(exit)\n");
}


/**
 *  Detaches this object from all active extension classes.
 * 
 *  @author: CCHyper
 */
void Extension::Detach_This_From_All(AbstractClass * target, bool all)
{
    //DEV_DEBUG_INFO("Extension::Detach_This_From_All(enter)\n");

    /**
     *  #NOTE: AnimClass and IsoTileTypeClass detach calls are disabled because they currently do nothing but take up a lot of performance.
     */

    /**
     *  #NOTE: The order of these calls must match the relevant RTTIType order!
     */
    Extension_Detach_This_From_All(UnitExtensions, target, all);
    Extension_Detach_This_From_All(AircraftExtensions, target, all);
    Extension_Detach_This_From_All(AircraftTypeExtensions, target, all);
    //Extension_Detach_This_From_All(AnimExtensions, target, all);
    Extension_Detach_This_From_All(AnimTypeExtensions, target, all);
    Extension_Detach_This_From_All(BuildingExtensions, target, all);
    Extension_Detach_This_From_All(BuildingTypeExtensions, target, all);
    //Extension_Detach_This_From_All(BulletExtensions, target, all);            // Not yet implemented
    Extension_Detach_This_From_All(BulletTypeExtensions, target, all);
    //Extension_Detach_This_From_All(CampaignExtensions, target, all);          // Does not need to be processed.
    //Extension_Detach_This_From_All(CellExtensions, target, all);              // Not yet implemented
    Extension_Detach_This_From_All(FactoryExtensions, target, all);
    Extension_Detach_This_From_All(HouseExtensions, target, all);
    Extension_Detach_This_From_All(HouseTypeExtensions, target, all);
    Extension_Detach_This_From_All(InfantryExtensions, target, all);
    Extension_Detach_This_From_All(InfantryTypeExtensions, target, all);
    //Extension_Detach_This_From_All(IsometricTileExtensions, target, all);     // Not yet implemented
    //Extension_Detach_This_From_All(IsometricTileTypeExtensions, target, all);
    //Extension_Detach_This_From_All(BuildingLightExtensions, target, all);     // Not yet implemented
    Extension_Detach_This_From_All(OverlayExtensions, target, all);
    Extension_Detach_This_From_All(OverlayTypeExtensions, target, all);
    //Extension_Detach_This_From_All(ParticleExtensions, target, all);          // Not yet implemented
    Extension_Detach_This_From_All(ParticleTypeExtensions, target, all);
    //Extension_Detach_This_From_All(ParticleSystemExtensions, target, all);    // Not yet implemented
    Extension_Detach_This_From_All(ParticleSystemTypeExtensions, target, all);
    //Extension_Detach_This_From_All(ScriptExtensions, target, all);            // Not yet implemented
    //Extension_Detach_This_From_All(ScriptTypeExtensions, target, all);        // Not yet implemented
    Extension_Detach_This_From_All(SideExtensions, target, all);
    Extension_Detach_This_From_All(SmudgeExtensions, target, all);
    Extension_Detach_This_From_All(SmudgeTypeExtensions, target, all);
    Extension_Detach_This_From_All(SuperWeaponTypeExtensions, target, all);
    //Extension_Detach_This_From_All(TaskForceExtensions, target, all);         // Not yet implemented
    //Extension_Detach_This_From_All(TeamExtensions, target, all);              // Not yet implemented
    //Extension_Detach_This_From_All(TeamTypeExtensions, target, all);          // Not yet implemented
    Extension_Detach_This_From_All(TerrainExtensions, target, all);
    Extension_Detach_This_From_All(TerrainTypeExtensions, target, all);
    //Extension_Detach_This_From_All(TriggerExtensions, target, all);           // Not yet implemented
    //Extension_Detach_This_From_All(TriggerTypeExtensions, target, all);       // Not yet implemented
    Extension_Detach_This_From_All(UnitTypeExtensions, target, all);
    //Extension_Detach_This_From_All(VoxelAnimExtensions, target, all);         // Not yet implemented
    Extension_Detach_This_From_All(VoxelAnimTypeExtensions, target, all);
    Extension_Detach_This_From_All(WaveExtensions, target, all);
    //Extension_Detach_This_From_All(TagExtensions, target, all);               // Not yet implemented
    //Extension_Detach_This_From_All(TagTypeExtensions, target, all);           // Not yet implemented
    Extension_Detach_This_From_All(TiberiumExtensions, target, all);
    //Extension_Detach_This_From_All(TActionExtensions, target, all);           // Not yet implemented
    //Extension_Detach_This_From_All(TEventExtensions, target, all);            // Not yet implemented
    Extension_Detach_This_From_All(WeaponTypeExtensions, target, all);
    Extension_Detach_This_From_All(WarheadTypeExtensions, target, all);
    //Extension_Detach_This_From_All(WaypointExtensions, target, all);          // Not yet implemented
    //Extension_Detach_This_From_All(TubeExtensions, target, all);              // Not yet implemented
    //Extension_Detach_This_From_All(LightSourceExtensions, target, all);       // Not yet implemented
    //Extension_Detach_This_From_All(EMPulseExtensions, target, all);           // Not yet implemented
    Extension_Detach_This_From_All(SuperExtensions, target, all);
    //Extension_Detach_This_From_All(AITriggerExtensions, target, all);         // Not yet implemented
    //Extension_Detach_This_From_All(AITriggerTypeExtensions, target, all);     // Not yet implemented
    //Extension_Detach_This_From_All(NeuronExtensions, target, all);            // Not yet implemented
    //Extension_Detach_This_From_All(FoggedObjectExtensions, target, all);      // Not yet implemented
    //Extension_Detach_This_From_All(AlphaShapeExtensions, target, all);        // Not yet implemented
    //Extension_Detach_This_From_All(VeinholeMonsterExtensions, target, all);   // Not yet implemented

    TacticalMapExtension->Detach(target, all);
    RuleExtension->Detach(target, all);
    ScenExtension->Detach(target, all);
    SessionExtension->Detach(target, all);

    //DEV_DEBUG_INFO("Extension::Detach_This_From_All(exit)\n");
}


/**
 *  Calculate the save game version.
 * 
 *  @author: CCHyper
 */
unsigned Extension::Get_Save_Version_Number()
{
    unsigned version = 0x100000;

    /**
     *  For debug builds, offset the save file version.
     */
#ifndef NDEBUG
    version += 0x10000000;
#endif

    /**
     *  Game classes.
     */
    version += sizeof(UnitClassExtension);
    version += sizeof(AircraftClassExtension);
    version += sizeof(AircraftTypeClassExtension);
    version += sizeof(AnimClassExtension);
    version += sizeof(AnimTypeClassExtension);
    version += sizeof(BuildingClassExtension);
    version += sizeof(BuildingTypeClassExtension);
    //version += sizeof(BulletClassExtension);                                  // Not yet implemented
    version += sizeof(BulletTypeClassExtension);
    version += sizeof(CampaignClassExtension);
    //version += sizeof(CellClassExtension);                                    // Not yet implemented
    version += sizeof(FactoryClassExtension);
    version += sizeof(HouseClassExtension);
    version += sizeof(HouseTypeClassExtension);
    version += sizeof(InfantryClassExtension);
    version += sizeof(InfantryTypeClassExtension);
    //version += sizeof(IsometricTileClassExtension);                           // Not yet implemented
    version += sizeof(IsometricTileTypeClassExtension);
    //version += sizeof(BuildingLightClassExtension);                           // Not yet implemented
    version += sizeof(OverlayClassExtension);
    version += sizeof(OverlayTypeClassExtension);
    //version += sizeof(ParticleClassExtension);                                // Not yet implemented
    version += sizeof(ParticleTypeClassExtension);
    //version += sizeof(ParticleSystemClassExtension);                          // Not yet implemented
    version += sizeof(ParticleSystemTypeClassExtension);
    //version += sizeof(ScriptClassExtension);                                  // Not yet implemented
    //version += sizeof(ScriptTypeClassExtension);                              // Not yet implemented
    version += sizeof(SideClassExtension);
    version += sizeof(SmudgeClassExtension);
    version += sizeof(SmudgeTypeClassExtension);
    version += sizeof(SuperWeaponTypeClassExtension);
    //version += sizeof(TaskForceClassExtension);                               // Not yet implemented
    //version += sizeof(TeamClassExtension);                                    // Not yet implemented
    //version += sizeof(TeamTypeClassExtension);                                // Not yet implemented
    version += sizeof(TerrainClassExtension);
    version += sizeof(TerrainTypeClassExtension);
    //version += sizeof(TriggerClassExtension);                                 // Not yet implemented
    //version += sizeof(TriggerTypeClassExtension);                             // Not yet implemented
    version += sizeof(UnitTypeClassExtension);
    //version += sizeof(VoxelAnimClassExtension);                               // Not yet implemented
    version += sizeof(VoxelAnimTypeClassExtension);
    version += sizeof(WaveClassExtension);
    //version += sizeof(TagClassExtension);                                     // Not yet implemented
    //version += sizeof(TagTypeClassExtension);                                 // Not yet implemented
    version += sizeof(TiberiumClassExtension);
    //version += sizeof(TActionClassExtension);                                 // Not yet implemented
    //version += sizeof(TEventClassExtension);                                  // Not yet implemented
    version += sizeof(WeaponTypeClassExtension);
    version += sizeof(WarheadTypeClassExtension);
    //version += sizeof(WaypointClassExtension);                                // Not yet implemented
    //version += sizeof(TubeClassExtension);                                    // Not yet implemented
    //version += sizeof(LightSourceClassExtension);                             // Not yet implemented
    //version += sizeof(EMPulseClassExtension);                                 // Not yet implemented
    version += sizeof(SuperClassExtension);
    //version += sizeof(AITriggerClassExtension);                               // Not yet implemented
    //version += sizeof(AITriggerTypeClassExtension);                           // Not yet implemented
    //version += sizeof(NeuronClassExtension);                                  // Not yet implemented
    //version += sizeof(FoggedObjectClassExtension);                            // Not yet implemented
    //version += sizeof(AlphaShapeClassExtension);                              // Not yet implemented
    //version += sizeof(VeinholeMonsterClassExtension);                         // Not yet implemented

    /**
     *  Global classes.
     */
    version += sizeof(TacticalExtension);                                       // We ignore the fact that Tactical is an abstract derived class, as we treat the extension as a global.
    version += sizeof(RulesClassExtension);
    version += sizeof(ScenarioClassExtension);
    version += sizeof(SidebarClassExtension);
    version += sizeof(SessionClassExtension);

    /**
     *  All other classes.
     */
    version += sizeof(ThemeControlExtension);
    version += sizeof(ArmorTypeClass);
    version += sizeof(RocketTypeClass);
    version += sizeof(SpawnManagerClass);
    version += sizeof(KamikazeTrackerClass);
    version += sizeof(AircraftTrackerClass);

    return version;
}
