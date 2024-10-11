/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          KAMIKAZETRACKER.CPP
 *
 *  @authors       ZivDero
 *
 *  @brief         KamikazeTrackerClass reimplementation from YR.
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

#include "kamikazetracker.h"

#include "aircraft.h"
#include "aircrafttypeext.h"
#include "cell.h"
#include "extension.h"
#include "tibsun_inline.h"
#include "tibsun_globals.h"
#include "mouse.h"
#include "vinifera_globals.h"
#include "vinifera_saveload.h"


/**
 *  Basic constructor for the KamikazeTrackerClass.
 *
 *  @author: ZivDero
 */
KamikazeTrackerClass::~KamikazeTrackerClass()
{
    for (int i = 0; i < Controls.Count(); i++)
        delete Controls[i];
}


/**
 *  Loads the object from the stream and requests a new pointer to
 *  the class we extended post-load.
 *
 *  @author: ZivDero
 */
HRESULT KamikazeTrackerClass::Load(IStream* pStm)
{
    //EXT_DEBUG_TRACE("KamikazeTrackerClass::Load - 0x%08X\n", (uintptr_t)(this));

    if (!pStm) {
        return E_POINTER;
    }

    /**
     *  Load the unique id for this class.
     */
    LONG id = 0;
    HRESULT hr = pStm->Read(&id, sizeof(LONG), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    /**
     *  Register this instance to be available for remapping references to.
     */
    VINIFERA_SWIZZLE_REGISTER_POINTER(id, this, "KamikazeTracker");

    /**
     *  Read this class's binary blob data directly into this instance.
     */
    hr = pStm->Read(this, sizeof(*this), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    /**
     *  Read the count of active kamikaze controls.
     */
    int count;

    hr = pStm->Read(&count, sizeof(count), nullptr);
    if (FAILED(hr))
        return hr;

    new (&Controls) DynamicVectorClass<KamikazeControl*>();

    if (count <= 0)
        return hr;

    /**
     *  Read each of the controls as a binary blob.
     */
    for (int index = 0; index < count; ++index)
    {
        const auto control = new KamikazeControl;
        hr = pStm->Read(control, sizeof(KamikazeControl), nullptr);
        if (FAILED(hr))
            return hr;
        Controls.Add(control);
    }

    for (int index = 0; index < count; index++)
    {
        VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(Controls[index]->Aircraft, "Aircraft");
        VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(Controls[index]->Cell, "Cell");
    }

    return hr;
}


/**
 *  Saves the object to the stream.
 *
 *  @author: ZivDero
 */
HRESULT KamikazeTrackerClass::Save(IStream* pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("KamikazeTrackerClass::Save - 0x%08X\n", (uintptr_t)(this));

    if (!pStm) {
        return E_POINTER;
    }

    /**
     *  Fetch the save id for this instance.
     */
    const LONG id = reinterpret_cast<LONG>(this);

    //DEV_DEBUG_INFO("Writing id = 0x%08X.\n", id);

    HRESULT hr = pStm->Write(&id, sizeof(id), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    /**
     *  Write this class instance as a binary blob.
     */
    hr = pStm->Write(this, sizeof(*this), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    /**
     *  Write the count of active kamikaze controls.
     */
    const int count = Controls.Count();

    hr = pStm->Write(&count, sizeof(count), nullptr);
    if (FAILED(hr))
        return hr;

    if (count <= 0)
        return hr;

    /**
     *  Write each of the controls as a binary blob.
     */
    for (int index = 0; index < count; ++index)
    {
        hr = pStm->Write(Controls[index], sizeof(KamikazeControl), nullptr);
        if (FAILED(hr))
            return hr;
    }

    return hr;
}


/**
 *  Adds a new aircraft to the tracker.
 *
 *  @author: ZivDero
 */
void KamikazeTrackerClass::Add(AircraftClass* aircraft, TARGET target)
{
    if (!Extension::Fetch<AircraftTypeClassExtension>(aircraft->Techno_Type_Class())->IsMissileSpawn)
    {
        aircraft->On_Death(nullptr);
        return;
    }

    const auto control = new KamikazeControl();
    control->Aircraft = aircraft;
    control->Cell = target == nullptr ?
        &aircraft->Get_Cell_Ptr()->Adjacent_Cell(Dir_Facing(aircraft->PrimaryFacing.Current().Get_Dir())) :
        &Map[Coord_Cell(target->Center_Coord())];
    aircraft->IsKamikaze = true;
    aircraft->Ammo = 1;
    Controls.Add(control);
}


/**
 *  Processes the kamikaze tracker logic.
 *
 *  @author: ZivDero
 */
void KamikazeTrackerClass::AI()
{
    if (!UpdateTimer.Expired())
        return;

    UpdateTimer.Start();
    UpdateTimer = 30;

    for (int i = 0; i < Controls.Count(); i++)
    {
        const auto control = Controls[i];
        CellClass* cell = control->Cell;
        AircraftClass* aircraft = control->Aircraft;

        aircraft->Ammo = 1;
        if (cell)
            aircraft->Assign_Target(cell);
        else
            aircraft->Assign_Target(&aircraft->Get_Cell_Ptr()->Adjacent_Cell(Dir_Facing(aircraft->PrimaryFacing.Current().Get_Dir())));

        aircraft->Assign_Mission(MISSION_ATTACK);
    }
}


/**
 *  Removes an aircraft from the tracker.
 *
 *  @author: ZivDero
 */
void KamikazeTrackerClass::Detach(AircraftClass const* aircraft)
{
    for (int i = 0; i < Controls.Count(); i++)
    {
        const auto control = Controls[i];
        if (control->Aircraft == aircraft)
        {
            delete control;
            Controls.Delete(control);
            return;
        }
    }
}


/**
 *  Clears the tracker.
 *
 *  @author: ZivDero
 */
void KamikazeTrackerClass::Clear()
{
    for (int i = 0; i  < Controls.Count(); i++)
        delete Controls[i];

    Controls.Clear();
    UpdateTimer.Start();
    UpdateTimer = 1;
}
