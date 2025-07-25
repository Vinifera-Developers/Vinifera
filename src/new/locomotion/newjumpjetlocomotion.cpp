/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          NEWJUMPJETLOCOMOTION.CPP
 *
 *  @authors       ZivDero, tomsons26
 *
 *  @brief         Jumpjet locomotion re-implementation.
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
#include "newjumpjetlocomotion.h"

#include <algorithm>
#include <new>

#include "aircrafttracker.h"
#include "rules.h"
#include "foot.h"
#include "mouse.h"
#include "cell.h"
#include "building.h"
#include "coord.h"
#include "extension.h"
#include "footext.h"
#include "house.h"
#include "ionstorm.h"
#include "technotypeext.h"
#include "vinifera_globals.h"


NewJumpjetLocomotionClass::NewJumpjetLocomotionClass() :
    BASECLASS(),
    JumpjetTurnRate(Rule->JumpjetTurnRate),
    JumpjetSpeed(Rule->JumpjetSpeed),
    JumpjetClimb(Rule->JumpjetClimb),
    JumpjetCruiseHeight(Rule->JumpjetCruiseHeight),
    JumpjetAcceleration(Rule->JumpjetAcceleration),
    JumpjetWobblesPerSecond(Rule->JumpjetWobblesPerSecond),
    JumpjetWobbleDeviation(Rule->JumpjetWobbleDeviation),
    JumpjetNoWobbles(false),
    JumpjetCloakDetectionRadius(Rule->JumpjetCloakDetectionRadius),
    HeadToCoord(COORD_NONE),
    IsMoving(false),
    CurrentState(GROUNDED),
    Facing(Rule->JumpjetTurnRate),
    CurrentSpeed(0.0),
    TargetSpeed(0.0),
    FlightLevel(0),
    CurrentWobble(0.0),
    IsLanding(false)
{

}


IFACEMETHODIMP NewJumpjetLocomotionClass::Link_To_Object(void* object)
{
    TechnoTypeClassExtension const* type_ext = Extension::Fetch(static_cast<TechnoClass*>(object)->TClass);
    JumpjetTurnRate = type_ext->JumpjetTurnRate;
    JumpjetSpeed = type_ext->JumpjetSpeed;
    JumpjetClimb = type_ext->JumpjetClimb;
    int height = type_ext->JumpjetCruiseHeight;
    height = std::max(height, 2 * LEVEL_LEPTON_H);
    JumpjetCruiseHeight = height;
    JumpjetAcceleration = type_ext->JumpjetAcceleration;
    JumpjetWobblesPerSecond = type_ext->JumpjetWobblesPerSecond;
    JumpjetWobbleDeviation = type_ext->JumpjetWobbleDeviation;
    Facing = JumpjetTurnRate;
    Facing.Set_Desired(DirType(0x4000));
    Facing.Set(DirType(0x4000));
    JumpjetCloakDetectionRadius = type_ext->JumpjetCloakDetectionRadius;
    JumpjetNoWobbles = type_ext->JumpjetNoWobbles;
    return BASECLASS::Link_To_Object(object);
}


IFACEMETHODIMP_(bool) NewJumpjetLocomotionClass::Is_Moving()
{
    return IsMoving;
}


IFACEMETHODIMP_(Coord) NewJumpjetLocomotionClass::Destination()
{
    if (Is_Moving()) {
        return HeadToCoord;
    } else {
        return COORD_NONE;
    }
}


IFACEMETHODIMP_(bool) NewJumpjetLocomotionClass::Process()
{
    LayerType layer = In_Which_Layer();

    if (Is_Moving() || Is_Moving_Now()) {
        Movement_AI();
        if (!LinkedTo->IsActive) {
            return false;
        }

        if (IonStorm_Is_Active()) {
            if (CurrentState != GROUNDED) {
                ResultType result = LinkedTo->Take_Damage(LinkedTo->Strength, 0, Rule->C4Warhead, nullptr, true, true);
                if (result == RESULT_DESTROYED) {
                    return false;
                }
            }
        }

        switch (CurrentState) {
            case GROUNDED:
                Process_Grounded();
                break;

            case ASCENDING:
                Process_Ascent();
                break;

            case HOVERING:
                Process_Hover();
                break;

            case CRUISING:
                Process_Cruise();
                break;

            case DESCENDING:
                Process_Descent();
                break;
        }

        if (LinkedTo->IsSelected && LinkedTo->House != PlayerPtr) {
            if (Map.Is_Shrouded(LinkedTo->PositionCoord) || (Scen->Special.IsFogOfWar && Map.Is_Fogged(LinkedTo->PositionCoord))) {
                LinkedTo->Unselect();
            }
        }

        if (!LinkedTo->IsDiscoveredByPlayer && LinkedTo->House != PlayerPtr) {
            if (!Map.Is_Shrouded(LinkedTo->PositionCoord)) {
                LinkedTo->Revealed(PlayerPtr);
            }
        }
    }

    if (In_Which_Layer() != layer) {
        Map.Submit(LinkedTo);
    }

    return false;
}


