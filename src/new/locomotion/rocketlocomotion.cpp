/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ROCKETLOCOMOTION.CPP
 *
 *  @authors       ZivDero
 *
 *  @brief         Rocket locomotion implementation.
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
#include "rocketlocomotion.h"
#include "tibsun_inline.h"
#include "tibsun_globals.h"
#include "iomap.h"
#include "aircraftext.h"
#include "aircraft.h"
#include "aircrafttracker.h"
#include "aircrafttype.h"
#include "cell.h"
#include "anim.h"
#include "combat.h"
#include "foot.h"
#include "tactical.h"
#include "wwmath.h"
#include "debughandler.h"
#include "extension.h"
#include "fastmath.h"
#include "vector2.h"
#include "voc.h"


/**
 *  Retrieves the class identifier (CLSID) of the object.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP RocketLocomotionClass::GetClassID(CLSID *pClassID)
{    
    if (pClassID == nullptr) {
        return E_POINTER;
    }

    *pClassID = __uuidof(this);

    return S_OK;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP RocketLocomotionClass::Load(IStream *pStm)
{
    HRESULT hr = LocomotionClass::Locomotion_Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
        // Insert any data to be loaded here.
    }

    new (this) RocketLocomotionClass(NoInitClass());

    return hr;
}


/**
 *  Class default constructor.
 * 
 *  @author: ZivDero
 */
RocketLocomotionClass::RocketLocomotionClass() :
    LocomotionClass(),
    DestinationCoord(),
    MissionTimer(),
    TrailTimer(),
    MissionState(RocketMissionState::None),
    CurrentSpeed(0),
    NeedToSubmit(true),
    IsSpawnerElite(false),
    CurrentPitch(0.0),
    ApogeeDistance(0)
{
}


RocketLocomotionClass::RocketLocomotionClass(const NoInitClass& noinit) : LocomotionClass(noinit) { }

/**
 *  Sees if object is moving.
 *
 *  @author: ZivDero
 */
IFACEMETHODIMP_(bool) RocketLocomotionClass::Is_Moving()
{
    return DestinationCoord != Coord();
}


/**
 *  Fetches destination coordinate.
 *
 *  @author: ZivDero
 */
IFACEMETHODIMP_(Coord) RocketLocomotionClass::Destination()
{
    return DestinationCoord;
}


/**
 *  Fetch voxel draw matrix.
 * 
 *  @author: ZivDero
 */
IFACEMETHODIMP_(Matrix3D) RocketLocomotionClass::Draw_Matrix(int *key)
{
    Matrix3D matrix;
    matrix.Make_Identity();

    /**
     *  Rotate the rocket to its current facing.
     */
    const float z_angle = LinkedTo->PrimaryFacing.Current().Get_Radian<32>();
    matrix.Rotate_Z(z_angle);

    if (CurrentPitch != 0.0)
    {
        matrix.Rotate_Y(-CurrentPitch);

        /**
         *  Get this rocket's type.
         */
        const auto atype = reinterpret_cast<AircraftClass*>(LinkedTo)->Class;
        const RocketTypeClass* rocket = RocketTypeClass::From_AircraftType(atype);

        if (key)
        {
            if (CurrentPitch == rocket->PitchInitial * DEG_TO_RAD(90))
                *key |= 0x20;
            else if (CurrentPitch == rocket->PitchFinal * DEG_TO_RAD(90))
                *key |= 0x40;
            else
                *key = -1;
        }
    }

    if (key)
    {
        *key |= LinkedTo->PrimaryFacing.Current().Get_Facing<32>();
    }

    return matrix;
}


/**
 *  Shadow draw point center location.
 *
 *  @author: ZivDero
 */
IFACEMETHODIMP_(Point2D) RocketLocomotionClass::Shadow_Point()
{
    return { 0, 0 };
}


/**
 *  Process movement of object.
 * 
 *  @author: ZivDero
 */
