/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TESTLOCOMOTION.H
 *
 *  @authors       CCHyper
 *
 *  @brief         Test locomotion implementation for demonstration purposes.
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
#include "vinifera_defines.h"


class DECLSPEC_UUID(CLSID_TEST_LOCOMOTOR)
TestLocomotionClass : public LocomotionClass
{
    public:
        /**
         *  IUnknown
         */
        IFACEMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObj);
        IFACEMETHOD_(ULONG, AddRef)();
        IFACEMETHOD_(ULONG, Release)();

        /**
         *  IPersist
         */
        IFACEMETHOD(GetClassID)(CLSID *pClassID);
        
        /**
         *  IPersistStream
         */
        IFACEMETHOD_(LONG, IsDirty)();
        IFACEMETHOD(Load)(IStream *pStm);
        IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);
        IFACEMETHOD_(LONG, GetSizeMax)(ULARGE_INTEGER *pcbSize);

        /**
         *  ILocomotion
         */
        IFACEMETHOD(Link_To_Object)(void *object);
        IFACEMETHOD_(bool, Is_Moving)();
        IFACEMETHOD_(Coordinate, Destination)();
        IFACEMETHOD_(Coordinate, Head_To_Coord)();
        IFACEMETHOD_(MoveType, Can_Enter_Cell)(Cell cell);
        IFACEMETHOD_(bool, Is_To_Have_Shadow)();
        IFACEMETHOD_(Matrix3D, Draw_Matrix)(int *key);
        IFACEMETHOD_(Matrix3D, Shadow_Matrix)(int *key);
        IFACEMETHOD_(Point2D, Draw_Point)();
        IFACEMETHOD_(Point2D, Shadow_Point)();
        IFACEMETHOD_(VisualType, Visual_Character)(bool flag);
        IFACEMETHOD_(int, Z_Adjust)();
        IFACEMETHOD_(ZGradientType, Z_Gradient)();
        IFACEMETHOD_(bool, Process)();
        IFACEMETHOD_(void, Move_To)(Coordinate to);
        IFACEMETHOD_(void, Stop_Moving)();
        IFACEMETHOD_(void, Do_Turn)(DirStruct coord);
        IFACEMETHOD_(void, Unlimbo)();
        IFACEMETHOD_(void, Tilt_Pitch_AI)();
        IFACEMETHOD_(bool, Power_On)();
        IFACEMETHOD_(bool, Power_Off)();
        IFACEMETHOD_(bool, Is_Powered)();
        IFACEMETHOD_(bool, Is_Ion_Sensitive)();
        IFACEMETHOD_(bool, Push)(DirStruct dir);
        IFACEMETHOD_(bool, Shove)(DirStruct dir);
        IFACEMETHOD_(void, Force_Track)(int track, Coordinate coord);
        IFACEMETHOD_(LayerType, In_Which_Layer)();
        IFACEMETHOD_(void, Force_Immediate_Destination)(Coordinate coord);
        IFACEMETHOD_(void, Force_New_Slope)(int ramp);
        IFACEMETHOD_(bool, Is_Moving_Now)();
        IFACEMETHOD_(int, Apparent_Speed)();
        IFACEMETHOD_(int, Drawing_Code)();
        IFACEMETHOD_(FireErrorType, Can_Fire)();
        IFACEMETHOD_(int, Get_Status)();
        IFACEMETHOD_(void, Acquire_Hunter_Seeker_Target)();
        IFACEMETHOD_(bool, Is_Surfacing)();
        IFACEMETHOD_(void, Mark_All_Occupation_Bits)(int mark);
        IFACEMETHOD_(bool, Is_Moving_Here)(Coordinate to);
        IFACEMETHOD_(bool, Will_Jump_Tracks)();
        IFACEMETHOD_(bool, Is_Really_Moving_Now)();
        IFACEMETHOD_(void, Stop_Movement_Animation)();
        IFACEMETHOD_(void, Lock)();
        IFACEMETHOD_(void, Unlock)();
        IFACEMETHOD_(int, Get_Track_Number)();
        IFACEMETHOD_(int, Get_Track_Index)();
        IFACEMETHOD_(int, Get_Speed_Accum)();

    public:
        virtual int Size_Of(bool firestorm = false) const override { return sizeof(*this); }

    public:
        TestLocomotionClass();
        ~TestLocomotionClass();
        
    protected:
        /**
         *  This is the desired destination coordinate of the object.
         */
        Coordinate DestinationCoord;

        /**
         *  This is the coordinate that the unit is heading to as an immediate
         *  destination. This coordinate is never further than once cell (or track)
         *  from the unit's location. When this coordinate is reached, then the
         *  next location in the path list becomes the next HeadTo coordinate.
         */
        Coordinate HeadToCoord;

        /**
         *  This is the logical coordinate for the object. It is the center of
         *  the circle when calculating the rotation.
         */
        Coordinate CenterCoord;

        /**
         *  The current rotation angle.
         */
        double Angle;

        /**
         *  If this object is moving, then this flag will be true.
         */
        bool IsMoving;
};
