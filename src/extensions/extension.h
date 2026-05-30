/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  The file contains the functions required for the extension system.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once


#include "abstract.h"
#include "abstractext.h"
#include "asserthandler.h"
#include "extension_globals.h"
#include "swizzle.h"
#include "vinifera_defines.h"

#include <string>
#include <typeinfo>
#include <unknwn.h> // for IStream


class EventClass;
class CRCEngine;

// Abstract classes
class AbstractClass;
class AbstractTypeClass;
class ObjectClass;
class ObjectTypeClass;
class RadioClass;
class MissionClass;
class TechnoClass;
class TechnoTypeClass;
class FootClass;

// Final classes
class UnitClass;
class AircraftClass;
class AircraftTypeClass;
class AnimClass;
class AnimTypeClass;
class BuildingClass;
class BuildingTypeClass;
class BulletClass;
class BulletTypeClass;
class CampaignClass;
class CellClass;
class FactoryClass;
class HouseClass;
class HouseTypeClass;
class InfantryClass;
class InfantryTypeClass;
class IsometricTileClass;
class IsometricTileTypeClass;
class BuildingLightClass;
class OverlayClass;
class OverlayTypeClass;
class ParticleClass;
class ParticleTypeClass;
class ParticleSystemClass;
class ParticleSystemTypeClass;
class ScriptClass;
class ScriptTypeClass;
class SideClass;
class SmudgeClass;
class SmudgeTypeClass;
class SuperWeaponTypeClass;
class TaskForceClass;
class TeamClass;
class TeamTypeClass;
class TerrainClass;
class TerrainTypeClass;
class TriggerClass;
class TriggerTypeClass;
class UnitTypeClass;
class VoxelAnimClass;
class VoxelAnimTypeClass;
class WaveClass;
class TagClass;
class TagTypeClass;
class TiberiumClass;
class TActionClass;
class TEventClass;
class WeaponTypeClass;
class WarheadTypeClass;
class WaypointClass;
class TubeClass;
class LightSourceClass;
class EMPulseClass;
class SuperClass;
class AITriggerClass;
class AITriggerTypeClass;
class NeuronClass;
class FoggedObjectClass;
class AlphaShapeClass;
class VeinholeMonsterClass;

// Abstract class extensions
class AbstractClassExtension;
class ObjectClassExtension;
class RadioClassExtension;
class MissionClassExtension;
class TechnoClassExtension;
class FootClassExtension;
class AbstractTypeClassExtension;
class ObjectTypeClassExtension;
class TechnoTypeClassExtension;

// Final class extensions
class UnitClassExtension;
class AircraftClassExtension;
class AircraftTypeClassExtension;
class AnimClassExtension;
class AnimTypeClassExtension;
class BuildingClassExtension;
class BuildingTypeClassExtension;
// class BulletClassExtension;
class BulletTypeClassExtension;
class CampaignClassExtension;
// class CellClassExtension;
class FactoryClassExtension;
class HouseClassExtension;
class HouseTypeClassExtension;
class InfantryClassExtension;
class InfantryTypeClassExtension;
// class IsometricTileClassExtension;
class IsometricTileTypeClassExtension;
// class BuildingLightClassExtension;
class OverlayClassExtension;
class OverlayTypeClassExtension;
// class ParticleClassExtension;
class ParticleTypeClassExtension;
// class ParticleSystemClassExtension;
class ParticleSystemTypeClassExtension;
// class ScriptClassExtension;
// class ScriptTypeClassExtension;
class SideClassExtension;
class SmudgeClassExtension;
class SmudgeTypeClassExtension;
class SuperWeaponTypeClassExtension;
// class TaskForceClassExtension;
// class TeamClassExtension;
// class TeamTypeClassExtension;
class TerrainClassExtension;
class TerrainTypeClassExtension;
// class TriggerClassExtension;
// class TriggerTypeClassExtension;
class UnitTypeClassExtension;
// class VoxelAnimClassExtension;
class VoxelAnimTypeClassExtension;
class WaveClassExtension;
// class TagClassExtension;
// class TagTypeClassExtension;
class TiberiumClassExtension;
class TActionClassExtension;
// class TEventClassExtension;
class WeaponTypeClassExtension;
class WarheadTypeClassExtension;
// class WaypointClassExtension;
// class TubeClassExtension;
// class LightSourceClassExtension;
// class EMPulseClassExtension;
class SuperClassExtension;
// class AITriggerClassExtension;
// class AITriggerTypeClassExtension;
// class NeuronClassExtension;
// class FoggedObjectClassExtension;
// class AlphaShapeClassExtension;
// class VeinholeMonsterClassExtension;


