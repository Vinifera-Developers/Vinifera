/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ROCKETLOCOMOTION.H
 *
 *  @authors       CCHyper
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
#pragma once

#include "always.h"
#include "locomotion.h"
#include "rockettype.h"
#include "vinifera_defines.h"


enum class RocketMissionState
{
    State_0 = 0,
    Pause = 1,
    Tilt = 2,
    State_3 = 3,
    State_4 = 4,
    State_5 = 5,
};

#define ROCKET_SPEED 416

struct RocketMotionStruct;

class DECLSPEC_UUID(CLSID_ROCKET_LOCOMOTOR)
RocketLocomotionClass : public LocomotionClass
{
public:
    struct RocketMotionStruct
    {
        int VerticalSpeed;
        int HorizontalSpeed;
        int X;
        int Y;
    };

public:
    /**
     *  IPersist
     */
    IFACEMETHOD(GetClassID)(CLSID* pClassID);

    /**
     *  IPersistStream
     */
    IFACEMETHOD(Load)(IStream* pStm);

    /**
     *  ILocomotion
     */
    IFACEMETHOD_(bool, Is_Moving)();
    IFACEMETHOD_(Coordinate, Destination)();
    IFACEMETHOD_(Matrix3D, Draw_Matrix)(int *key);
    IFACEMETHOD_(bool, Process)();
    IFACEMETHOD_(void, Move_To)(Coordinate to);
    IFACEMETHOD_(void, Stop_Moving)();
    IFACEMETHOD_(LayerType, In_Which_Layer)();
    IFACEMETHOD_(bool, Is_Moving_Now)();

    /**
     *  LocomotionClass
     */
    virtual int Size_Of(bool firestorm = false) const override { return sizeof(*this); }

private:
    /**
     *  RocketLocomotionClass
     */
    Coordinate Get_Next_Position(int rocket_length);
    double Calculate_Pitch();
    void Explode();
    bool Time_To_Explode(const RocketTypeClass* rocket);
    RocketMotionStruct Get_Motion(int rocket_length);

public:
    RocketLocomotionClass();
    ~RocketLocomotionClass() = default;
    
protected:
    /**
     *  This is the desired destination coordinate of the rocket.
     */
    Coordinate DestinationCoord;

    CDRateTimerClass<FrameTimerClass> MissionTimer;
    CDTimerClass<FrameTimerClass> TrailerTimer;
    RocketMissionState MissionState;
    DWORD unknown_44;
    double CurrentSpeed;
    bool unknown_bool_4C;
    bool IsSpawnerElite;
    double CurrentPitch;
    DWORD unknown_58;
    DWORD unknown_5C;
};