IFACEMETHODIMP_(void) NewJumpjetLocomotionClass::Move_To(Coord to)
{
    if (HeadToCoord != COORD_NONE && CurrentState != GROUNDED && IsLanding) {
        LinkedTo->Clear_Occupy_Bit(HeadToCoord);
        IsLanding = false;
    }

    HeadToCoord = to;

    if (to != COORD_NONE) {
        Cell cell = Map.Nearby_Location(to.As_Cell(), LinkedTo->TClass->Speed, -1, MZONE_FLYER, Map[to].IsUnderBridge);
        Coord free = Closest_Free_Spot(cell.As_Coord());
        if (free != COORD_NONE) {
            HeadToCoord = free;
            LinkedTo->NavCom = &Map[HeadToCoord];
            IsMoving = true;
            if (CurrentState == DESCENDING) {
                CurrentState = ASCENDING;
                FlightLevel = JumpjetCruiseHeight;
            }
        }
    } else {
        IsMoving = false;
    }
}


IFACEMETHODIMP_(void) NewJumpjetLocomotionClass::Stop_Moving()
{
    if (IsMoving) {
        if (HeadToCoord != COORD_NONE && CurrentState != GROUNDED && IsLanding) {
            LinkedTo->Clear_Occupy_Bit(HeadToCoord);
            IsLanding = false;
        }
        Cell cell = LinkedTo->PositionCoord.As_Cell();
        Cell nearby = Map.Nearby_Location(cell, SPEED_TRACK);
        if (nearby != CELL_NONE) {
            Coord nearby_coord = nearby.As_Coord();
            nearby_coord.Z = Map.Get_Height_GL(nearby_coord);
            if (Map[nearby].IsUnderBridge) {
                nearby_coord.Z += BRIDGE_LEPTON_HEIGHT;
            }
            Move_To(nearby_coord);
        } else {
            LinkedTo->Take_Damage(LinkedTo->Strength, 0, Rule->C4Warhead, nullptr, true, true);
            HeadToCoord = COORD_NONE;
        }
    }
}


IFACEMETHODIMP_(void) NewJumpjetLocomotionClass::Do_Turn(DirType coord)
{
    LinkedTo->PrimaryFacing.Set(coord);
}


IFACEMETHODIMP_(HRESULT) NewJumpjetLocomotionClass::GetClassID(CLSID * pClassID)
{
    if (pClassID == nullptr) {
        return E_POINTER;
    }

    *pClassID = __uuidof(this);

    return S_OK;
}


IFACEMETHODIMP_(HRESULT) NewJumpjetLocomotionClass::Load(IStream * stream)
{
    HRESULT result = BASECLASS::Locomotion_Load(stream);
    if (SUCCEEDED(result)) {
        new (this) NewJumpjetLocomotionClass(NoInitClass());
    }
    return result;
}


IFACEMETHODIMP_(LayerType) NewJumpjetLocomotionClass::In_Which_Layer()
{
    int height = LinkedTo->HeightAGL;
    if (!LinkedTo->IsOnBridge) {
        if (Map[LinkedTo->PositionCoord].IsUnderBridge && height >= BRIDGE_LEPTON_HEIGHT && !LinkedTo->IsFalling) {
            height -= BRIDGE_LEPTON_HEIGHT;
        }
    }

    if (height == 0) {
        return LAYER_GROUND;
    } else if (height < JumpjetCruiseHeight) {
        return LAYER_AIR;
    } else {
        return LAYER_TOP;
    }
}


