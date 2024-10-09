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
    if (SUCCEEDED(hr)) {
        // Insert any data to be loaded here.
    }

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
    TrailerTimer(),
    MissionState(RocketMissionState::None),
    CurrentSpeed(0),
    NeedToSubmit(true),
    IsSpawnerElite(false),
    CurrentPitch(0.0),
    ApogeeDistance(0)
{
}


/**
 *  Sees if object is moving.
 *
 *  @author: ZivDero
 */
IFACEMETHODIMP_(bool) RocketLocomotionClass::Is_Moving()
{
    return DestinationCoord != Coordinate();
}


/**
 *  Fetches destination coordinate.
 *
 *  @author: ZivDero
 */
IFACEMETHODIMP_(Coordinate) RocketLocomotionClass::Destination()
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

    const float z_angle = (Dir_To_32(Linked_To()->PrimaryFacing.Current()) - 12) * -WWMATH_P16; // YR has -8, somehow the facing is wrong??? I'm not convinced this is THE fix, since the facing isn't always correct like this either.
    matrix.Rotate_Z(z_angle);

    if (CurrentPitch != 0.0)
    {
        matrix.Rotate_Y(-CurrentPitch);

        /**
         *  Get this rocket's type.
         */
        const auto atype = reinterpret_cast<AircraftClass*>(Linked_To())->Class;
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
        *key |= Dir_To_32(Linked_To()->PrimaryFacing.Current());
        return matrix;
    }
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
    const auto atype = reinterpret_cast<AircraftClass*>(Linked_To())->Class;
    const RocketTypeClass* rocket = RocketTypeClass::From_AircraftType(atype);

    TechnoClass* spawn_owner = Extension::Fetch<AircraftClassExtension>(Linked_To())->SpawnOwner;

    switch (MissionState)
    {
    case RocketMissionState::Pause:
        {
            CurrentSpeed = 0;
            IsSpawnerElite = spawn_owner && spawn_owner->Veterancy.Is_Elite();

            if (rocket->IsCruiseMissile)
            {
                if (TrailerTimer.Expired() && rocket->TakeoffAnim)
                {
                    new AnimClass(rocket->TakeoffAnim, Linked_To()->Coord, 2, 1, SHAPE_WIN_REL | SHAPE_CENTER, -10);
                    TrailerTimer = 24;
                }

                if (NeedToSubmit)
                {
                    Linked_To()->Mark(MARK_UP);
                    NeedToSubmit = false;
                    Map.Submit(Linked_To());
                    Linked_To()->Mark(MARK_DOWN);
                }
            }
            else
            {
                NeedToSubmit = true;
            }

            if (MissionTimer.Expired())
            {
                MissionState = rocket->IsCruiseMissile ? RocketMissionState::VerticalTakeOff : RocketMissionState::GainingAltitude;
                MissionTimer = rocket->TiltFrames;
            }
            
        }
        break;

    case RocketMissionState::Tilt:
        {
            CurrentSpeed = 0;
            IsSpawnerElite = spawn_owner && spawn_owner->Veterancy.Is_Elite();

            if (MissionTimer.Expired())
            {
                CurrentPitch = rocket->PitchFinal * DEG_TO_RAD(90);
                MissionState = RocketMissionState::GainingAltitude;
                if (rocket->TakeoffAnim)
                    new AnimClass(rocket->TakeoffAnim, Linked_To()->Coord, 2, 1, SHAPE_WIN_REL | SHAPE_CENTER, -10);
                Sound_Effect(Linked_To()->Techno_Type_Class()->AuxSound1, Linked_To()->Coord);
            }
            else
            {
                const double pitch_initial = rocket->PitchInitial * DEG_TO_RAD(90);
                const double pitch_final = rocket->PitchFinal * DEG_TO_RAD(90);
                CurrentPitch = (pitch_final - pitch_initial) * MissionTimer.Percent_Expired() + pitch_initial;
            }
            break;
        }

    case RocketMissionState::GainingAltitude:
        {
            if (!NeedToSubmit)
            {
                Linked_To()->Mark(MARK_UP);
                NeedToSubmit = true;
                Map.Submit(Linked_To());
                Linked_To()->Mark(MARK_DOWN);
            }

            CurrentSpeed += rocket->Acceleration;
            CurrentSpeed = std::min(CurrentSpeed, static_cast<double>(Linked_To()->Techno_Type_Class()->MaxSpeed));

            if (Linked_To()->Get_Height() >= rocket->Altitude)
            {
                MissionState = RocketMissionState::Flight;
                Coordinate center_coord = Linked_To()->Center_Coord();
                ApogeeDistance = static_cast<int>(Vector2(static_cast<float>(center_coord.X - DestinationCoord.X), static_cast<float>(center_coord.Y - DestinationCoord.Y)).Length());
            }
            break;
        }

    case RocketMissionState::Flight:
        {
            if (Linked_To()->Get_Height() > 0)
            {
                CurrentSpeed += rocket->Acceleration;
                CurrentSpeed = std::min(CurrentSpeed, static_cast<double>(Linked_To()->Techno_Type_Class()->MaxSpeed));

                if (rocket->LazyCurve && ApogeeDistance)
                {
                    if (Time_To_Explode(rocket))
                        return false;

                    Coordinate center_coord = Linked_To()->Center_Coord();
                    const double dist = Vector2(static_cast<float>(center_coord.X - DestinationCoord.X), static_cast<float>(center_coord.Y - DestinationCoord.Y)).Length();
                    const double ratio = dist / ApogeeDistance;

                    CurrentPitch = rocket->PitchFinal * ratio * DEG_TO_RAD(90) + Calculate_Pitch() * (1 - ratio);
                }
                else
                {
                    if (CurrentPitch > 0.0)
                    {
                        CurrentPitch -= rocket->TurnRate;
                        CurrentPitch = std::max(CurrentPitch, 0.0);
                    }

                    Coordinate center_coord = Linked_To()->Center_Coord();
                    Coordinate coord(center_coord.X - DestinationCoord.X, center_coord.Y - DestinationCoord.Y, center_coord.Z - Linked_To()->Coord.Z);
                    if (coord.Length() <= Linked_To()->Coord.Z - DestinationCoord.Z)
                        MissionState = RocketMissionState::ClosingIn;
                }

                Coordinate center_coord = Linked_To()->Center_Coord();
                double atan2 = FastMath::Atan2(center_coord.Y - DestinationCoord.Y, DestinationCoord.X - center_coord.X) - DEG_TO_RAD(90);
                Linked_To()->PrimaryFacing.Set_Desired(DirStruct(RAD_TO_BAU(atan2)));
            }
            else
            {
                Explode();
                return false;
            }
            break;
        }

    case RocketMissionState::ClosingIn:
        {
            if (Time_To_Explode(rocket))
                return false;

            const double pitch = Calculate_Pitch() - CurrentPitch;

            if (std::abs(pitch) > rocket->TurnRate)
                CurrentPitch = pitch < 0 ? CurrentPitch - rocket->TurnRate : CurrentPitch + rocket->TurnRate;
            else
                CurrentPitch += pitch;

            break;
        }

    case RocketMissionState::VerticalTakeOff:
        {
            IsSpawnerElite = spawn_owner && spawn_owner->Veterancy.Is_Elite();

            if (TrailerTimer.Expired())
            {
                if (rocket->TakeoffAnim)
                {
                    new AnimClass(rocket->TakeoffAnim, Linked_To()->Coord, 2, 1, SHAPE_WIN_REL | SHAPE_CENTER, -10);
                    TrailerTimer = 24;
                }
            }

            if (MissionTimer.Expired())
            {
                CurrentPitch = rocket->PitchFinal * DEG_TO_RAD(90);
                Sound_Effect(Linked_To()->Techno_Type_Class()->AuxSound1, Linked_To()->Coord);

                TrailerTimer = 0;
                MissionState = RocketMissionState::GainingAltitude;
            }
            else
            {
                Coordinate coord = Linked_To()->Coord;
                coord.Z += rocket->RaiseRate;
                if (Map.In_Radar(Coord_Cell(coord)))
                    Linked_To()->Set_Coord(coord);
            }
        }
        break;

    default:
        break;
    }

    if (Is_Moving_Now() && TrailerTimer.Expired() && rocket->TrailAnim)
    {
        new AnimClass(rocket->TrailAnim, Linked_To()->Coord, 2, 1, SHAPE_WIN_REL | SHAPE_CENTER);
        TrailerTimer = 3;
    }

    if (CurrentSpeed > 0.0)
    {
        Coordinate coord = Get_Next_Position(static_cast<int>(CurrentSpeed));

        if (Map.In_Radar(Coord_Cell(coord)))
            Linked_To()->Set_Coord(coord);

        if (Linked_To()->Strength <= 0)
            Explode();
    }

    return Is_Moving();
}