namespace Extension
{

/**
 *  Do not call these directly! Use the template functions below.
 */
namespace Private
{

bool Is_Supported(const AbstractClass* abstract);
AbstractClassExtension* Make_Internal(const AbstractClass* abstract);
bool Destroy_Internal(const AbstractClass *abstract);
AbstractClassExtension *Fetch_Internal(const AbstractClass *abstract);

}; // namespace "Extension::Private".

namespace Utility
{

/**
 *  Erase First Occurrence of given substring from main string.
 * 
 *  @author: CCHyper
 */
inline void Erase_Sub_String(std::string &str, const std::string &erase)
{
    /**
     *  Search for the substring in string.
     */
    size_t pos = str.find(erase);
    if (pos != std::string::npos) {

        /**
         *  If found then erase it from string.
         */
        str.erase(pos, erase.length());
    }
}

/**
 *  Wrapper for "typeid(T).name()", removes the "class" or "struct" prefix on the string.
 * 
 *  @author: CCHyper
 */
template<typename T>
std::string Get_TypeID_Name()
{
    std::string str = typeid(T).name();
    Erase_Sub_String(str, "class ");
    Erase_Sub_String(str, "struct ");
    return str;
}

static std::string Get_TypeID_Name(const AbstractClass *abstract)
{
    std::string str = typeid(*abstract).name();
    Erase_Sub_String(str, "class ");
    return str;
}

static std::string Get_TypeID_Name(const AbstractClassExtension *abstract_ext)
{
    std::string str = typeid(*abstract_ext).name();
    Erase_Sub_String(str, "class ");
    return str;
}

}; // namespace "Extension::Utility"

namespace Singleton
{

/**
 *  Create an isntance of the singleton class.
 * 
 *  @author: CCHyper
 */
template<class BASE_CLASS, class EXT_CLASS>
EXT_CLASS *Make(const BASE_CLASS *base)
{
    ASSERT(base != nullptr);

    EXT_CLASS *ext_ptr = new EXT_CLASS(base);
    ASSERT(ext_ptr != nullptr);

    return ext_ptr;
}

/**
 *  Destroy an instance of the singleton class.
 * 
 *  @author: CCHyper
 */
template<class BASE_CLASS, class EXT_CLASS>
void Destroy(const EXT_CLASS *ext)
{
    ASSERT(ext != nullptr);

    delete ext;

}

}; // namespace "Extension::Singleton".

namespace List
{
/**
 *  Fetch an extension instance from a list whose extension pointer points to the base class.
 * 
 *  @author: CCHyper
 */
template<class BASE_CLASS, class EXT_CLASS>
EXT_CLASS *Fetch(const BASE_CLASS *base, DynamicVectorClass<EXT_CLASS *> &list)
{
    ASSERT(base != nullptr);

    for (int index = 0; index < list.Count(); ++index) {
        EXT_CLASS * ext = list[index];
        if (list[index]->This() == base) {
            return ext;
        }
    }

    return nullptr;
}

/**
 *  Creation an instance of the extension class and add it to the list.
 * 
 *  @author: CCHyper
 */
template<class BASE_CLASS, class EXT_CLASS>
EXT_CLASS *Make(const BASE_CLASS *base, DynamicVectorClass<EXT_CLASS *> &list)
{
    ASSERT(base != nullptr);

    EXT_CLASS *ext_ptr = new EXT_CLASS(base);
    ASSERT(ext_ptr != nullptr);

    list.Add(ext_ptr);

    return ext_ptr;
}

/**
 *  Destroy an instance of the extension and remove it from the list.
 * 
 *  @author: CCHyper
 */
template<class BASE_CLASS, class EXT_CLASS>
void Destroy(const BASE_CLASS *base, DynamicVectorClass<EXT_CLASS *> &list)
{
    ASSERT(base != nullptr);

    for (int index = 0; index < list.Count(); ++index) {
        EXT_CLASS * ext = list[index].This();
        if (ext->This() == base) {
            delete ext;
            return;
        }
    }

}

}; // namespace "Extension::List".


/**
 *  Trait mapping a base class to its corresponding extension class.
 *  Specialize this for each supported base type.
 */
template<typename BASE> struct Extension_Pair;

#define MAKE_EXTENSION_PAIR(base) \
    template<> struct Extension_Pair<base> { using Type = base##Extension; };

MAKE_EXTENSION_PAIR(AbstractClass);
MAKE_EXTENSION_PAIR(ObjectClass);
MAKE_EXTENSION_PAIR(RadioClass);
MAKE_EXTENSION_PAIR(MissionClass);
MAKE_EXTENSION_PAIR(TechnoClass);
MAKE_EXTENSION_PAIR(FootClass);
MAKE_EXTENSION_PAIR(AbstractTypeClass);
MAKE_EXTENSION_PAIR(ObjectTypeClass);
MAKE_EXTENSION_PAIR(TechnoTypeClass);
MAKE_EXTENSION_PAIR(UnitClass);
MAKE_EXTENSION_PAIR(AircraftClass);
MAKE_EXTENSION_PAIR(AircraftTypeClass);
MAKE_EXTENSION_PAIR(AnimClass);
MAKE_EXTENSION_PAIR(AnimTypeClass);
MAKE_EXTENSION_PAIR(BuildingClass);
MAKE_EXTENSION_PAIR(BuildingTypeClass);
//MAKE_EXTENSION_PAIR(BulletClass);                                     // Not yet implemented
MAKE_EXTENSION_PAIR(BulletTypeClass);
MAKE_EXTENSION_PAIR(CampaignClass);
//MAKE_EXTENSION_PAIR(CellClass);                                       // Not yet implemented
MAKE_EXTENSION_PAIR(FactoryClass);
MAKE_EXTENSION_PAIR(HouseClass);
MAKE_EXTENSION_PAIR(HouseTypeClass);
MAKE_EXTENSION_PAIR(InfantryClass);
MAKE_EXTENSION_PAIR(InfantryTypeClass);
//MAKE_EXTENSION_PAIR(IsometricTileClass);                              // Not yet implemented
MAKE_EXTENSION_PAIR(IsometricTileTypeClass);
//MAKE_EXTENSION_PAIR(BuildingLightClass);                              // Not yet implemented
MAKE_EXTENSION_PAIR(OverlayClass);
MAKE_EXTENSION_PAIR(OverlayTypeClass);
//MAKE_EXTENSION_PAIR(ParticleClass);                                   // Not yet implemented
MAKE_EXTENSION_PAIR(ParticleTypeClass);
//MAKE_EXTENSION_PAIR(ParticleSystemClass);                             // Not yet implemented
MAKE_EXTENSION_PAIR(ParticleSystemTypeClass);
//MAKE_EXTENSION_PAIR(ScriptClass);                                     // Not yet implemented
//MAKE_EXTENSION_PAIR(ScriptTypeClass);                                 // Not yet implemented
MAKE_EXTENSION_PAIR(SideClass);
MAKE_EXTENSION_PAIR(SmudgeClass);
MAKE_EXTENSION_PAIR(SmudgeTypeClass);
MAKE_EXTENSION_PAIR(SuperWeaponTypeClass);
//MAKE_EXTENSION_PAIR(TaskForceClass);                                  // Not yet implemented
MAKE_EXTENSION_PAIR(TeamClass);
MAKE_EXTENSION_PAIR(TeamTypeClass);
MAKE_EXTENSION_PAIR(TerrainClass);
MAKE_EXTENSION_PAIR(TerrainTypeClass);
//MAKE_EXTENSION_PAIR(TriggerClass);                                    // Not yet implemented
//MAKE_EXTENSION_PAIR(TriggerTypeClass);                                // Not yet implemented
MAKE_EXTENSION_PAIR(UnitTypeClass);
//MAKE_EXTENSION_PAIR(VoxelAnimClass);                                  // Not yet implemented
MAKE_EXTENSION_PAIR(VoxelAnimTypeClass);
MAKE_EXTENSION_PAIR(WaveClass);
//MAKE_EXTENSION_PAIR(TagClass);                                        // Not yet implemented
//MAKE_EXTENSION_PAIR(TagTypeClass);                                    // Not yet implemented
MAKE_EXTENSION_PAIR(TiberiumClass);
MAKE_EXTENSION_PAIR(TActionClass);
//MAKE_EXTENSION_PAIR(TEventClass);                                     // Not yet implemented
MAKE_EXTENSION_PAIR(WeaponTypeClass);
MAKE_EXTENSION_PAIR(WarheadTypeClass);
//MAKE_EXTENSION_PAIR(WaypointClass);                                   // Not yet implemented
//MAKE_EXTENSION_PAIR(TubeClass);                                       // Not yet implemented
//MAKE_EXTENSION_PAIR(LightSourceClass);                                // Not yet implemented
//MAKE_EXTENSION_PAIR(EMPulseClass);                                    // Not yet implemented
MAKE_EXTENSION_PAIR(SuperClass);
//MAKE_EXTENSION_PAIR(AITriggerClass);                                  // Not yet implemented
//MAKE_EXTENSION_PAIR(AITriggerTypeClass);                              // Not yet implemented
//MAKE_EXTENSION_PAIR(NeuronClass);                                     // Not yet implemented
//MAKE_EXTENSION_PAIR(FoggedObjectClass);                               // Not yet implemented
//MAKE_EXTENSION_PAIR(AlphaShapeClass);                                 // Not yet implemented
//MAKE_EXTENSION_PAIR(VeinholeMonsterClass);                            // Not yet implemented

/**
 *  Macro to declare a fake extension class that also
 *  registers it as a pair with the extension, so that
 *  the extension can be fetched without having to cast it
 *  to the original class.
 */
#define DECLARE_EXTENDING_CLASS_AND_PAIR(BASE) \
    class BASE##Ext;                           \
    class BASE##Extension;                     \
    namespace Extension {                      \
        template<>                             \
        struct Extension_Pair<BASE##Ext> {     \
            using Type = BASE##Extension;      \
        };                                     \
    }                                          \
    class BASE##Ext : public BASE

/**
 *  Fetch the extension instance linked to this abstract object. 
 * 
 *  @author: CCHyper, ZivDero
 */
template<typename BASE>
typename Extension_Pair<typename std::remove_cv<BASE>::type>::Type*
Fetch(BASE* base_ptr)
{
    using BareBase = typename std::remove_cv<BASE>::type;

    static_assert(std::is_base_of<AbstractClass, BareBase>::value,
        "BASE must derive from AbstractClass");

    using EXT = typename Extension_Pair<BareBase>::Type;
    return static_cast<EXT*>(Extension::Private::Fetch_Internal(base_ptr));
}

/**
 *  Create an instance of the extension class and link it to the abstract object.
 * 
 *  @author: CCHyper
 */
template<class EXT_CLASS>
EXT_CLASS *Make(const AbstractClass *abstract)
{
    ASSERT(abstract != nullptr);

    return (EXT_CLASS *)Extension::Private::Make_Internal(abstract);
}

/**
 *  Destory an instance of the extension class linked to this abstract object.
 * 
 *  @author: CCHyper
 */
template<class EXT_CLASS>
void Destroy(const AbstractClass *abstract)
{
    ASSERT(abstract != nullptr);

    Extension::Private::Destroy_Internal(abstract);
}

/**
 *  Save and load interface.
 */
bool Save(IStream *pStm);
bool Load(IStream *pStm);
bool Request_Pointer_Remap();
unsigned Get_Save_Version_Number();

/**
 *  Announcement and debugging functions.
 */
bool Register_Class_Factories();
void Free_Heaps();
void Print_CRCs(EventClass *ev);
void Print_CRCs(FILE *fp, EventClass *ev);

}; // namespace "Extension".


