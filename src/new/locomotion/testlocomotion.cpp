/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TESTLOCOMOTION.CPP
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
#include "testlocomotion.h"
#include "tibsun_inline.h"
#include "tibsun_globals.h"
#include "iomap.h"
#include "cell.h"
#include "foot.h"
#include "tactical.h"
#include "wwmath.h"
#include "debughandler.h"


/**
 *  Retrieves pointers to the supported interfaces on an object.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP TestLocomotionClass::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    return LocomotionClass::QueryInterface(riid, ppvObj);
}


/**
 *  Increments the reference count for an interface pointer to a COM object.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(ULONG) TestLocomotionClass::AddRef()
{
    return LocomotionClass::AddRef();
}


/**
 *  Decrements the reference count for an interface on a COM object.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(ULONG) TestLocomotionClass::Release()
{
    return LocomotionClass::Release();
}


/**
 *  Determines whether an object has changed since it was last saved to its stream.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(LONG) TestLocomotionClass::IsDirty()
{
    return LocomotionClass::IsDirty();
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP TestLocomotionClass::GetClassID(CLSID *pClassID)
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
 * 
 *  @param      pStm           An IStream pointer to the stream from which the object should be loaded.
 */
IFACEMETHODIMP TestLocomotionClass::Load(IStream *pStm)
{
    HRESULT hr = LocomotionClass::Locomotion_Load(pStm);
    if (SUCCEEDED(hr)) {
        // Insert any data to be loaded here.
    }

    return hr;
}


/**
 *  Saves an object to the specified stream.
 * 
 *  @author: CCHyper
 * 
 *  @param      pStm           An IStream pointer to the stream into which the object should be saved.
 * 
 *  @param      fClearDirty    Indicates whether to clear the dirty flag after the save is complete.
 */
IFACEMETHODIMP TestLocomotionClass::Save(IStream *pStm, BOOL fClearDirty)
{
    HRESULT hr = LocomotionClass::Save(pStm, fClearDirty);
    if (SUCCEEDED(hr)) {
        // Insert any data to be saved here.
    }

    return hr;
}


/**
 *  Retrieves the size of the stream needed to save the object.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(LONG) TestLocomotionClass::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    if (pcbSize == nullptr) {
        return E_POINTER;
    }

    HRESULT hr = LocomotionClass::GetSizeMax(pcbSize);

    return S_OK;
}


/**
 *  Class default constructor.
 * 
 *  @author: CCHyper
 */
TestLocomotionClass::TestLocomotionClass() :
    LocomotionClass(),
    HeadToCoord(),
    DestinationCoord(),
    CenterCoord(),
    Angle(0),
    IsMoving(false)
{
}


/**
 *  Class destructor.
 * 
 *  @author: CCHyper
 */
TestLocomotionClass::~TestLocomotionClass()
{
}


/**
 *  Links object to locomotor.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP TestLocomotionClass::Link_To_Object(void *object)
{
    HRESULT hr = LocomotionClass::Link_To_Object(object);

    if (SUCCEEDED(hr)) {
        DEBUG_INFO("TestLocomotionClass - Sucessfully linked to \"%s\"\n", LinkedTo->Name());
    }

    return hr;
}


/**
 *  Sees if object is moving.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(bool) TestLocomotionClass::Is_Moving()
{
    return IsMoving;
}


/**
 *  Fetches destination coordinate.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(Coordinate) TestLocomotionClass::Destination()
{
    if (IsMoving) {
        return DestinationCoord;
    }

    return Coordinate(0, 0, 0);
}


/**
 *  Fetches immediate (next cell) destination coordinate.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(Coordinate) TestLocomotionClass::Head_To_Coord()
{
    /**
     *  If currently moving, return the immediate coordinate.
     */
    if (IsMoving) {
        return HeadToCoord;
    }

    /**
     *  Return the current coordinate.
     */
    return Linked_To()->Get_Coord();
}


/**
 *  Determine if specific cell can be entered.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(MoveType) TestLocomotionClass::Can_Enter_Cell(Cell cell)
{
    /**
     *  Query the linked object to determine if the cell can be entered.
     */
    return Linked_To()->Can_Enter_Cell(&Map[cell]);
}