IFACEMETHODIMP_(bool) RocketLocomotionClass::Process()
{
    /**
     *  Get this rocket's type.
     */
    const auto atype = reinterpret_cast<AircraftClass*>(LinkedTo)->Class;
    const RocketTypeClass* rocket = RocketTypeClass::From_AircraftType(atype);

    TechnoClass* spawn_owner = Extension::Fetch(LinkedTo)->SpawnOwner;

    switch (MissionState)
    {
        /**
         *  The rocket is currently waiting to be launched.
         */
    case RocketMissionState::Pause:
        {
            CurrentSpeed = 0;
            IsSpawnerElite = spawn_owner && spawn_owner->Veterancy.Is_Elite();

            /**
             *  Cruise missiles spawn a "taking off" animation in this state.
             */
            if (rocket->IsCruiseMissile)
            {
                if (TrailTimer.Expired() && rocket->TakeoffAnim)
                {
                    new AnimClass(rocket->TakeoffAnim, LinkedTo->Position, 2, 1, SHAPE_WIN_REL | SHAPE_CENTER, -10);
                    TrailTimer = 24;
                }

                if (NeedToSubmit)
                {
                    LinkedTo->Mark(MARK_UP);
                    NeedToSubmit = false;
                    Map.Submit(LinkedTo);
                    LinkedTo->Mark(MARK_DOWN);
                }
            }
            else
            {
                NeedToSubmit = true;
            }

            /**
             *  If we're done waiting, proceed to take off.
             *  Cruise missiles take off vertically, while regular rockets tilt up first.
             */
            if (MissionTimer.Expired())
            {
                MissionState = rocket->IsCruiseMissile ? RocketMissionState::VerticalTakeOff : RocketMissionState::GainingAltitude;
                MissionTimer = rocket->TiltFrames;
            }
            
        }
        break;

        /**
         *  The rocket is currently tilting up to its firing position.
         */
    case RocketMissionState::Tilt:
        {
            CurrentSpeed = 0;
            IsSpawnerElite = spawn_owner && spawn_owner->Veterancy.Is_Elite();

            /**
             *  If the rocket is done tilting, play a sound and animation, and proceed to take off.
             */
            if (MissionTimer.Expired())
            {
                CurrentPitch = rocket->PitchFinal * DEG_TO_RAD(90);
                MissionState = RocketMissionState::GainingAltitude;

                auto linked_ext = Extension::Fetch(LinkedTo);
                if (linked_ext->Get_Last_Flight_Cell() == CELL_NONE)
                    AircraftTracker->Track(LinkedTo);

                if (rocket->TakeoffAnim)
                    new AnimClass(rocket->TakeoffAnim, LinkedTo->Position, 2, 1, SHAPE_WIN_REL | SHAPE_CENTER, -10);

                Static_Sound(LinkedTo->TClass->AuxSound1, LinkedTo->Position);
            }
            /**
             *  Otherwise, keep tilting.
             */
            else
            {
                const double pitch_initial = rocket->PitchInitial * DEG_TO_RAD(90);
                const double pitch_final = rocket->PitchFinal * DEG_TO_RAD(90);
                CurrentPitch = (pitch_final - pitch_initial) * MissionTimer.Percent_Expired() + pitch_initial;
            }
            break;
        }

        /**
         *  The rocket is currently gaining altitude.
         */
    case RocketMissionState::GainingAltitude:
        {
            if (!NeedToSubmit)
            {
                LinkedTo->Mark(MARK_UP);
                NeedToSubmit = true;
                Map.Submit(LinkedTo);
                LinkedTo->Mark(MARK_DOWN);
            }

            /**
             *  Accelerate towards the maximum speed.
             */
            CurrentSpeed += rocket->Acceleration;
            CurrentSpeed = std::min(CurrentSpeed, static_cast<double>(LinkedTo->TClass->MaxSpeed));

            /**
             *  If the rocket has reached its cruising altitude, proceed to flight.
             *  Save the distance to the destination for lazy curve rockets.
             */
            if (LinkedTo->HeightAGL >= rocket->Altitude)
            {
                MissionState = RocketMissionState::Flight;
                Coord center_coord = LinkedTo->Center_Coord();
                ApogeeDistance = (Cell(center_coord.X, center_coord.Y) - Cell(DestinationCoord.X, DestinationCoord.Y)).Length();
            }
            break;
        }

        /**
         *  The rocket is currently in flight.
         */
    case RocketMissionState::Flight:
        {
            /**
             *  Check if we're still above ground. If not, explode.
             */
            if (LinkedTo->HeightAGL > 0)
            {
                /**
                 *  Keep accelerating towards the maximum speed.
                 */
                CurrentSpeed += rocket->Acceleration;
                CurrentSpeed = std::min(CurrentSpeed, static_cast<double>(LinkedTo->TClass->MaxSpeed));

                /**
                 *  Lazy curve rockets curve towards the destination.
                 */
                if (rocket->IsLazyCurve && ApogeeDistance)
                {
                    /**
                     *  Since the rocket doesn't dip towards the ground explicitly,
                     *  we need to check if it's time to explode.
                     */
                    if (Time_To_Explode(rocket))
                        return false;

                    /**
                     *  Calculate how much to tilt the rocket based on the distance to the destination
                     *  compared to how far it was when it reached its cruising altitude.
                     */
                    const Coord center_coord = LinkedTo->Center_Coord();
                    const double dist = (Cell(center_coord.X, center_coord.Y) - Cell(DestinationCoord.X, DestinationCoord.Y)).Length();
                    const double ratio = dist / ApogeeDistance;

                    CurrentPitch = rocket->PitchFinal * ratio * DEG_TO_RAD(90) + Get_Next_Pitch() * (1 - ratio);
                }
                else
                {
                    /**
                     *  Level off the rocket, slowly.
                     */
                    if (CurrentPitch > 0.0)
                    {
                        CurrentPitch -= rocket->TurnRate;
                        CurrentPitch = std::max(CurrentPitch, 0.0);
                    }

                    /**
                     *  If we're there, proceed to closing in.
                     */
                    const int horizontal_distance = (Cell(LinkedTo->Center_Coord().X, LinkedTo->Center_Coord().Y) - Cell(DestinationCoord.X, DestinationCoord.Y)).Length();
                    const int vertical_distance = LinkedTo->Center_Coord().Z - DestinationCoord.Z;
                    if (horizontal_distance <= vertical_distance * rocket->CloseEnoughFactor)
                        MissionState = RocketMissionState::ClosingIn;
                }

                /**
                 *  Orient the rocket towards the destination.
                 */
                const Coord center_coord = LinkedTo->Center_Coord();
                LinkedTo->PrimaryFacing.Set_Desired(Desired_Facing(DestinationCoord.X, DestinationCoord.Y, center_coord.X, center_coord.Y));
            }
            else
            {
                /**
                 *  KABOOM!!!
                 */
                Explode();
                return false;
            }

            /**
             *  If the rocket has flown outside the map's bounds, remove it so as to not lag the game.
             */
            if (!Map.In_Radar(LinkedTo->Center_Coord().As_Cell()))
                LinkedTo->Delete_Me();

            break;
        }

    case RocketMissionState::ClosingIn:
        {
            /**
             *  Check if it's time to explode.
             */
            if (Time_To_Explode(rocket))
                return false;

            /**
             *  Pitch towards the destination.
             */
            const double pitch = Get_Next_Pitch() - CurrentPitch;

            if (std::abs(pitch) > rocket->TurnRate)
                CurrentPitch = pitch < 0 ? CurrentPitch - rocket->TurnRate : CurrentPitch + rocket->TurnRate;
            else
                CurrentPitch += pitch;

            break;
        }

        /**
         *  Cruise missiles take off vertically.
         */
    case RocketMissionState::VerticalTakeOff:
        {
            IsSpawnerElite = spawn_owner && spawn_owner->Veterancy.Is_Elite();

            /**
             *  Spawn the trail animation, as if the rocket is doing its best to lift off.
             */
            if (TrailTimer.Expired())
            {
                if (rocket->TakeoffAnim)
                {
                    new AnimClass(rocket->TakeoffAnim, LinkedTo->Position, 2, 1, SHAPE_WIN_REL | SHAPE_CENTER, -10);
                    TrailTimer = 24;
                }
            }

            /**
             *  If we're done taking off, play a sound and proceed to flight.
             */
            if (MissionTimer.Expired())
            {
                CurrentPitch = rocket->PitchFinal * DEG_TO_RAD(90);

                const auto linked_ext = Extension::Fetch(LinkedTo);
                if (linked_ext->Get_Last_Flight_Cell() == CELL_NONE)
                {
                    Static_Sound(LinkedTo->TClass->AuxSound1, LinkedTo->Position);
                    AircraftTracker->Track(LinkedTo);
                }

                TrailTimer = 0;
                MissionState = RocketMissionState::GainingAltitude;
            }
            /**
             *  Otherwise, slowly nudge the rocket upwards to simulate taking off.
             */
            else
            {
                Coord coord = LinkedTo->Position;
                coord.Z += rocket->RaiseRate;
                if (Map.In_Radar(coord.As_Cell()))
                    LinkedTo->PositionCoord = coord;
            }
        }
        break;

    default:
        break;
    }

    /**
     *  Spawn the rocket's trail.
     */
    if (Is_Moving_Now() && TrailTimer.Expired() && rocket->TrailAnim)
    {
        new AnimClass(rocket->TrailAnim, LinkedTo->Position, rocket->TrailAppearDelay, 1, SHAPE_WIN_REL | SHAPE_CENTER);
        TrailTimer = rocket->TrailSpawnDelay;
    }

    /**
     *  Move the rocket.
     */
    if (CurrentSpeed > 0.0)
    {
        Coord coord = Get_Next_Position(static_cast<int>(CurrentSpeed));

        if (Map.In_Radar(coord.As_Cell()))
            LinkedTo->PositionCoord = coord;

        if (LinkedTo->Strength <= 0)
            Explode();
    }

    return Is_Moving();
}