/**
 *  Instruct to move to location specified.
 * 
 *  @author: ZivDero
 */
IFACEMETHODIMP_(void) RocketLocomotionClass::Move_To(Coordinate to)
{
    const auto atype = reinterpret_cast<AircraftClass*>(Linked_To())->Class;
    const RocketTypeClass* rocket = RocketTypeClass::From_AircraftType(atype);

    if (!DestinationCoord)
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
        DestinationCoord = to;
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
    return LAYER_AIR;
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


RocketLocomotionClass::RocketMotionStruct RocketLocomotionClass::Get_Motion(double speed) const
{
    RocketMotionStruct motion;

    const double horizontal_speed = FastMath::Cos(CurrentPitch) * speed;
    const double horizontal_angle = BAU_TO_RAD(Linked_To()->PrimaryFacing.Current().Get_Raw() + DEG_TO_BAU(90));

    motion.X = static_cast<int>(Linked_To()->Coord.X + FastMath::Cos(horizontal_angle) * horizontal_speed);
    motion.Y = static_cast<int>(Linked_To()->Coord.Y - FastMath::Sin(horizontal_angle) * horizontal_speed);
    motion.Z = static_cast<int>(Linked_To()->Coord.Z + FastMath::Sin(CurrentPitch) * speed);

    return motion;
}


Coordinate RocketLocomotionClass::Get_Next_Position(double speed) const
{
    RocketMotionStruct motion = Get_Motion(speed);
    return { motion.X, motion.Y, motion.Z };
}


double RocketLocomotionClass::Calculate_Pitch() const
{
    /**
     *  Calculate how much is there left to go.
     */
    const Coordinate left_to_go = DestinationCoord - Linked_To()->Coord;
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
    /**
     *  Get the warhead this rocket carries.
     */
    const auto atype = reinterpret_cast<AircraftClass*>(Linked_To())->Class;
    const RocketTypeClass* rocket = RocketTypeClass::From_AircraftType(atype);
    const WarheadTypeClass* warhead = IsSpawnerElite ? rocket->EliteWarhead : rocket->Warhead;

    /**
     *  Calculate where it's moving right now.
     */
    Coordinate coord = Get_Next_Position(rocket->BodyLength);
    Cell cell = Coord_Cell(coord);

    /**
     *  The rocket uses its spawner's elite status to determine if it should deal elite damage.
     */
    int damage = IsSpawnerElite ? rocket->EliteDamage : rocket->Damage;

    /**
     *  KABOOM!!!
     */
    const auto animtype = Combat_Anim(damage, warhead, Map[cell].Land_Type(), &coord);
    if (animtype)
        new AnimClass(animtype, coord, 0, 1, SHAPE_WIN_REL | SHAPE_CENTER | SHAPE_FLAT, -15);
    Do_Flash(damage, warhead, coord);
    Explosion_Damage(&coord, damage, Linked_To(), warhead, true);
    delete Linked_To();
}



bool RocketLocomotionClass::Time_To_Explode(const RocketTypeClass* rocket)
{
    RocketMotionStruct motion = Get_Motion(rocket->BodyLength);

    /**
     *  Check if we're there yet.
     */
    if (motion.Z > DestinationCoord.Z)
    {
        CellClass* rocket_cell = Linked_To()->Get_Cell_Ptr();
        if (!rocket_cell || !rocket_cell->Bit2_16 /*might be Bit2_8*/ || DestinationCoord.Z != rocket_cell->Center_Coord().Z || motion.Z > DestinationCoord.Z + ROCKET_SPEED)
        {
            /**
             *  Nope, too early.
             */
            if (Linked_To()->Get_Height() > 0)
                return false;
        }
    }

    /**
     *  KABOOM!!!
     */
    Explode();
    return true;
}