/**
 *  Should object cast a shadow?
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(bool) TestLocomotionClass::Is_To_Have_Shadow()
{
    return LocomotionClass::Is_To_Have_Shadow();
}


/**
 *  Fetch voxel draw matrix.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(Matrix3D) TestLocomotionClass::Draw_Matrix(int *key)
{
    return LocomotionClass::Draw_Matrix(key);
}


/**
 *  Fetch shadow draw matrix.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(Matrix3D) TestLocomotionClass::Shadow_Matrix(int *key)
{
    return LocomotionClass::Shadow_Matrix(key);
}


/**
 *  Draw point center location.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(Point2D) TestLocomotionClass::Draw_Point()
{
    return LocomotionClass::Draw_Point();
}


/**
 *  Shadow draw point center location.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(Point2D) TestLocomotionClass::Shadow_Point()
{
    return LocomotionClass::Shadow_Point();
}


/**
 *  Visual character for drawing.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(VisualType) TestLocomotionClass::Visual_Character(bool flag)
{
    return VISUAL_NORMAL;
}


/**
 *  Z adjust control value.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(int) TestLocomotionClass::Z_Adjust()
{
    return 0;
}


/**
 *  Z gradient control value.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(ZGradientType) TestLocomotionClass::Z_Gradient()
{
    return ZGRAD_90DEG;
}


/**
 *  Process movement of object.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(bool) TestLocomotionClass::Process()
{
    if (IsMoving) {

        Coordinate coord = DestinationCoord;

        /**
         *  Rotate the object around the center coord..
         */
        int radius = CELL_LEPTON_W*2;
        coord.X += radius * WWMath::Sin(Angle);
        coord.Y += radius * WWMath::Cos(Angle);
        //coord.Z // No need to adjust the height of the object.

        /**
         *  Pickup the object the game world before we set the new coord.
         */
        Linked_To()->Mark(MARK_UP);
        if (Can_Enter_Cell(Coord_Cell(coord)) == MOVE_OK) {
            Linked_To()->Set_Coord(coord);

            /**
             *  Increase the angle, wrapping if full circle is complete.
             */
            double scale = 360.0;
            Angle += DEG_TO_RAD(360.0) / scale;
            if (Angle > 360.0) {
                Angle = 0;
            }
        }
        Linked_To()->Mark(MARK_DOWN);
    }

    return Is_Moving();
}


/**
 *  Instruct to move to location specified.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(void) TestLocomotionClass::Move_To(Coordinate to)
{
    DestinationCoord = to;

    if (DestinationCoord == COORD_NONE) {
        if (HeadToCoord == COORD_NONE) {
            IsMoving = false;
        }

    } else {
        IsMoving = true;
    }
}


/**
 *  Stop moving at first opportunity.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(void) TestLocomotionClass::Stop_Moving()
{
    HeadToCoord = Coordinate();
    DestinationCoord = Coordinate();

    Angle = 0;

    IsMoving = false;
}


/**
 *  Try to face direction specified.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(void) TestLocomotionClass::Do_Turn(DirStruct coord)
{
    Linked_To()->PrimaryFacing.Set(coord);
}


/**
 *  Object is appearing in the world.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(void) TestLocomotionClass::Unlimbo()
{
    /**
     *  Set the objects ramp for redraw.
     */
    Force_New_Slope(Linked_To()->Get_Cell_Ptr()->Ramp);
}


