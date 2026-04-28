/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extension interface global values.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "extension_globals.h"


DynamicVectorClass<UnitClassExtension *> UnitExtensions;
DynamicVectorClass<AircraftClassExtension *> AircraftExtensions;
DynamicVectorClass<AircraftTypeClassExtension *> AircraftTypeExtensions;
DynamicVectorClass<AnimClassExtension *> AnimExtensions;
DynamicVectorClass<AnimTypeClassExtension *> AnimTypeExtensions;
DynamicVectorClass<BuildingClassExtension *> BuildingExtensions;
DynamicVectorClass<BuildingTypeClassExtension *> BuildingTypeExtensions;
DynamicVectorClass<BulletTypeClassExtension *> BulletTypeExtensions;
DynamicVectorClass<CampaignClassExtension *> CampaignExtensions;
DynamicVectorClass<FactoryClassExtension *> FactoryExtensions;
DynamicVectorClass<SideClassExtension *> SideExtensions;
DynamicVectorClass<HouseClassExtension *> HouseExtensions;
DynamicVectorClass<HouseTypeClassExtension *> HouseTypeExtensions;
DynamicVectorClass<InfantryClassExtension *> InfantryExtensions;
DynamicVectorClass<InfantryTypeClassExtension *> InfantryTypeExtensions;
DynamicVectorClass<IsometricTileTypeClassExtension *> IsometricTileTypeExtensions;
DynamicVectorClass<OverlayClassExtension *> OverlayExtensions;
DynamicVectorClass<OverlayTypeClassExtension *> OverlayTypeExtensions;
DynamicVectorClass<ParticleSystemTypeClassExtension *> ParticleSystemTypeExtensions;
DynamicVectorClass<ParticleTypeClassExtension *> ParticleTypeExtensions;
DynamicVectorClass<SmudgeClassExtension *> SmudgeExtensions;
DynamicVectorClass<SmudgeTypeClassExtension *> SmudgeTypeExtensions;
DynamicVectorClass<SuperClassExtension *> SuperExtensions;
DynamicVectorClass<SuperWeaponTypeClassExtension *> SuperWeaponTypeExtensions;
DynamicVectorClass<TeamClassExtension*> TeamExtensions;
DynamicVectorClass<TeamTypeClassExtension*> TeamTypeExtensions;
DynamicVectorClass<TerrainClassExtension *> TerrainExtensions;
DynamicVectorClass<TerrainTypeClassExtension *> TerrainTypeExtensions;
DynamicVectorClass<TiberiumClassExtension *> TiberiumExtensions;
DynamicVectorClass<UnitTypeClassExtension *> UnitTypeExtensions;
DynamicVectorClass<VoxelAnimTypeClassExtension *> VoxelAnimTypeExtensions;
DynamicVectorClass<WarheadTypeClassExtension *> WarheadTypeExtensions;
DynamicVectorClass<WaveClassExtension *> WaveExtensions;
DynamicVectorClass<WeaponTypeClassExtension *> WeaponTypeExtensions;
DynamicVectorClass<TEventClassExtension*> TEventExtensions;
DynamicVectorClass<TActionClassExtension*> TActionExtensions;

TacticalExtension *TacticalMapExtension = nullptr;

RulesClassExtension *RuleExtension = nullptr;
ScenarioClassExtension *ScenExtension = nullptr;
SessionClassExtension *SessionExtension = nullptr;
OptionsClassExtension *OptionsExtension = nullptr;

DynamicVectorClass<ThemeControlExtension *> ThemeControlExtensions;