/**
 *  Instruct to move to location specified.
 * 
 *  @author: ZivDero
 */
IFACEMETHODIMP_(void) RocketLocomotionClass::Move_To(Coord to)
{
    const auto atype = reinterpret_cast<AircraftClass*>(LinkedTo)->Class;
    const RocketTypeClass* rocket = RocketTypeClass::From_AircraftType(atype);

    /**
     *  Rockets only accept a destination once.
     */
    if (DestinationCoord == COORD_NONE)
    {
        int timer_delay;
        if (rocket->PauseFrames)
        {
            MissionState = RocketMissionState::Pause;
            timer_delay = rocket->PauseFrames;
        }
        else
        {
            MissionState = RocketMissionState::Tilt;
            timer_delay = rocket->TiltFrames;
        }

        MissionTimer = timer_delay;
        CurrentPitch = rocket->PitchInitial * DEG_TO_RAD(90);

        /**
         *  Apply some inaccuracy to the coordinate if the rocket type specifies so.
         */
        if (rocket->Inaccuracy <= 0) {
            DestinationCoord = to;
        }
        else {
            const int randomx = Random_Pick(-rocket->Inaccuracy, rocket->Inaccuracy);
            const int randomy = Random_Pick(-rocket->Inaccuracy, rocket->Inaccuracy);
            DestinationCoord = to + Coord(randomx, randomy, 0);
        }
    }
}


