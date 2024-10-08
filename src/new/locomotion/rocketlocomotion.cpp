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
    MissionState(RocketMissionState::State_0),
    CurrentSpeed(0),
    unknown_bool_4C(true),
    IsSpawnerElite(false),
    CurrentPitch(0.0),
    unknown_58(0),
    unknown_5C(0)
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

    const float z_angle = (Dir_To_32(LinkedTo->PrimaryFacing.Current()) - 8) * -WWMATH_P16;
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
        *key |= Dir_To_32(LinkedTo->PrimaryFacing.Current());
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

    TechnoClass* spawn_owner = Extension::Fetch<AircraftClassExtension>(LinkedTo)->Spawner;

    switch (MissionState)
    {
    case RocketMissionState::Pause:
        CurrentSpeed = 0;
        IsSpawnerElite = spawn_owner && spawn_owner->Veterancy.Is_Elite();

        // if ( rocket->Type != Rule->CMisl.Type )
        unknown_bool_4C = true;
        if (MissionTimer.Get_Rate())
        {
            MissionTimer = 
        }

    }
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
    return MissionState >= RocketMissionState::State_3 && MissionState <= RocketMissionState::State_5;
}


RocketLocomotionClass::RocketMotionStruct RocketLocomotionClass::Get_Motion(int rocket_length)
{
    RocketMotionStruct motion;

    motion.VerticalSpeed = static_cast<int>(FastMath::Sin(CurrentPitch) * static_cast<double>(rocket_length) + LinkedTo->Coord.Z);
    motion.HorizontalSpeed = static_cast<int>(FastMath::Cos(CurrentPitch) * static_cast<double>(rocket_length));

    const int facing_angle = BAU_TO_RAD(LinkedTo->PrimaryFacing.Current().Get_Raw() + DEG_TO_BAU(90));
    motion.X = static_cast<int>(LinkedTo->Coord.X + FastMath::Cos(facing_angle) * static_cast<double>(motion.HorizontalSpeed));
    motion.Y = static_cast<int>(LinkedTo->Coord.Y - FastMath::Sin(facing_angle) * static_cast<double>(motion.HorizontalSpeed));

    return motion;
}



Coordinate RocketLocomotionClass::Get_Next_Position(int rocket_length)
{
    RocketMotionStruct motion = Get_Motion(rocket_length);
    return Coordinate(motion.X, motion.Y, motion.VerticalSpeed);
}


double RocketLocomotionClass::Calculate_Pitch()
{
    /**
     *  Calculate how much is there left to go.
     */
    Coordinate left_to_go = DestinationCoord - LinkedTo->Coord;
    double length = Vector2(left_to_go.X, left_to_go.Y).Length();

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
     *  The rocket uses it's spawner's elite status to determine if it should deal elite damage.
     */
    int damage = IsSpawnerElite ? rocket->EliteDamage : rocket->Damage;

    /**
     *  KABOOM!!!
     */
    constexpr int zadjust = -15; // Combat_ZAdjust
    AnimClass anim(Combat_Anim(damage, warhead, Map[cell].Land_Type(), &coord), coord, 0, 1, SHAPE_WIN_REL | SHAPE_CENTER | SHAPE_FLAT, zadjust);
    Do_Flash(damage, const_cast<WarheadTypeClass*>(warhead), coord);
    Explosion_Damage(&coord, damage, LinkedTo, warhead, true);
    delete LinkedTo;
}



bool RocketLocomotionClass::Time_To_Explode(const RocketTypeClass* rocket)
{
    RocketMotionStruct motion = Get_Motion(rocket->BodyLength);

    /**
     *  Check if we're there yet.
     */
    if (motion.VerticalSpeed > DestinationCoord.Z)
    {
        CellClass* rocket_cell = LinkedTo->Get_Cell_Ptr();
        if (!rocket_cell || !rocket_cell->Bit2_16 /*might be Bit2_8*/ || DestinationCoord.Z != rocket_cell->Center_Coord().Z || motion.VerticalSpeed > DestinationCoord.Z + ROCKET_SPEED)
        {
            /**
             *  Nope, too early.
             */
            if (LinkedTo->Get_Height() > 0)
                return false;
        }
    }

    /**
     *  KABOOM!!!
     */
    Explode();
    return true;
}
