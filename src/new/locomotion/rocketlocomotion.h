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
    None = 0,
    Pause = 1,
    Tilt = 2,
    GainingAltitude = 3,
    Flight = 4,
    ClosingIn = 5,
    VerticalTakeOff = 6,
};

#define ROCKET_SPEED 416

class DECLSPEC_UUID(CLSID_ROCKET_LOCOMOTOR)
RocketLocomotionClass : public LocomotionClass
{
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
    IFACEMETHOD_(Point2D, Shadow_Point)();
    IFACEMETHOD_(bool, Process)();
    IFACEMETHOD_(void, Move_To)(Coordinate to);
    IFACEMETHOD_(void, Stop_Moving)();
    IFACEMETHOD_(LayerType, In_Which_Layer)();
    IFACEMETHOD_(bool, Is_Moving_Now)();

    RocketLocomotionClass();
    ~RocketLocomotionClass() override = default;

    /**
     *  LocomotionClass
     */
    virtual int Size_Of(bool firestorm = false) const override { return sizeof(*this); }

private:
    /**
     *  RocketLocomotionClass
     */
    Coordinate Get_Next_Position(double speed) const;
    double Get_Next_Pitch() const;
    void Explode();
    bool Time_To_Explode(const RocketTypeClass* rocket);

public:
    RocketLocomotionClass(const RocketLocomotionClass&) = delete;
    RocketLocomotionClass(const NoInitClass& noinit);
    RocketLocomotionClass& operator=(const RocketLocomotionClass&) = delete;
    
protected:
    /**
     *  This is the desired destination coordinate of the rocket.
     */
    Coordinate DestinationCoord;

    /**
     *  This is the timer used by various mission states of the rocket.
     */
    CDRateTimerClass<FrameTimerClass> MissionTimer;

    /**
     *  This is the timer used for timing the trail animation.
     */
    CDTimerClass<FrameTimerClass> TrailTimer;

    /**
     *  The current state of the rocket.
     */
    RocketMissionState MissionState;

    /**
     *  The current speed of the rocket.
     */
    double CurrentSpeed;

    /**
     *  This boolean gets used to determine if the rocket needs to be submit to DisplayClass.
     */
    bool NeedToSubmit;

    /**
     *  Is this rocket's spawner elite?
     */
    bool IsSpawnerElite;

    /**
     *  The current pitch of the rocket.
     */
    double CurrentPitch;

    /**
     *  The distance to the destination from when the rocket has reached its desired altitude.
     */
    int ApogeeDistance;
};
