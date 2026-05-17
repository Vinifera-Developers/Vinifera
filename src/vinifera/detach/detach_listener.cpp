/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  RTTI-based dispatcher for the type-indexed detach registry.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/
#include "detach_listener.h"

#include "abstract.h"
#include "aircraft.h"
#include "aitrigger.h"
#include "alphashape.h"
#include "anim.h"
#include "building.h"
#include "buildinglight.h"
#include "bullet.h"
#include "empulse.h"
#include "factory.h"
#include "foggedobject.h"
#include "foot.h"
#include "house.h"
#include "infantry.h"
#include "isotile.h"
#include "lightsource.h"
#include "mission.h"
#include "object.h"
#include "overlay.h"
#include "particle.h"
#include "particlesys.h"
#include "radio.h"
#include "script.h"
#include "smudge.h"
#include "super.h"
#include "taction.h"
#include "tag.h"
#include "team.h"
#include "techno.h"
#include "terrain.h"
#include "tevent.h"
#include "tibsun_defines.h"
#include "trigger.h"
#include "tube.h"
#include "unit.h"
#include "veinholemonster.h"
#include "voxelanim.h"
#include "wave.h"

#include <type_traits>


namespace Vinifera::Detach {


/**
 *  Every type that may be a detach target — i.e. anything a Listener<T> might
 *  legally watch. Includes both base classes and concrete leaves of the engine
 *  hierarchy.
 *
 *  Listing a type here costs nothing per call site: for each concrete dying
 *  type, the fold below only emits Registry<T>::Notify for T's actual
 *  ancestors (filtered at compile time via std::is_base_of). Unwatched types
 *  retain only a single empty static DynamicVectorClass.
 *
 *  Adding a Listener<T> elsewhere in the codebase requires no change here as
 *  long as T already appears below — which it should for any engine class.
 */
template<typename... Ts> struct Type_List {};

using Notifiable_Types = Type_List<
    /* Root */
    AbstractClass,

    /* ObjectClass hierarchy bases */
    ObjectClass, MissionClass, RadioClass, TechnoClass, FootClass,

    /* TechnoClass-derived */
    UnitClass, AircraftClass, InfantryClass, BuildingClass,

    /* ObjectClass-derived (non-Techno) */
    AnimClass, BulletClass, OverlayClass, ParticleClass, ParticleSystemClass,
    SmudgeClass, TerrainClass, VoxelAnimClass, WaveClass, IsometricTileClass,
    VeinholeMonsterClass, BuildingLightClass,

    /* AbstractClass-derived (non-Object) */
    FactoryClass, HouseClass, SuperClass, TeamClass, TriggerClass, TagClass,
    ScriptClass, AITriggerClass, TubeClass, TActionClass, TEventClass,
    LightSourceClass, EMPulseClass, FoggedObjectClass, AlphaShapeClass
>;


/**
 *  For each T in the typelist, emit Registry<T>::Notify if the concrete
 *  dying type IS-A T. The if constexpr discards the rest at compile time.
 */
template<typename Concrete, typename... Ts>
static inline void Notify_Bases(Type_List<Ts...>, AbstractClass* t, bool all)
{
    auto visit = [&]<typename T>() {
        if constexpr (std::is_base_of_v<T, Concrete>) {
            Registry<T>::Notify(static_cast<T*>(t), all);
        }
    };
    (visit.template operator()<Ts>(), ...);
}


template<typename Concrete>
static inline void Notify_Chain(AbstractClass* t, bool all)
{
    Notify_Bases<Concrete>(Notifiable_Types{}, t, all);
}

void Notify_Abstract(AbstractClass* t, bool all)
{
    if (t == nullptr) return;

    switch (t->RTTI) {
    case RTTI_UNIT:
        Notify_Chain<UnitClass>(t, all);
        return;
    case RTTI_AIRCRAFT:
        Notify_Chain<AircraftClass>(t, all);
        return;
    case RTTI_INFANTRY:
        Notify_Chain<InfantryClass>(t, all);
        return;
    case RTTI_BUILDING:
        Notify_Chain<BuildingClass>(t, all);
        return;
    case RTTI_ANIM:
        Notify_Chain<AnimClass>(t, all);
        return;
    case RTTI_BULLET:
        Notify_Chain<BulletClass>(t, all);
        return;
    case RTTI_OVERLAY:
        Notify_Chain<OverlayClass>(t, all);
        return;
    case RTTI_PARTICLE:
        Notify_Chain<ParticleClass>(t, all);
        return;
    case RTTI_PARTICLESYSTEM:
        Notify_Chain<ParticleSystemClass>(t, all);
        return;
    case RTTI_SMUDGE:
        Notify_Chain<SmudgeClass>(t, all);
        return;
    case RTTI_TERRAIN:
        Notify_Chain<TerrainClass>(t, all);
        return;
    case RTTI_VOXELANIM:
        Notify_Chain<VoxelAnimClass>(t, all);
        return;
    case RTTI_WAVE:
        Notify_Chain<WaveClass>(t, all);
        return;
    case RTTI_ISOTILE:
        Notify_Chain<IsometricTileClass>(t, all);
        return;
    case RTTI_VEINHOLEMONSTER:
        Notify_Chain<VeinholeMonsterClass>(t, all);
        return;
    case RTTI_LIGHT:
        Notify_Chain<BuildingLightClass>(t, all);
        return;
    case RTTI_FACTORY:
        Notify_Chain<FactoryClass>(t, all);
        return;
    case RTTI_HOUSE:
        Notify_Chain<HouseClass>(t, all);
        return;
    case RTTI_SUPERWEAPON:
        Notify_Chain<SuperClass>(t, all);
        return;
    case RTTI_TEAM:
        Notify_Chain<TeamClass>(t, all);
        return;
    case RTTI_TRIGGER:
        Notify_Chain<TriggerClass>(t, all);
        return;
    case RTTI_TAG:
        Notify_Chain<TagClass>(t, all);
        return;
    case RTTI_SCRIPT:
        Notify_Chain<ScriptClass>(t, all);
        return;
    case RTTI_AITRIGGER:
        Notify_Chain<AITriggerClass>(t, all);
        return;
    case RTTI_TUBE:
        Notify_Chain<TubeClass>(t, all);
        return;
    case RTTI_ACTION:
        Notify_Chain<TActionClass>(t, all);
        return;
    case RTTI_EVENT:
        Notify_Chain<TEventClass>(t, all);
        return;
    case RTTI_LIGHTSOURCE:
        Notify_Chain<LightSourceClass>(t, all);
        return;
    case RTTI_EMPULSE:
        Notify_Chain<EMPulseClass>(t, all);
        return;
    case RTTI_FOGGEDOBJECT:
        Notify_Chain<FoggedObjectClass>(t, all);
        return;
    case RTTI_ALPHASHAPE:
        Notify_Chain<AlphaShapeClass>(t, all);
        return;

    default:
        /**
         *  Unmapped RTTI (type classes, waypoints, etc.). Still notify the
         *  AbstractClass-wide listeners so generic AbstractClass* fields get
         *  cleared. Concrete-type listeners can't apply because we don't
         *  have a downcast target.
         */
        Registry<AbstractClass>::Notify(t, all);
        return;
    }
}


} // namespace Vinifera::Detach