/**
 * 
 *  Base class for all global extension classes.
 * 
 */
template<class T>
class GlobalExtensionClass
{
    public:
        STDMETHOD(Load)(IStream *pStm);
        STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

    public:
        GlobalExtensionClass(const T *this_ptr = nullptr);
        GlobalExtensionClass(const NoInitClass &noinit);
        virtual ~GlobalExtensionClass();

        /**
         *  Return the raw size of class data for save/load purposes.
         *  
         *  @note: This must be overridden by the extended class!
         */
        virtual int Get_Object_Size() const = 0;

        /**
         *  Compute a unique crc value for this instance.
         *
         *  @note: This must be overridden by the extended class!
         */
        virtual void Object_CRC(CRCEngine &crc) const = 0;

        /**
         *  Access to the class instance we extend.
         */
        virtual T *This() const { return const_cast<T *>(ThisPtr); }
        virtual const T *This_Const() const { return ThisPtr; }

        /**
         *  Assign the class instance that we extend.
         */
        virtual void Assign_This(const T *this_ptr) { ASSERT(this_ptr != nullptr); ThisPtr = this_ptr; }

        /**
         *  Returns the name of this object type.
         *  
         *  @note: This must be overridden by the extended class!
         */
        virtual const char *Name() const = 0;

        /**
         *  Returns the full name of this object type.
         *  
         *  @note: This must be overridden by the extended class!
         */
        virtual const char *Full_Name() const = 0;