void NewJumpjetLocomotionClass::Process_Grounded()
{
    if (Is_Moving()) {
        LinkedTo->Set_Speed(1.0);
        Facing.Set(LinkedTo->PrimaryFacing.Current());
        CurrentSpeed = 0;
        TargetSpeed = 0;
        FlightLevel = JumpjetCruiseHeight;
        if (!IonStorm_Is_Active()) {
            const auto extension = Extension::Fetch(LinkedTo);
            if (extension->Get_Last_Flight_Cell() == CELL_NONE) {
                AircraftTracker->Track(LinkedTo);
            }

            CurrentState = ASCENDING;
        }
    }
}


void NewJumpjetLocomotionClass::Process_Ascent()
{
    int height = LinkedTo->HeightAGL;
    if (!LinkedTo->IsOnBridge) {
        if (Map[LinkedTo->PositionCoord].IsUnderBridge && height >= BRIDGE_LEPTON_HEIGHT) {
            height -= BRIDGE_LEPTON_HEIGHT;
        }
    }

    const auto extension = Extension::Fetch(LinkedTo);
    Cell oldcell = extension->Get_Last_Flight_Cell();
    Cell newcell = LinkedTo->Get_Cell();

    if (newcell != oldcell) {
        AircraftTracker->Update_Position(LinkedTo, oldcell, newcell);
    }

    if (height >= FlightLevel) {
        CurrentState = HOVERING;
    } else if (height > FlightLevel / 4) {
        TargetSpeed = JumpjetSpeed;
        Facing.Set_Desired(Direction(LinkedTo->PositionCoord, HeadToCoord));
    }
}


void NewJumpjetLocomotionClass::Process_Hover()
{
    if (Is_Moving()) {
        Coord headto = HeadToCoord;
        Coord position = LinkedTo->PositionCoord;
        if (Point2D(headto.X, headto.Y) == Point2D(position.X, position.Y)) {
            if (LinkedTo->TarCom == nullptr) {
                CurrentState = DESCENDING;
            }
        } else {
            Facing.Set_Desired(Direction(LinkedTo->PositionCoord, HeadToCoord));
            CurrentState = CRUISING;
        }
    }
}


void NewJumpjetLocomotionClass::Process_Cruise()
{
    Coord position = LinkedTo->PositionCoord;

    const auto extension = Extension::Fetch(LinkedTo);
    Cell oldcell = extension->Get_Last_Flight_Cell();
    Cell newcell = LinkedTo->Get_Cell();

    if (newcell != oldcell) {
        AircraftTracker->Update_Position(LinkedTo, oldcell, newcell);
    }

    Facing.Set_Desired(Direction(position, HeadToCoord));

    int distance = Point2D(position.X, position.Y).Distance_To(Point2D(HeadToCoord.X, HeadToCoord.Y));
    if (distance < 20) {
        CurrentSpeed = 0;
        TargetSpeed = 0;
        position.X = HeadToCoord.X;
        position.Y = HeadToCoord.Y;
        bool down = LinkedTo->IsDown;
        LinkedTo->IsDown = false;
        LinkedTo->PositionCoord = position;
        LinkedTo->IsDown = down;
        if (LinkedTo->TarCom == nullptr) {
            FlightLevel = 0;
            CurrentState = DESCENDING;
        } else {
            CurrentState = HOVERING;
        }
    } else if (distance < CELL_LEPTON) {
        TargetSpeed = JumpjetSpeed * 0.3;
        if (LinkedTo->TarCom == nullptr) {
            FlightLevel = static_cast<int>(JumpjetCruiseHeight * 0.75);
        }
    } else if (distance < CELL_LEPTON * 2) {
        TargetSpeed = JumpjetSpeed * 0.5;
    } else {
        TargetSpeed = JumpjetSpeed;
        FlightLevel = JumpjetCruiseHeight;
    }
}


