/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended CellClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "cellext_hooks.h"

#include "asserthandler.h"
#include "building.h"
#include "buildingtype.h"
#include "buildingtypeext.h"
#include "cellext.h"
#include "cellext_const.h"
#include "extension.h"
#include "foot.h"
#include "hooker.h"
#include "house.h"
#include "iomap.h"
#include "isotiletype.h"
#include "isotiletypeext.h"
#include "overlaytype.h"
#include "overlaytypeext.h"
#include "rules.h"
#include "rulesext.h"
#include "session.h"
#include "syringe.h"
#include "tactical.h"
#include "techno.h"
#include "technotype.h"
#include "terrain.h"
#include "terraintype.h"
#include "theatertype.h"
#include "tiberium.h"
#include "tiberiumext.h"
#include "tibsun_globals.h"

/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static DECLARE_EXTENDING_CLASS_AND_PAIR(CellClass)
{
public:
    bool _Can_Tiberium_Germinate(TiberiumClass const* tiberium) const;
    bool _Can_Tiberium_Spread();
    bool _Can_Place_Veins() const;
    bool _Spread_Tiberium(bool forced);
    void _Recalc_Passability();
    int _Reduce_Tiberium(int levels);
};


/**
 *  Re-implementation of CellClass::Can_Tiberium_Germinate.
 *
 *  @author: ZivDero
 */
bool CellClassExt::_Can_Tiberium_Germinate(TiberiumClass const* tiberium) const
{
    if (!Map.In_Local_Radar(CellID, true)) return false;

    if (IsUnderBridge || WasUnderBridge) return false;

    /*
    **  Don't allow Tiberium to grow on a cell with a building unless that building is
    **  invisible. In such a case, the Tiberium must grow or else the location of the
    **  building will be revealed.
    */
    BuildingClass const* building = Cell_Building();
    if (building != nullptr && building->Strength > 0 && !building->Class->IsInvisible && !building->Class->IsInvisibleInGame) return false;

    TerrainClass* terrain = Cell_Terrain();
    if (terrain != nullptr && terrain->Class->IsTiberiumSpawn) return false;

    if (!Ground[Land_Type()].Build) return false;

    if (Overlay != OVERLAY_NONE) return false;

    if (Ramp > RAMP_SOUTH || (Ramp != RAMP_NONE && tiberium != nullptr && tiberium->RampVariety == 0)) return false;

    if (ITType >= ISOTILE_FIRST && ITType < IsoTileTypes.Count()) {

        IsometricTileTypeClass* ittype = IsoTileTypes[ITType];
        if (!ittype->IsAllowTiberium) return false;

        auto ittype_ext = Extension::Fetch(ittype);

        if (tiberium != nullptr && ittype_ext->AllowedTiberiums.Count() > 0 && !ittype_ext->AllowedTiberiums.Is_Present(tiberium->HeapID)) return false;
    }

    return true;
}


/**
 *  Re-implementation of CellClass::Can_Tiberium_Spread.
 *
 *  @author: ZivDero, tomsons26
 */
bool CellClassExt::_Can_Tiberium_Spread()
{
    if (!Scen->Special.IsTSpread) return false;

    TiberiumType tiberium = Tiberium_Type_Here();

    if (tiberium == TIBERIUM_NONE) return false;

    /**
     *  Use the new setting as the minimum stage to spread.
     */
    if (OverlayData < Extension::Fetch(Tiberiums[tiberium])->MinSpreadStage) return false;

    if (Tiberiums[tiberium]->SpreadPercentage < 0.00001) return false;

    if (Cell_Occupier() != nullptr) return false;

    return true;
}


/**
 *  Re-implementation of CellClass::Can_Place_Veins.
 *
 *  @author: ZivDero
 */
bool CellClassExt::_Can_Place_Veins() const
{
    if (Ramp <= RAMP_SOUTH) {
        if (Land != LAND_WATER && Land != LAND_ROCK && Land != LAND_ICE && Land != LAND_BEACH) {
            if (Overlay == OVERLAY_NONE || OverlayTypes[Overlay]->IsVeins) {
                IsometricTileType ittype = ITType;
                if (ITType < ISOTILE_FIRST || ITType >= IsoTileTypes.Count()) {
                    ittype = ISOTILE_FIRST;
                }
                if (Extension::Fetch(IsoTileTypes[ittype])->IsAllowVeins) {
                    for (int dir = FACING_N; dir < FACING_COUNT; dir += 2) {
                        CellClass const& adjacent = Adjacent_Cell(static_cast<FacingType>(dir));
                        if (adjacent.Ramp > RAMP_SOUTH && Ramp == RAMP_NONE) {
                            if (adjacent.Overlay == OVERLAY_NONE || !OverlayTypes[adjacent.Overlay]->IsVeins) {
                                return false;
                            }
                        }
                        if (adjacent.Land == LAND_WATER || adjacent.Land == LAND_ROCK || adjacent.Land == LAND_ICE || adjacent.Land == LAND_BEACH) {
                            return false;
                        }
                        if (adjacent.Overlay != OVERLAY_NONE && !OverlayTypes[adjacent.Overlay]->IsVeins) {
                            return false;
                        }
                    }
                    return true;
                }
            }
        }
    }
    return false;
}


