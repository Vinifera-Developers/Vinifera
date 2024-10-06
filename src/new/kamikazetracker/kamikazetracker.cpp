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


KamikazeTrackerClass::~KamikazeTrackerClass()
{
    for (int i = Controls.Count() - 1; i >= 0; i--)
    {
        if (this->Controls[i] != nullptr)
            delete this->Controls[i];
    }
}


void KamikazeTrackerClass::Add(AircraftClass* aircraft, TARGET target)
{
    if (!Extension::Fetch<AircraftTypeClassExtension>(aircraft->Techno_Type_Class())->IsMissileSpawn)
    {
        aircraft->On_Death(nullptr);
        return;
    }

    KamikazeControl* control = new KamikazeControl();
    control->Aircraft = aircraft;
    control->Cell = target == nullptr ?
        &aircraft->Get_Cell_Ptr()->Adjacent_Cell(Dir_Facing(aircraft->PrimaryFacing.Current().Get_Dir())) :
        &Map[Coord_Cell(target->Center_Coord())];
    aircraft->IsKamikaze = true;
    aircraft->Ammo = 1;
    Controls.Add(control);

}

void KamikazeTrackerClass::AI()
{
    if (!UpdateTimer.Expired())
        return;

    UpdateTimer.Start();
    UpdateTimer = 30;

    for (int i = 0; i < Controls.Count(); i++)
    {
        KamikazeControl* control = Controls[i];
        CellClass* cell = control->Cell;
        AircraftClass* aircraft = control->Aircraft;

        aircraft->Ammo = 1;
        if (cell != nullptr)
            aircraft->Assign_Target(cell);
        else
            aircraft->Assign_Target(&aircraft->Get_Cell_Ptr()->Adjacent_Cell(Dir_Facing(aircraft->PrimaryFacing.Current().Get_Dir())));

        aircraft->Assign_Mission(MISSION_ATTACK);
    }
}
void KamikazeTrackerClass::Detach(AircraftClass const* aircraft)
{
    KamikazeControl* control = nullptr;
    for (int i = Controls.Count() - 1; i >= 0; i--)
    {
        control = Controls[i];
        if (control->Aircraft == aircraft)
        {
            CellClass* cell = &Map[Coord_Cell(aircraft->Center_Coord())];
            control->Aircraft->Assign_Target(cell);
            control->Aircraft->Assign_Mission(MISSION_ATTACK);
            control->Cell = cell;
            return;
        }
    }

    if (control != nullptr)
    {
        delete control;
        Controls.Delete(control);
    }
}
void KamikazeTrackerClass::Clear()
{
    for (int i = Controls.Count() - 1; i >= 0; i--)
        delete this->Controls[i];

    Controls.Clear();
    UpdateTimer.Start();
    UpdateTimer = 1;
}