void NewJumpjetLocomotionClass::Process_Descent()
{
    CellClass * cellptr = &Map[HeadToCoord];
    MoveType move = LinkedTo->Can_Enter_Cell(cellptr);
    int spot = CellClass::Spot_Index(HeadToCoord);
    bool stop = true;

    if ((cellptr->Is_Spot_Free(spot, cellptr->IsUnderBridge) || IsLanding) &&
        (Map[HeadToCoord].IsUnderBridge || move <= MOVE_MOVING_BLOCK && (move != MOVE_MOVING_BLOCK || IsLanding))) {
        
        if (!IsLanding) {
            IsLanding = true;
            LinkedTo->Set_Occupy_Bit(HeadToCoord);
        }

        FlightLevel = 0;

        int height = LinkedTo->HeightAGL;
        if (!LinkedTo->IsOnBridge) {
            if (Map[LinkedTo->PositionCoord].IsUnderBridge && height >= BRIDGE_LEPTON_HEIGHT) {
                height -= BRIDGE_LEPTON_HEIGHT;
            }
        }

        if (height == 0) {
            LinkedTo->Set_Speed(0.0);
            LinkedTo->Mark(MARK_UP);
            LinkedTo->PositionCoord = HeadToCoord;

            if (LinkedTo->PositionCoord.Z > Map.Get_Height_GL(LinkedTo->PositionCoord)) {
                if (Map[LinkedTo->PositionCoord].IsUnderBridge) {
                    LinkedTo->IsOnBridge = true;
                }
            }

            LinkedTo->Mark(MARK_DOWN);
            HeadToCoord = COORD_NONE;
            IsMoving = false;
            LinkedTo->Assign_Destination(nullptr);
            LinkedTo->Per_Cell_Process(PCP_END);

            if (LinkedTo != nullptr && LinkedTo->IsActive && !LinkedTo->IsInLimbo && !LinkedTo->IsFalling) {
                LinkedTo->Look();
                AircraftTracker->Untrack(LinkedTo);
                CurrentState = GROUNDED;
                IsLanding = false;
            }
        }
        stop = false;
    }

    if (stop) Stop_Moving();
}


IFACEMETHODIMP_(bool) NewJumpjetLocomotionClass::Is_Moving_Now()
{
    if (CurrentState != GROUNDED && CurrentState != HOVERING) {
        return true;
    }
    return false;
}


void NewJumpjetLocomotionClass::Movement_AI()
{
    bool need_to_mark = CurrentState != HOVERING && CurrentState != CRUISING;
    bool was_down = LinkedTo->IsDown;

    if (need_to_mark) {
        LinkedTo->Mark(MARK_UP);
    } else {
        LinkedTo->IsDown = false;
    }

    if (TargetSpeed > CurrentSpeed) {
        CurrentSpeed += JumpjetAcceleration;
        CurrentSpeed = std::min(CurrentSpeed, static_cast<double>(JumpjetSpeed));
    }
    if (TargetSpeed < CurrentSpeed) {
        CurrentSpeed -= JumpjetAcceleration * 1.5;
        CurrentSpeed = std::max(CurrentSpeed, 0.0);
    }

    LinkedTo->Set_Speed(CurrentSpeed / JumpjetSpeed);

    bool at_destination = LinkedTo->Get_Cell() == HeadToCoord.As_Cell();

    if ((CurrentState == HOVERING || CurrentState == CRUISING) && !JumpjetNoWobbles) {
        CurrentWobble += DEG_TO_RAD(360) / (15.0 / JumpjetWobblesPerSecond);
    } else {
        CurrentWobble = 0;
    }

    int desired_height = std::sin(CurrentWobble) * JumpjetWobbleDeviation + FlightLevel;
    int height = LinkedTo->Height;
    int ground_height = Map.Get_Height_GL(LinkedTo->PositionCoord);

    if (Map[LinkedTo->PositionCoord].IsUnderBridge && LinkedTo->PositionCoord.Z >= ground_height + 4 * LEVEL_LEPTON_H) {
        ground_height += BRIDGE_LEPTON_HEIGHT;
    }

    int height_diff = 0;
    if (CurrentState != DESCENDING && CurrentState != GROUNDED && !at_destination) {
        height_diff = height - Desired_Flight_Level();
    } else {
        height_diff = height - ground_height;
    }

    bool moved = false;
    if (height_diff < desired_height) {
        int height_agl = LinkedTo->HeightAGL;
        if (Map[LinkedTo->PositionCoord].IsUnderBridge && !LinkedTo->IsOnBridge) {
            if (LinkedTo->PositionCoord.Z >= Map.Get_Height_GL(LinkedTo->PositionCoord) + BRIDGE_LEPTON_HEIGHT) {
                height_agl -= BRIDGE_LEPTON_HEIGHT;
            }
        }
        if (height_agl == 0) {
            LinkedTo->Clear_Occupy_Bit(LinkedTo->PositionCoord);
            LinkedTo->IsOnBridge = false;
        }
        height += JumpjetClimb;
        moved = true;
    }
    if (height_diff > desired_height) {
        height -= JumpjetClimb;
        height = std::max(height, ground_height);
        moved = true;
        height_diff = std::max(height_diff, 0);
    }

    if (LinkedTo->Get_Cell() != HeadToCoord.As_Cell()) {
        if (height_diff < desired_height / 2) {
            CurrentSpeed *= 0.9;
        }
        if (height_diff < desired_height / 4) {
            CurrentSpeed *= 0.9;
        }
    }

    if (moved) {
        LinkedTo->Height = height;
        if (need_to_mark) {
            LinkedTo->Mark(MARK_UP);
        }
    }

    Coord new_coord = Coord_Move(LinkedTo->PositionCoord, Facing.Current(), CurrentSpeed);
    LinkedTo->PositionCoord = new_coord;

    if (LinkedTo != nullptr) {
        const int & rad = JumpjetCloakDetectionRadius;
        for (int x = -rad; x <= rad; x++) {
            for (int y = -rad; y <= rad; y++) {
                CellClass * cellptr = &Map[Cell(x, y) + new_coord.As_Cell()];
                if (cellptr != nullptr) {
                    ObjectClass * occupier = cellptr->Cell_Occupier(false);
                    while (occupier != nullptr) {
                        TechnoClass * tech = dynamic_cast<TechnoClass *>(occupier);
                        if (tech != nullptr) {
                            tech->Do_Shimmer();
                        }
                        occupier = occupier->Next;
                    }
                    occupier = cellptr->Cell_Occupier(true);
                    while (occupier != nullptr) {
                        TechnoClass * tech = dynamic_cast<TechnoClass *>(occupier);
                        if (tech != nullptr) {
                            tech->Do_Shimmer();
                        }
                        occupier = occupier->Next;
                    }
                }
            }
        }
    }

    CellClass * cellptr = &Map[new_coord];
    BuildingClass * building = cellptr->Cell_Building();
    if (building && building->Class->IsFirestormWall && building->House->IsFirestormActive) {
        building->Crossing_Firestorm(LinkedTo, true);
    }

    LinkedTo->PrimaryFacing.Set(Facing.Current());

    if (need_to_mark) {
        LinkedTo->Mark(MARK_DOWN);
    } else {
        LinkedTo->IsDown = was_down;
    }
}


