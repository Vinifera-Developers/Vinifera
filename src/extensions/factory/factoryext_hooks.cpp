/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          FACTORYEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended FactoryClass.
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
#include "factoryext_hooks.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"
#include "house.h"
#include "factory.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "extension_globals.h"
#include "factoryext_init.h"
#include "techno.h"
#include "technotype.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "houseext.h"
#include "mouse.h"
#include "rulesext.h"
#include "sidebarext.h"
#include "unittypeext.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static DECLARE_EXTENDING_CLASS_AND_PAIR(FactoryClass)
{
public:
    void _Sanitize_Queue();
    void _AI();
    bool _Start(bool suspend);
    void _Resume_Queue();
    bool _Abandon();
};


/**
 *  Checks if this factory should abandon construction because the objects
 *  in the queue are no longer available to build.
 *
 *  @author: ZivDero
 */
void FactoryClassExt::_Sanitize_Queue()
{
    const TechnoClass* producing_object = Get_Object();

    if (producing_object == nullptr) {
        return;
    }

    const TechnoTypeClass* producing_type = producing_object->TClass;
    const RTTIType type = producing_type->RTTI;
    const bool is_building = type == RTTI_BUILDING || type == RTTI_BUILDINGTYPE;

    bool need_update = false;

    // Check the thing we're currently building
    if (!House->Can_Build(producing_type, false, true)) {
        Abandon();
        need_update = true;

        // Cancel map placement
        if (is_building && House == PlayerPtr) {
            Map.PendingObject = nullptr;
            Map.PendingObjectPtr = nullptr;
            Map.PendingHouse = HOUSE_NONE;
            Map.Set_Cursor_Shape(nullptr);
        }
    }

    // Make sure there are no unavailable objects in the queue
    for (int i = 0; i < QueuedObjects.Count(); i++) {
        if (!House->Can_Build(QueuedObjects[i], false, true)) {
            Remove_From_Queue(*QueuedObjects[i]);
            need_update = true;
            i--;
        }
    }

    if (need_update) {
        if (House == PlayerPtr) {
            SidebarExtension->Flag_Strip_To_Redraw(type, TechnoTypeClassExtension::Get_Production_Flags(producing_type));
        }

        House->Update_Factories(type);
        Resume_Queue();
    }
}


/**
 *  Reimplements the entire FactoryClass::Start function.
 *  Fixes an issue where if you started construction with < Cost_Per_Tick() credits,
 *  it would be instantly put on hold.
 *
 *  @author: ZivDero
 */
bool FactoryClassExt::_Start(bool suspend)
{
    if ((Object || SpecialItem) && IsSuspended && !Has_Completed()) {
        const int time = Object ? Object->Time_To_Build() : 0;
        int rate = time / STEP_COUNT;
        rate = std::clamp(rate, 1, 255);

        Set_Rate(rate);
        IsSuspended = false;

        if (House->Available_Money() >= Cost_Per_Tick()) {
            IsOnHold = true;
            if (suspend) {
                Suspend(true);
            }

            return true;
        }
    }

    return false;
}


/**
 *  Reimplements the entire FactoryClass::AI function.
 *
 *  @author: ZivDero
 */