/**
 *  Proxy for CellClassExt::Spread_Tiberium.
 *
 *  @author: ZivDero
 */
bool CellClassExt::_Spread_Tiberium(bool forced)
{
    return CellClassExtension::Spread_Tiberium(this, forced);
}


/**
 *  Re-implementation of CellClass::Recalc_Passability.
 *
 *  @author: ZivDero, tomsons26
 */
void CellClassExt::_Recalc_Passability()
{
    if (!Map.In_Local_Radar(CellID)) {
        Passability = PASSABLE_OUTSIDE;
        return;
    }

    if (Overlay != OVERLAY_NONE) {
        OverlayTypeClass const* overlay = OverlayTypes[Overlay];
        if (Land == LAND_TUNNEL && Extension::Fetch(overlay)->IsWaterTunnel) {
            Passability = PASSABLE_WATER;
            return;
        }
        if (overlay->IsCrushable) {
            Passability = PASSABLE_CRUSH;
            return;
        }
        if (overlay->IsWall) {
            Passability = PASSABLE_BLOCKED;
            return;
        }
        if (Ground[overlay->Land].Cost[SPEED_WHEEL] == 0) {
            Passability = PASSABLE_NO;
            return;
        }
    }

    if (Land == LAND_BEACH) {
        if (RuleExtension->IsBeachIsCrush) {
            Passability = PASSABLE_CRUSH;
        } else {
            Passability = PASSABLE_WATER;
        }
        return;
    }

    if (Land == LAND_WATER || (Land == LAND_TUNNEL && ITType != ISOTILE_NONE && Extension::Fetch(IsoTileTypes[ITType])->IsWaterTunnel)) {
        Passability = PASSABLE_WATER;
        return;
    }

    if (Ground[Land].Cost[SPEED_WHEEL] <= 0.01) {
        Passability = PASSABLE_NO;
        return;
    }

    ObjectClass* occupier = OccupierPtr;
    while (occupier != nullptr) {
        switch (occupier->RTTI) {
        case RTTI_BUILDING:
            if (static_cast<BuildingClass*>(occupier)->Class->IsFirestormWall) {
                if (static_cast<BuildingClass*>(occupier)->House->IsFirestormActive) {
                    Passability = PASSABLE_NO;
                    return;
                }
            } else if (static_cast<BuildingClass*>(occupier)->Class->IsLaserFence) {
                if (static_cast<BuildingClass*>(occupier)->LaserFenceFrame != 12 && static_cast<BuildingClass*>(occupier)->LaserFenceFrame != 8) {
                    Passability = PASSABLE_NO;
                }
            }
            break;

        case RTTI_TERRAIN:
            if ((!TheaterTypeClass::Is_Arctic(Scen->Theater) && static_cast<TerrainClass*>(occupier)->Class->TemperateOccupationBits != 7) || (TheaterTypeClass::Is_Arctic(Scen->Theater) && static_cast<TerrainClass*>(occupier)->Class->SnowOccupationBits != 7)) {
                Passability = PASSABLE_PARTIALLY_BLOCKED;
                return;
            }

            Passability = PASSABLE_BLOCKED;
            return;
        }
        occupier = occupier->Next;
    }

    Passability = PASSABLE_LAND;
}


/**
 *  Re-implementation of CellClass::Reduce_Tiberium.
 *
 *  @author: ZivDero
 */
int CellClassExt::_Reduce_Tiberium(int levels)
{
    Rect dirty = Union(Overlay_Render_Rect(), Overlay_Shadow_Render_Rect());
    dirty.Y -= TacticalRect.Y;

    TiberiumType tibtype = Tiberium_Type_Here();
    int reducer = levels;

    if (levels > 0 && tibtype != TIBERIUM_NONE) {
        TiberiumClass* tiberium = Tiberiums[tibtype];
        if (OverlayData == tiberium->FrameCount - 1) {
            tiberium->Queue_Growth(CellID);
        }
        if (OverlayData + 1 > levels) {
            OverlayData -= levels;
            reducer = levels;
        } else {
            PassabilityType passability = Passability;
            Overlay = OVERLAY_NONE;
            reducer = OverlayData;
            OverlayData = 0;
            Recalc_Attributes();
            if (passability != Passability) {
                Map.Update_Cell_Zone(CellID);
                Map.Update_Cell_Subzones(CellID);
            }
            Map.Flag_Background_Update(CellID);
            tiberium->Clear_Spread_State(CellID);
            for (int facing = FACING_FIRST; facing < FACING_COUNT; facing++) {
                Cell adjacent = ::Adjacent_Cell(CellID, static_cast<FacingType>(facing));
                if (Map.In_Radar(adjacent) && !Extension::Fetch(tiberium)->SpreadState[Map_Cell_Index(adjacent)]) {
                    tiberium->Queue_Spread(adjacent);
                }
            }
        }
        TacticalMap->Register_Dirty_Area(dirty);
        return reducer;
    }
    return 0;
}