/**
 *  Stop moving at first opportunity.
 * 
 *  @author: ZivDero
 */
IFACEMETHODIMP_(void) RocketLocomotionClass::Stop_Moving()
{
}


/**
 *  What display layer is it located in.
 * 
 *  @author: ZivDero
 */
IFACEMETHODIMP_(LayerType) RocketLocomotionClass::In_Which_Layer()
{
    return LAYER_TOP;
}


/**
 *  Is it actually moving across the ground this very second?
 * 
 *  @author: ZivDero
 */
IFACEMETHODIMP_(bool) RocketLocomotionClass::Is_Moving_Now()
{
    return MissionState >= RocketMissionState::GainingAltitude && MissionState <= RocketMissionState::ClosingIn;
}


/**
 *  Calculates the next position of the rocket along its current trajectory.
 *
 *  @author: ZivDero
 */
Coord RocketLocomotionClass::Get_Next_Position(double speed) const
{
    Coord coord;

    const double horizontal_speed = FastMath::Cos(CurrentPitch) * speed;
    const double horizontal_angle = LinkedTo->PrimaryFacing.Current().Get_Radian();

    coord.X = static_cast<int>(LinkedTo->Position.X + FastMath::Cos(horizontal_angle) * horizontal_speed);
    coord.Y = static_cast<int>(LinkedTo->Position.Y - FastMath::Sin(horizontal_angle) * horizontal_speed);
    coord.Z = static_cast<int>(LinkedTo->Position.Z + FastMath::Sin(CurrentPitch) * speed);

    return coord;
}