void FactoryClassExt::_AI()
{
    /**
     *  If sticky techs are disabled, clear anything that's no longer available from the build queue.
     */
    if (RuleExtension->IsRecheckPrerequisites) {
        _Sanitize_Queue();
    }

    if (!IsSuspended && (Object != nullptr || SpecialItem)) {
        if (!Has_Completed() && Graphic_Logic()) {
            IsDifferent = true;

            int cost = Cost_Per_Tick();
            cost = std::min(cost, Balance);

            /*
            **  Enough time has expired so that another production step can occur.
            **  If there is insufficient funds, then go back one production step and
            **  continue the countdown. The idea being that by the time the next
            **  production step occurs, there may be sufficient funds available.
            */
            if (cost > House->Available_Money()) {
                Set_Stage(Fetch_Stage() - 1);
            }
            else {
                House->Spend_Money(cost);
                Balance -= cost;
            }

            /**
             *  Patch for InstantBuildCommandClass
             *
             *  @author: CCHyper
             */
            if (Vinifera_DeveloperMode) {
                /*
                **  If AIInstantBuild is toggled on, make sure this is a non-human AI house.
                */
                if (Vinifera_Developer_AIInstantBuild
                    && !House->Is_Human_Player() && House != PlayerPtr) {
                    Set_Stage(STEP_COUNT);
                }

                /*
                **  If InstantBuild is toggled on, make sure the local player is a human house.
                */
                if (Vinifera_Developer_InstantBuild
                    && House->Is_Human_Player() && House == PlayerPtr) {
                    Set_Stage(STEP_COUNT);
                }

                /*
                **  If the AI has taken control of the player house, it needs a special
                **  case to handle the "player" instant build mode.
                */
                if (Vinifera_Developer_InstantBuild) {
                    if (Vinifera_Developer_AIControl && House == PlayerPtr)
                        Set_Stage(STEP_COUNT);
                }

            }

            /*
            **  If the production has completed, then suspend further production.
            */
            if (Fetch_Stage() == STEP_COUNT) {
                IsSuspended = true;
                Set_Rate(0);
                House->Spend_Money(Balance);
                Balance = 0;
            }
        }
    }
}


/**
 *  Reimplementation of FactoryClass::Resume_Queue.
 *
 *  @author: ZivDero
 */
void FactoryClassExt::_Resume_Queue()
{
    if (QueuedObjects.Count()) {
        if (Object == nullptr && (!Fetch_Rate() || IsSuspended)) {
            const TechnoTypeClass* object = QueuedObjects[0];
            QueuedObjects.Delete(0);
            int id = object->Fetch_Heap_ID();
            if (id >= 0) {
                Extension::Fetch(House)->Begin_Production(object->RTTI, id, true, TechnoTypeClassExtension::Get_Production_Flags(object->RTTI, id));
            }
        }
    }
}


/**
 *  Reimplementation of FactoryClass::Abandon.
 *
 *  @author: ZivDero
 */
bool FactoryClassExt::_Abandon()
{
    if (Object) {

        DEBUG_INFO("Abandoning production of %s\n", Object->Class_Of()->FullName);

        /*
        **  Refund all money expended so far, back to the owner of the object under construction.
        */
        int money = Object->Class_Of()->Cost_Of(Object->House);
        House->Refund_Money(money - Balance);
        Balance = 0;

        if (SpecialItem) {
            SpecialItem = SUPER_NONE;
        }

        /*
        **  Set the factory back to the idle and empty state.
        */
        Set_Rate(0);
        Set_Stage(0);
        IsSuspended = true;
        IsDifferent = true;

        if (!House->Is_Human_Player()) {
            if (Object->RTTI == RTTI_INFANTRY) {
                House->BuildInfantry = INFANTRY_NONE;
            }
            if (Object->RTTI == RTTI_UNIT) {
                if (Extension::Fetch(Object->TClass)->IsNaval) {
                    Extension::Fetch(House)->BuildNavalUnit = UNIT_NONE;
                } else {
                    House->BuildUnit = UNIT_NONE;
                }
            }
            if (Object->RTTI == RTTI_AIRCRAFT) {
                House->BuildAircraft = AIRCRAFT_NONE;
            }
            if (Object->RTTI == RTTI_BUILDING) {
                House->BuildStructure = STRUCT_NONE;
            }
        }

        /*
        **  Delete the object under construction.
        */
        ScenarioInit++;
        delete Object;
        Object = nullptr;
        ScenarioInit--;

        return true;
    }
    return false;
}


/**
 *  Main function for patching the hooks.
 */
void FactoryClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    FactoryClassExtension_Init();

    Patch_Jump(0x00496EA0, &FactoryClassExt::_AI);
    Patch_Jump(0x004971E0, &FactoryClassExt::_Start);
    Patch_Jump(0x004978D0, &FactoryClassExt::_Resume_Queue);
    Patch_Jump(0x00497330, &FactoryClassExt::_Abandon);
}