/**
 *  Special tilting AI function.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(void) TestLocomotionClass::Tilt_Pitch_AI()
{
}


/**
 *  Locomotor becomes powered.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(bool) TestLocomotionClass::Power_On()
{
    return LocomotionClass::Power_On();
}


/**
 *  Locomotor loses power.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(bool) TestLocomotionClass::Power_Off()
{
    return LocomotionClass::Power_Off();
}


/**
 *  Is locomotor powered?
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(bool) TestLocomotionClass::Is_Powered()
{
    return LocomotionClass::Is_Powered();
}


/**
 *  Is locomotor sensitive to ion storms?
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(bool) TestLocomotionClass::Is_Ion_Sensitive()
{
    return false;
}


/**
 *  Push object in direction specified.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(bool) TestLocomotionClass::Push(DirStruct dir)
{
    return false;
}


/**
 *  Shove object (with spin) in direction specified.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(bool) TestLocomotionClass::Shove(DirStruct dir)
{
    return false;
}


/**
 *  Force drive track -- special case only.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(void) TestLocomotionClass::Force_Track(int track, Coordinate coord)
{
}


/**
 *  What display layer is it located in.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(LayerType) TestLocomotionClass::In_Which_Layer()
{
    return LAYER_GROUND;
}


/**
 *  Force object to destination (no processing).
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(void) TestLocomotionClass::Force_Immediate_Destination(Coordinate coord)
{
    DestinationCoord = coord;
}


/**
 *  Force a voxel unit to a given slope. Used in cratering.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(void) TestLocomotionClass::Force_New_Slope(int ramp)
{
}


/**
 *  Is it actually moving across the ground this very second?
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(bool) TestLocomotionClass::Is_Moving_Now()
{
    if (Linked_To()->PrimaryFacing.Is_Rotating()) {
        return true;
    }

    if (Is_Moving()) {
        return HeadToCoord != COORD_NONE && Apparent_Speed() > 0;
    }

    return false;
}


/**
 *  Actual current speed of object expressed as leptons per game frame.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(int) TestLocomotionClass::Apparent_Speed()
{
    return Linked_To()->Current_Speed();
}


/**
 *  Special drawing feedback code (locomotor specific meaning).
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(int) TestLocomotionClass::Drawing_Code()
{
    return 0;
}


/**
 *  Queries if any locomotor specific state prevents the object from firing.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(FireErrorType) TestLocomotionClass::Can_Fire()
{
    return FIRE_OK;
}


/**
 *  Queries the general state of the locomotor.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(int) TestLocomotionClass::Get_Status()
{
    return 0;
}


/**
 *  Forces a hunter seeker droid to find a target.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(void) TestLocomotionClass::Acquire_Hunter_Seeker_Target()
{
}


/**
 *  Is this object surfacing?
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(bool) TestLocomotionClass::Is_Surfacing()
{
    return false;
}


/**
 *  Lifts all occupation bits associated with the object off the map.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(void) TestLocomotionClass::Mark_All_Occupation_Bits(int mark)
{
    Coordinate headto = Head_To_Coord();
    if (mark != 0) {
        Linked_To()->Set_Occupy_Bit(headto);
    } else {
        Linked_To()->Clear_Occupy_Bit(headto);
    }
}


/**
 *  Is this object in the process of moving into this coord.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(bool) TestLocomotionClass::Is_Moving_Here(Coordinate to)
{
    return Coord_Cell(Head_To_Coord()) == Coord_Cell(to) && std::abs(Head_To_Coord().Z - to.Z) <= LEVEL_LEPTON_H;
}


/**
 *  Will this object jump tracks?
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(bool) TestLocomotionClass::Will_Jump_Tracks()
{
    return false;
}


/**
 *  Infantry moving query function.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(bool) TestLocomotionClass::Is_Really_Moving_Now()
{
    return IsMoving;
}


/**
 *  Falsifies the IsReallyMoving flag in WalkLocomotionClass.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(void) TestLocomotionClass::Stop_Movement_Animation()
{
}


/**
 *  Locks the locomotor from being deleted.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(void) TestLocomotionClass::Lock()
{
}


/**
 *  Unlocks the locomotor from being deleted.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(void) TestLocomotionClass::Unlock()
{
}


/**
 *  Queries internal variables.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(int) TestLocomotionClass::Get_Track_Number()
{
    return -1;
}


/**
 *  Queries internal variables.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(int) TestLocomotionClass::Get_Track_Index()
{
    return -1;
}


/**
 *  Queries internal variables.
 * 
 *  @author: CCHyper
 */
IFACEMETHODIMP_(int) TestLocomotionClass::Get_Speed_Accum()
{
    return -1;
}
