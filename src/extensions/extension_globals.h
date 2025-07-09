/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          EXTENSION_GLOBALS.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extension interface global values.
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

#include "always.h"
#include "vinifera_defines.h"
#include "debughandler.h"
#include "asserthandler.h"


class AbstractClass;
class AbstractClassExtension;

class AircraftClassExtension;
class AircraftTypeClassExtension;
class AnimClassExtension;
class AnimTypeClassExtension;
class BuildingClassExtension;
class BuildingTypeClassExtension;
class BulletTypeClassExtension;
class CampaignClassExtension;
class FactoryClassExtension;
class SideClassExtension;
class HouseClassExtension;
class HouseTypeClassExtension;
class InfantryClassExtension;
class InfantryTypeClassExtension;
class IsometricTileTypeClassExtension;
class OverlayClassExtension;
class OverlayTypeClassExtension;
class ParticleSystemTypeClassExtension;
class ParticleTypeClassExtension;
class SmudgeClassExtension;
class SmudgeTypeClassExtension;
class SuperClassExtension;
class SuperWeaponTypeClassExtension;
class TerrainClassExtension;
class TerrainTypeClassExtension;
class TiberiumClassExtension;
class UnitClassExtension;
class UnitTypeClassExtension;
class VoxelAnimTypeClassExtension;
class WarheadTypeClassExtension;
class WaveClassExtension;
class WeaponTypeClassExtension;

class TacticalExtension;

class RulesClassExtension;
class ScenarioClassExtension;
class SidebarClassExtension;
class SessionClassExtension;
class OptionsClassExtension;

class ThemeControlExtension;


/**
 *  For printing out extension debug info.
 */
#ifdef VINIFERA_ENABLE_EXTENSION_DEBUG_PRINTING
#define EXT_DEBUG_SAY(x, ...) DEV_DEBUG_SAY(x, ##__VA_ARGS__)
#define EXT_DEBUG_INFO(x, ...) DEV_DEBUG_INFO(x, ##__VA_ARGS__)
#define EXT_DEBUG_WARNING(x, ...) DEV_DEBUG_WARNING(x, ##__VA_ARGS__)
#define EXT_DEBUG_ERROR(x, ...) DEV_DEBUG_ERROR(x, ##__VA_ARGS__)
#define EXT_DEBUG_FATAL(x, ...) DEV_DEBUG_FATAL(x, ##__VA_ARGS__)
#define EXT_DEBUG_TRACE(x, ...) DEV_DEBUG_TRACE(x, ##__VA_ARGS__)
#else
#define EXT_DEBUG_SAY(x, ...) ((void)0)
#define EXT_DEBUG_INFO(x, ...) ((void)0)
#define EXT_DEBUG_WARNING(x, ...) ((void)0)
#define EXT_DEBUG_ERROR(x, ...) ((void)0)
#define EXT_DEBUG_FATAL(x, ...) ((void)0)
#define EXT_DEBUG_TRACE(x, ...) ((void)0)
#endif


/**
 *  Abstract derived classes.
 */
extern DynamicVectorClass<UnitClassExtension *> UnitExtensions;
extern DynamicVectorClass<AircraftClassExtension *> AircraftExtensions;
extern DynamicVectorClass<AircraftTypeClassExtension *> AircraftTypeExtensions;
extern DynamicVectorClass<AnimClassExtension *> AnimExtensions;
extern DynamicVectorClass<AnimTypeClassExtension *> AnimTypeExtensions;
extern DynamicVectorClass<BuildingClassExtension *> BuildingExtensions;
extern DynamicVectorClass<BuildingTypeClassExtension *> BuildingTypeExtensions;
extern DynamicVectorClass<BulletTypeClassExtension *> BulletTypeExtensions;
extern DynamicVectorClass<CampaignClassExtension *> CampaignExtensions;
extern DynamicVectorClass<FactoryClassExtension *> FactoryExtensions;
extern DynamicVectorClass<SideClassExtension *> SideExtensions;
extern DynamicVectorClass<HouseClassExtension *> HouseExtensions;
extern DynamicVectorClass<HouseTypeClassExtension *> HouseTypeExtensions;
extern DynamicVectorClass<InfantryClassExtension *> InfantryExtensions;
extern DynamicVectorClass<InfantryTypeClassExtension *> InfantryTypeExtensions;
extern DynamicVectorClass<IsometricTileTypeClassExtension *> IsometricTileTypeExtensions;
extern DynamicVectorClass<OverlayClassExtension *> OverlayExtensions;
extern DynamicVectorClass<OverlayTypeClassExtension *> OverlayTypeExtensions;
extern DynamicVectorClass<ParticleSystemTypeClassExtension *> ParticleSystemTypeExtensions;
extern DynamicVectorClass<ParticleTypeClassExtension *> ParticleTypeExtensions;
extern DynamicVectorClass<SmudgeClassExtension *> SmudgeExtensions;
extern DynamicVectorClass<SmudgeTypeClassExtension *> SmudgeTypeExtensions;
extern DynamicVectorClass<SuperClassExtension *> SuperExtensions;
extern DynamicVectorClass<SuperWeaponTypeClassExtension *> SuperWeaponTypeExtensions;
extern DynamicVectorClass<TerrainClassExtension *> TerrainExtensions;
extern DynamicVectorClass<TerrainTypeClassExtension *> TerrainTypeExtensions;
extern DynamicVectorClass<TiberiumClassExtension *> TiberiumExtensions;
extern DynamicVectorClass<UnitTypeClassExtension *> UnitTypeExtensions;
extern DynamicVectorClass<VoxelAnimTypeClassExtension *> VoxelAnimTypeExtensions;
extern DynamicVectorClass<WarheadTypeClassExtension *> WarheadTypeExtensions;
extern DynamicVectorClass<WaveClassExtension *> WaveExtensions;
extern DynamicVectorClass<WeaponTypeClassExtension *> WeaponTypeExtensions;

/**
 *  Abstract derived classes, but only a single instance is required.
 */
extern TacticalExtension *TacticalMapExtension;

/**
 *  Global classes that are not abstract derived.
 */
extern RulesClassExtension *RuleExtension;
extern ScenarioClassExtension *ScenExtension;
extern SidebarClassExtension* SidebarExtension;
extern SessionClassExtension *SessionExtension;
extern OptionsClassExtension *OptionsExtension;

/**
 *  Classes that require a list, but are not abstract derived.
 */
extern DynamicVectorClass<ThemeControlExtension *> ThemeControlExtensions;