Coord NewJumpjetLocomotionClass::Closest_Free_Spot(Coord const & to) const
{
    Coord closest = Map.Closest_Free_Spot(to);
    if (closest != COORD_NONE) {
        closest.Z = Map.Get_Height_GL(closest);
        if (Map[closest].IsUnderBridge) {
            closest.Z += BRIDGE_LEPTON_HEIGHT;
        }
    }
    return closest;
}


int NewJumpjetLocomotionClass::Desired_Flight_Level() const
{
    Coord coord = LinkedTo->PositionCoord;

    int height = Map[coord].Occupier_Height();
    if (Map[coord].IsUnderBridge) {
        height += BRIDGE_LEPTON_HEIGHT;
    }

    if (CurrentSpeed > 0) {
        coord = Adjacent_Cell(coord, static_cast<FacingType>(Facing.Current().Get_Facing<8>()));
        int adjancent_height = Map[coord].Occupier_Height();
        if (Map[coord].IsUnderBridge) {
            height += BRIDGE_LEPTON_HEIGHT;
        }
        if (adjancent_height > height) {
            return adjancent_height;
        }
        return(height + adjancent_height) / 2;
    }
    return height;
}


IFACEMETHODIMP_(void) NewJumpjetLocomotionClass::Mark_All_Occupation_Bits(MarkType mark)
{
    if (mark == MARK_UP) {
        Coord headto = Head_To_Coord();
        if (headto != COORD_NONE && (CurrentState == GROUNDED || IsLanding)) {
            LinkedTo->Clear_Occupy_Bit(headto);
            IsLanding = false;
        }
    }
}


IFACEMETHODIMP_(Coord) NewJumpjetLocomotionClass::Head_To_Coord()
{
    if (CurrentState == GROUNDED) {
        return LinkedTo->PositionCoord;
    } else {
        return HeadToCoord;
    }
}

