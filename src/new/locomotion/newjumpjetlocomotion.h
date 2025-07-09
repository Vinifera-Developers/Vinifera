/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          NEWJUMPJETLOCOMOTION.H
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
#pragma once

#include "locomotion.h"
#include "facing.h"
#include "vinifera_defines.h"


class DECLSPEC_UUID(CLSID_NEWJUMPJET_LOCOMOTOR)
NewJumpjetLocomotionClass : public LocomotionClass
{
        typedef LocomotionClass BASECLASS;

    public:

        /**
         *  IPersist methods.
         */
        IFACEMETHOD_(HRESULT, GetClassID)(CLSID * retval) override;

        /**
         *  IPersistStream methods.
         */
        IFACEMETHOD_(HRESULT, Load)(IStream * stream) override;
        
        /**
         *  ILocomotion methods.
         */
        IFACEMETHOD(Link_To_Object)(void* object);
        IFACEMETHOD_(bool, Is_Moving)() override;
        IFACEMETHOD_(Coordinate, Destination)() override;
        IFACEMETHOD_(Coordinate, Head_To_Coord)() override;
        IFACEMETHOD_(bool, Process)() override;
        IFACEMETHOD_(void, Move_To)(Coordinate to) override;
        IFACEMETHOD_(void, Stop_Moving)() override;
        IFACEMETHOD_(void, Do_Turn)(DirType coord) override;
        IFACEMETHOD_(LayerType, In_Which_Layer)() override;
        IFACEMETHOD_(bool, Is_Moving_Now)() override;
        IFACEMETHOD_(void, Mark_All_Occupation_Bits)(MarkType mark) override;

        /**
         *  LocomotionClass methods.
         */
        virtual int Get_Object_Size(bool oldsave = false) const override {return sizeof(*this);}

        /**
         *  Constructors, Destructors, and overloaded operators.
         */
        NewJumpjetLocomotionClass();
        NewJumpjetLocomotionClass(NoInitClass const & x) : BASECLASS(x), Facing(x) {}
        virtual ~NewJumpjetLocomotionClass() override = default;

        /**
         *  Member function prototypes.
         */
        void Process_Grounded();
        void Process_Ascent();
        void Process_Hover();
        void Process_Cruise();
        void Process_Descent();
        void Movement_AI();
        Coordinate Closest_Free_Spot(Coordinate const & to) const;
        int Desired_Flight_Level() const;

    private:
        /**
         *  Turn rate of the jumpjet unit.
         */
        int JumpjetTurnRate;

        /**
         *  Maximum forward speed of the jumpjet unit in leptons per tick.
         */
        int JumpjetSpeed;

        /**
         *  Vertical climb rate per tick for jumpjet ascent and descent.
         */
        double JumpjetClimb;

        /**
         *  Desired cruising height (in leptons) for the jumpjet unit.
         */
        int JumpjetCruiseHeight;

        /**
         *  Acceleration applied when changing the jumpjet's movement speed.
         */
        double JumpjetAcceleration;

        /**
         *  Frequency in Hz of the jumpjet's wobble effect while flying.
         */
        double JumpjetWobblesPerSecond;

        /**
         *  Maximum deviation of jumpjet vertical wobble (in leptons).
         */
        int JumpjetWobbleDeviation;

        /**
         *  Whether this jumpjet unit doesn't wobble during flight.
         */
        bool JumpjetNoWobbles;

        /**
         *  Radius (in cells) within which the jumpjet can detect cloaked units.
         */
        int JumpjetCloakDetectionRadius;

        /**
         *  The destination coordinate the jumpjet is moving toward.
         */
        Coordinate HeadToCoord;

        /**
         *  Indicates whether the jumpjet is currently moving toward a target.
         */
        bool IsMoving;

        /**
         *  Represents the current flight state of the jumpjet.
         *  - GROUNDED: Landed and idle
         *  - ASCENDING: Taking off
         *  - HOVERING: Stationary at cruise height
         *  - CRUISING: Moving toward destination at cruise height
         *  - DESCENDING: Attempting to land
         */
        enum {
            GROUNDED,
            ASCENDING,
            HOVERING,
            CRUISING,
            DESCENDING
        } CurrentState;

        /**
         *  Current facing direction of the jumpjet.
         */
        FacingClass Facing;

        /**
         *  Current movement speed of the jumpjet, in leptons per tick.
         */
        double CurrentSpeed;

        /**
         *  Desired speed the jumpjet is accelerating or decelerating toward.
         */
        double TargetSpeed;

        /**
         *  Desired height above ground level for the jumpjet during cruise.
         */
        int FlightLevel;

        /**
         *  Phase value used to compute wobble displacement during flight.
         */
        double CurrentWobble;

        /**
         *  Indicates whether the jumpjet is currently in the process of landing.
         */
        bool IsLanding;
};
