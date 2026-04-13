/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CAMEO_BUTTON.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Button class for sidebar cameo slots.
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

#include "cameo_button.h"

#include "sidebar_model.h"
#include "sidebar_strip_view.h"

#include "building.h"
#include "eventext.h"
#include "extension.h"
#include "factory.h"
#include "factoryext.h"
#include "house.h"
#include "houseext.h"
#include "miscutil.h"
#include "mouse.h"
#include "super.h"
#include "supertype.h"
#include "techno.h"
#include "technotype.h"
#include "technotypeext.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "voc.h"
#include "vox.h"

#include <algorithm>


CameoButtonClass::CameoButtonClass() :
    ControlClass(0, 0, 0, SidebarStripView::OBJECT_WIDTH - 1, SidebarStripView::OBJECT_HEIGHT,
                 LEFTPRESS | RIGHTPRESS | LEFTUP),
    Strip(nullptr),
    Index(0),
    MousedOver(false)
{
}


CameoButtonClass::CameoButtonClass(const NoInitClass& x) :
    ControlClass(x),
    Strip(nullptr),
    Index(0),
    MousedOver(false)
{
}


/**
 *  Handles button input for cameo clicks: left-click to produce/place,
 *  right-click to suspend/cancel.
 *
 *  @author: ZivDero
 */