/**
 *  Calculates the next pitch of the rocket along its current trajectory.
 *
 *  @author: ZivDero
 */
double RocketLocomotionClass::Get_Next_Pitch() const
{
    /**
     *  Calculate how much is there left to go.
     */
    const Coord left_to_go = DestinationCoord - LinkedTo->Position;
    const double length = Vector2(static_cast<float>(left_to_go.X), static_cast<float>(left_to_go.Y)).Length();

    /**
     *  If we're still not there, calculate the pitch at which we should go.
     */
    if (length > 0)
        return FastMath::Atan(left_to_go.Z / length);

    /**
     *  Otherwise it's time to go straight down.
     */
    return -DEG_TO_RAD(90);
}


void RocketLocomotionClass::Explode()
{
    AircraftTracker->Untrack(LinkedTo);

    /**
     *  Get the warhead this rocket carries.
     */
    const auto atype = reinterpret_cast<AircraftClass*>(LinkedTo)->Class;
    const RocketTypeClass* rocket = RocketTypeClass::From_AircraftType(atype);
    const WarheadTypeClass* warhead = (IsSpawnerElite && rocket->EliteWarhead) ? rocket->EliteWarhead : rocket->Warhead;

    /**
     *  Calculate where it's moving right now.
     */
    Coord coord = Get_Next_Position(rocket->BodyLength);
    Cell cell = coord.As_Cell();

    /**
     *  The rocket uses its spawner's elite status to determine if it should deal elite damage.
     */
    int damage = IsSpawnerElite ? rocket->EliteDamage : rocket->Damage;

    /**
     *  KABOOM!!!
     */
    const auto animtype = Combat_Anim(damage, warhead, Map[cell].Land_Type(), &coord);
    if (animtype)
        new AnimClass(animtype, coord, 0, 1, SHAPE_WIN_REL | SHAPE_CENTER | SHAPE_FLAT, Get_Explosion_Z(coord));
    Combat_Lighting(coord, damage, warhead);
    Explosion_Damage(coord, damage, LinkedTo, warhead, true);
    LinkedTo->Delete_Me();
}



bool RocketLocomotionClass::Time_To_Explode(const RocketTypeClass* rocket)
{
    Coord coord = Get_Next_Position(rocket->BodyLength);

    /**
     *  Check if we're there yet.
     */
    if (coord.Z > DestinationCoord.Z)
    {
        const CellClass* rocket_cell = LinkedTo->Get_Cell_Ptr();
        if (!rocket_cell || !rocket_cell->IsUnderBridge || DestinationCoord.Z != rocket_cell->Center_Coord().Z || coord.Z > DestinationCoord.Z + ROCKET_SPEED)
        {
            /**
             *  Nope, too early.
             */
            if (LinkedTo->HeightAGL > 0)
                return false;
        }
    }

    /**
     *  KABOOM!!!
     */
    Explode();
    return true;
}
