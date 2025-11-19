/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera (Dawn of the Tiberium Age Build)
 *
 *  @file          PARTICLEEXT_HOOKS.CPP
 *
 *  @author        Rampastring
 *
 *  @brief         Contains the hooks for the extended ParticleClass.
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
#include "aircraft.h"
#include "building.h"
#include "house.h"
#include "housetype.h"
#include "particlesysext_hooks.h"
#include "particlesys.h"
#include "rules.h"
#include "scenario.h"
#include "unit.h"
#include "tibsun_globals.h"
#include "tibsun_defines.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "syringe.h"


UnitClass* Create_Visceroid(ObjectClass* destroyedobject)
{
    if (destroyedobject->RTTI == RTTI_INFANTRY || (destroyedobject->Is_Techno() && destroyedobject->TClass->IsCrew)) {
        return new UnitClass(Rule->SmallVisceroid, House_From_HousesType(HouseTypeClass::From_Name("Neutral")));
    }

    return nullptr;
}


/**
 *  Fixes a bug where gas clouds are able to turn everything into visceroids, including
 *  non-crewed vehicles and even terrain objects.
 *
 *  @author: Rampastring
 */
DEFINE_HOOK(0x005A389C, _ParticleClass_Smoke_And_WeakGas_Behaviour_AI_Tiberium_Death_Patch, 0)
{
    GET(ObjectClass*, destroyedobject, ESI);
    GET(ResultType, result, EAX);
    GET_STACK(ObjectClass*, nextobject, 0x40);

    enum {
        ContinueVisceroidPlacement = 0x005A38FC,
        SkipToNextObjectOnCell = 0x005A3965
    };

    R->ESI(nextobject);

    if (result != RESULT_DESTROYED) {
        // Object was not destroyed, do not create visceroid.
        return SkipToNextObjectOnCell;
    }

    if (!Scen->IsTiberiumDeathToVisceroid) {
        // Visceroids spawning from Tiberium death is disabled, do not create visceroid.
        return SkipToNextObjectOnCell;
    }

    UnitClass* visceroid = Create_Visceroid(destroyedobject);
    if (visceroid == nullptr) {
        // No visceroid was created.
        return SkipToNextObjectOnCell;
    }

    R->EDI(visceroid);
    return ContinueVisceroidPlacement;
}


/**
 *  Main function for patching the hooks.
 */
void ParticleClassExtension_Hooks()
{
    
}