bool CameoButtonClass::Action(unsigned flags, KeyNumType& key)
{
    if (!Strip) {
        return true;
    }

    BuildCategory* category = Strip->Get_Category();
    if (!category) {
        return true;
    }

    int item_index = Strip->TopIndex + Index;

    if (item_index >= category->Items.Count()) {
        return true;
    }

    BuildItem& item = category->Items[item_index];
    RTTIType otype = item.Type;
    int oid = item.ID;
    FactoryClass* factory = item.Factory;

    Map.Override_Mouse_Shape(MOUSE_NORMAL);

    TechnoTypeClass const* choice = nullptr;
    SuperWeaponType spc = SUPER_NONE;

    if (otype != RTTI_SPECIAL) {
        choice = Fetch_Techno_Type(otype, oid);
    } else {
        spc = (SuperWeaponType)oid;
    }

    if (spc != SUPER_NONE) {

        if (flags & LEFTUP) {
            flags &= ~LEFTUP;
        }

        if (flags & RIGHTPRESS) {
            Map.TargettingType = SUPER_NONE;
        }

        if (flags & LEFTPRESS) {
            if (spc < PlayerPtr->SuperWeapon.Count()) {
                if (PlayerPtr->SuperWeapon[spc]->Can_Place()) {
                    if (PlayerPtr->SuperWeapon[spc]->Class->Action != ACTION_NONE) {
                        Map.TargettingType = spc;
                        Unselect_All();
                        Speak(VOX_SELECT_TARGET);
                    } else {
                        OutList.Add(EventClass(PlayerPtr->Fetch_Heap_ID(), EVENT_SPECIAL_PLACE, PlayerPtr->SuperWeapon[spc]->Class->HeapID, CELL_NONE));
                    }
                } else {
                    PlayerPtr->SuperWeapon[spc]->Impatient_Click();
                }
            }
        }

    } else {
        if (choice != nullptr) {

            if (flags & LEFTUP) {
                flags &= ~LEFTUP;
            }

            if (flags & RIGHTPRESS) {

                if (factory != nullptr) {
                    if (Map.PendingObjectPtr && Map.PendingObjectPtr->Is_Techno()) {
                        Map.PendingObjectPtr = nullptr;
                        Map.PendingObject = nullptr;
                        Map.PendingHouse = HOUSE_NONE;
                        Map.Set_Cursor_Shape(nullptr);
                    }

                    if (!factory->Is_Building()) {
                        Speak(VOX_CANCELED);

                        int count_to_abandon = 1;

                        if (Key_Down(VK_SHIFT)) {
                            count_to_abandon = factory->Total_Queued(*choice);
                        } else if (Key_Down(VK_CONTROL)) {
                            count_to_abandon = std::clamp(5, 0, factory->Total_Queued(*choice));
                        }

                        for (int i = 0; i < count_to_abandon; i++) {
                            OutList.Add(EventClassExt(PlayerPtr->Fetch_Heap_ID(), EVENT_ABANDON, otype, oid, TechnoTypeClassExtension::Get_Production_Flags(otype, oid)).As_Event());
                        }
                    } else {
                        Speak(VOX_SUSPENDED);
                        OutList.Add(EventClassExt(PlayerPtr->Fetch_Heap_ID(), EVENT_SUSPEND, otype, oid, TechnoTypeClassExtension::Get_Production_Flags(otype, oid)).As_Event());
                        Strip->Flag_To_Redraw();
                    }
                } else {
                    factory = Extension::Fetch(PlayerPtr)->Fetch_Factory(otype, TechnoTypeClassExtension::Get_Production_Flags(choice));
                    if (factory && factory->Is_Queued(*choice)) {
                        int count_to_abandon = 1;

                        if (Key_Down(VK_SHIFT)) {
                            count_to_abandon = factory->Total_Queued(*choice);
                        } else if (Key_Down(VK_CONTROL)) {
                            count_to_abandon = std::clamp(5, 0, factory->Total_Queued(*choice));
                        }

                        for (int i = 0; i < count_to_abandon; i++) {
                            OutList.Add(EventClassExt(PlayerPtr->Fetch_Heap_ID(), EVENT_ABANDON, otype, oid, TechnoTypeClassExtension::Get_Production_Flags(otype, oid)).As_Event());
                        }
                    }
                }
            }

            if (flags & LEFTPRESS) {
                if (factory != nullptr && !factory->Is_Building() && !Extension::Fetch(factory)->IsHoldingExit) {

                    if (factory->Has_Completed()) {

                        TechnoClass* pending = factory->Get_Object();
                        if (!pending) {
                            if (factory->Get_Special_Item() != -1) {
                                Map.TargettingType = SUPER_ANY;
                            }
                        } else {
                            BuildingClass* builder = pending->Who_Can_Build_Me(false, false);
                            if (!builder) {
                                OutList.Add(EventClassExt(PlayerPtr->Fetch_Heap_ID(), EVENT_ABANDON, otype, oid, TechnoTypeClassExtension::Get_Production_Flags(otype, oid)).As_Event());
                                Speak(VOX_NO_FACTORY);
                            } else {

                                if (pending->Fetch_RTTI() == RTTI_BUILDING) {
                                    PlayerPtr->Manual_Place(builder, (BuildingClass*)pending);
                                } else {
                                    OutList.Add(EventClassExt(pending->Owner(), EVENT_PLACE, otype, CELL_NONE, TechnoTypeClassExtension::Get_Production_Flags(pending)).As_Event());
                                }
                            }
                        }
                    } else {

                        if (otype == RTTI_INFANTRYTYPE) {
                            Speak(VOX_TRAINING);
                        } else {
                            Speak(VOX_BUILDING);
                        }
                        OutList.Add(EventClassExt(PlayerPtr->Fetch_Heap_ID(), EVENT_PRODUCE, otype, oid, TechnoTypeClassExtension::Get_Production_Flags(otype, oid)).As_Event());
                    }

                } else {

                    factory = Extension::Fetch(PlayerPtr)->Fetch_Factory(otype, TechnoTypeClassExtension::Get_Production_Flags(choice));
                    bool produce = false;
                    if (factory != nullptr && (factory->Is_Building() || factory->Has_Production_Target())) {
                        if (otype == RTTI_BUILDINGTYPE) {
                            Speak(VOX_NO_FACTORY);
                        } else {
                            produce = true;
                        }
                    } else {

                        if (otype == RTTI_INFANTRYTYPE) {
                            Speak(VOX_TRAINING);
                        } else {
                            Speak(VOX_BUILDING);
                        }
                        produce = true;
                    }
                    if (produce) {
                        const int count_to_produce = Key_Down(VK_SHIFT) ? 5 : 1;
                        for (int i = 0; i < count_to_produce; i++) {
                            OutList.Add(EventClassExt(PlayerPtr->Fetch_Heap_ID(), EVENT_PRODUCE, otype, oid, TechnoTypeClassExtension::Get_Production_Flags(otype, oid)).As_Event());
                        }
                    }
                }
            }
        } else {
            flags = 0;
        }
    }

    ControlClass::Action(flags, key);
    return true;
}


void CameoButtonClass::On_Mouse_Enter()
{
    MousedOver = true;
}


void CameoButtonClass::On_Mouse_Leave()
{
    MousedOver = false;
}


void CameoButtonClass::Set_Owner(SidebarStripView& strip, int index)
{
    Strip = &strip;
    Index = index;
}