/**
 *  #issue-177
 * 
 *  Patches the check for if you own base units before giving you a crate MCV to use the new BaseUnit vector.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00457D90, _CellClass_Goodie_Check_BaseUnit_Quantity_Patch, 0)
{
    GET(FootClass *, object, EBX);
    static UnitTypeClass *unittype;
    static HouseClass *objhouse;
    static UnitType unit;
    static int count;

    objhouse = object->House;

    /**
     *  Fetch the first buildable base unit from the new base unit entry
     *  and get the current count of that unit that this house owns.
     */
    unittype = objhouse->Get_First_Ownable(RuleExtension->BaseUnit);
    if (unittype) {
        unit = unittype->HeapID;
        count = objhouse->UQuantity.Value(unit);
    }

    /**
     *  If no ownable base units were found, continue the force mcv check.
     */
    if (!count) {
        return 0x00457DB8;
    }

    /**
     *  Skip the check.
     */
    return 0x00457DCF;
}


/**
 *  #issue-177
 * 
 *  Patches crates to give you a base unit from the new BaseUnit vector.
 * 
 *  @author: CCHyper, ZivDero
 */
DEFINE_HOOK(0x0045813E, _CellClass_Goodie_Check_CRATE_UNIT_BaseUnit_Patch, 0)
{
    GET(FootClass *, object, EBX);
    static UnitTypeClass *unittype;
    static HouseClass *objhouse;
    static UnitType unit;

    objhouse = object->House;

    /**
     *  Fetch the first buildable base unit from the new base unit entry.
     */
    unittype = objhouse->Get_First_Ownable(RuleExtension->BaseUnit);

    if (unittype) {
        R->EAX(Rule);
        R->EDI(unittype);
        return 0x004581AA;
    }

    R->EAX(Rule);
    R->EDI(unittype);
    return 0x00458148;
}


/**
 *  #issue-177
 *
 *  Patches crates to check if you have refineries and harvesters using the entire lists.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00458148, _CellClass_Goodie_Check_CRATE_UNIT_BuildRefinery_HarvesterUnit_Patch, 0)
{
    GET(FootClass*, object, EBX);
    GET(UnitTypeClass*, unittype, EDI);
    HouseClass* owner_house;

    owner_house = object->House;

    if (owner_house->Count_Owned(Rule->BuildRefinery) > 0 && owner_house->Count_Owned(Rule->HarvesterUnit) == 0) {
        // We can grant a harvester
        unittype = owner_house->Get_First_Ownable(Rule->HarvesterUnit);
    }

    R->EAX(Rule);
    R->EDI(unittype);
    return 0x004581AA;
}


/**
 *  #issue-177
 *
 *  Patches crates to check if a unit is a BaseUnit using the new list.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x0045820E, _CellClass_Goodie_Check_No_Buildings_Force_MCV_BaseUnit_Patch, 0)
{
    GET(UnitTypeClass *, unittype, EDI);

    /**
     *  Check if this is a BaseUnit.
     *  If so, continue the loop.
     */
    if (RuleExtension->BaseUnit.Is_Present(unittype)) {
        return 0x004581BA;
    }

    return 0x0045821B;
}


/**
 *  #issue-381
 * 
 *  Hardcodes shroud and fog to circumvent cheating in multiplayer games.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00454E60, _CellClass_Draw_Shroud_Fog_Patch, 5)
{
    static bool _shroud_one_time = false;
    static const ShapeSet *_shroud_shape;
    static const ShapeSet *_fog_shape;

    /**
     *  Perform a one-time load of the shroud and fog shape data.
     */
    if (!_shroud_one_time) {
        _shroud_shape = static_cast<const ShapeSet*>(MFCD::Retrieve("SHROUD.SHP"));
        _fog_shape = static_cast<const ShapeSet*>(MFCD::Retrieve("FOG.SHP"));
        _shroud_one_time = true;
    }

    /**
     *  If we are playing a multiplayer game, use the hardcoded shape data.
     */
    if (!Session.Singleplayer_Game()) {
        Cell_ShroudShape = reinterpret_cast<const ShapeSet*>(&ShroudShapeBinary);
        Cell_FogShape = reinterpret_cast<const ShapeSet*>(&FogShapeBinary);

    } else {
        Cell_ShroudShape = _shroud_shape;
        Cell_FogShape = _fog_shape;
    }

    /**
     *  Continues function flow.
     */
    return 0;
}