    private:
        /**
         *  Pointer to the class we are extending. This provides us with a way of
         *  quickly referencing the base class without doing a look-up each time.
         */
        const T *ThisPtr;

    private:
        GlobalExtensionClass(const GlobalExtensionClass &) = delete;
        void operator = (const GlobalExtensionClass &) = delete;

    public:
};


/**
 *  Class constructor
 * 
 *  @author: CCHyper
 */
template<class T>
GlobalExtensionClass<T>::GlobalExtensionClass(const T *this_ptr) :
    ThisPtr(this_ptr)
{
    //ASSERT(ThisPtr != nullptr);      // NULL ThisPtr is valid when performing a Load state operation.
}


/**
 *  Class no-init constructor.
 * 
 *  @author: CCHyper
 */
template<class T>
GlobalExtensionClass<T>::GlobalExtensionClass(const NoInitClass &noinit)
{
}


/**
 *  Class destructor
 * 
 *  @author: CCHyper
 */
template<class T>
GlobalExtensionClass<T>::~GlobalExtensionClass()
{
    ThisPtr = nullptr;
}


/**
 *  Loads the object from the stream and requests a new pointer to
 *  the class we extended post-load.
 * 
 *  As singleton is static data, we do not need to request
 *  pointer remap of "ThisPtr" after loading has finished.
 * 
 *  @author: CCHyper, tomsons26
 */
template<class T>
HRESULT GlobalExtensionClass<T>::Load(IStream *pStm)
{
    if (!pStm) {
        return E_POINTER;
    }

    HRESULT hr;

    /**
     *  Read this class's binary blob data directly into this instance.
     */
    hr = pStm->Read(this, Get_Object_Size(), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Saves the object to the stream.
 * 
 *  @author: CCHyper, tomsons26
 */
template<class T>
HRESULT GlobalExtensionClass<T>::Save(IStream *pStm, BOOL fClearDirty)
{
    if (!pStm) {
        return E_POINTER;
    }

    HRESULT hr;
    
    /**
     *  Write this class instance as a binary blob.
     */
    hr = pStm->Write(this, Get_Object_Size(), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}