/**
 *  #issue-381
 * 
 *  Hardcodes shroud and fog to circumvent cheating in multiplayer games.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00455130, _CellClass_Draw_Fog_Patch, 5)
{
    static bool _fog_one_time = false;
    static const ShapeSet *_fog_shape;
    
    /**
     *  Perform a one-time load of the fog shape data.
     */
    if (!_fog_one_time) {
        _fog_shape = static_cast<const ShapeSet*>(MFCD::Retrieve("FOG.SHP"));
        _fog_one_time = true;
    }

    /**
     *  If we are playing a multiplayer game, use the hardcoded shape data.
     */
    if (!Session.Singleplayer_Game()) {
        Cell_FixupFogShape = reinterpret_cast<const ShapeSet*>(&FogShapeBinary);

    } else {
        Cell_FixupFogShape = _fog_shape;
    }

    /**
     *  Continues function flow.
     */
    R->EAX(Cell_FixupFogShape);
    return 0;
}


/**
 *  #issue-191
 * 
 *  Fixes a bug where pre-placed crates and crates spawned by a destroyed
 *  truck will trigger a respawn when they are picked up, even when the
 *  Crates game option was disabled.
 * 
 *  @author: CCHyper (based on research by Rampastring)
 */
DEFINE_HOOK(0x00457EAB, _CellClass_Goodie_Check_Crates_Disabled_Respawn_BugFix_Patch, 0)
{
    /**
     *  Random crates are only thing in multiplayer.
     */
    if (Session.Type != GAME_NORMAL) {

        /**
         *  Check to make sure crates are enabled for this game session.
         * 
         *  The original code was missing the Session "Goodies" check.
         */
        if (Rule->IsMPCrates && Session.Options.Goodies) {
            Map.Place_Random_Crate();
        }
    }

    /**
     *  Continues function flow.
     */
continue_function:
    return 0x00457ECE;
}


/**
 *  #issue-161
 * 
 *  Veterancy crate bonus does not check if a object is un-trainable
 *  before granting it the veterancy bonus.
 * 
 *  @author: CCHyper (based on research by Iran)
 */
DEFINE_HOOK(0x0045882C, _CellClass_Goodie_Check_Veterency_Trainable_BugFix_Patch, 0)
{
    GET(ObjectClass *, object, ECX);

    /**
     *  Make sure the ground layer object is a techno.
     */
    if (!object->Is_Techno()) {
        goto continue_loop;
    }

    /**
     *  Is this object trainable? If so, grant it the bonus.
     */
    if (object->TClass->IsTrainable) {
        goto passes_check;
    }

    /**
     *  Continues the loop over the ground layer objects.
     */
continue_loop:
    return 0x0045894E;

    /**
     *  Continue to grant the veterancy bonus.
     */
passes_check:
    return 0x00458839;
}


/**
 *  Prevents buildings that have WallOwner=no from claiming walls.
 * 
 *  Author: Rampastring
 */
DEFINE_HOOK(0x004531E4, _CellClass_Update_Wall_Owner_Skip_Buildings_That_Cannot_Own_Walls_Patch, 0)
{
    GET(BuildingClass*, building, ESI);

    /**
     *  Stolen bytes/code.
     *  Skip the building if it is not active.
     */
    if (!building->IsActive) {
        return 0x0045321C;
    }

    auto buildingtypeext = Extension::Fetch(building->Class);

    /**
     *  Skip the building if it cannot claim walls.
     */
    if (!buildingtypeext->IsWallOwner) {
        return 0x0045321C;
    }

    /**
     *  Continue to further checks in the wall claiming logic.
     */
    return 0x004531EB;
}


/**
 *  Main function for patching the hooks.
 */
void CellClassExtension_Hooks()
{
    Patch_Jump(0x004596C0, &CellClassExt::_Can_Tiberium_Germinate);
    Patch_Jump(0x00459300, &CellClassExt::_Can_Tiberium_Spread);
    Patch_Jump(0x0045B0D0, &CellClassExt::_Can_Place_Veins);
    Patch_Jump(0x004594D0, &CellClassExt::_Spread_Tiberium);
    Patch_Jump(0x00459A00, &CellClassExt::_Recalc_Passability);
    Patch_Jump(0x00456BF0, &CellClassExt::_Reduce_Tiberium);
    /**
     *  Patch away a check for GAME_INTERNET to enable statistics collection.
     */
    Patch_Jump(0x00457E7A, 0x00457E83); // CellClass::Goodie_Check
}
