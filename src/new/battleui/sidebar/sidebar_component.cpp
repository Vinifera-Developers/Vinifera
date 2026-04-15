/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Sidebar component implementation.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "sidebar_component.h"

#include "abstract.h"
#include "building.h"
#include "convert.h"
#include "eventext.h"
#include "extension.h"
#include "factory.h"
#include "factoryext.h"
#include "house.h"
#include "houseext.h"
#include "miscutil.h"
#include "mouse.h"
#include "object.h"
#include "palette.h"
#include "sidebar.h"
#include "sidebar_classic_view.h"
#include "sidebar_strip_view.h"
#include "sidebar_tabbed_view.h"
#include "isidebar_view.h"
#include "super.h"
#include "supertype.h"
#include "techno.h"
#include "technotype.h"
#include "technotypeext.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "uicontrol.h"
#include "voc.h"
#include "vox.h"

#include <algorithm>

namespace
{
/***************************************************************************
**  Cameo action resolution helpers
***************************************************************************/


struct ResolvedCameoAction
{
    RTTIType Type;
    int ID;
    ProductionFlags Flags;
    FactoryClass* LinkedFactory;
    const TechnoTypeClass* Choice;
    SuperWeaponType SuperType;
};


/**
 *  Resolves action context for a clicked sidebar cameo.
 *
 *  @author: ZivDero
 */
bool Resolve_Cameo_Action(BuildItem& item, ResolvedCameoAction& resolved)
{
    resolved.Type = item.Type;
    resolved.ID = item.ID;
    resolved.Flags = TechnoTypeClassExtension::Get_Production_Flags(item.Type, item.ID);
    resolved.LinkedFactory = item.Factory;
    resolved.Choice = nullptr;
    resolved.SuperType = SUPER_NONE;

    if (item.Type == RTTI_SPECIAL) {
        resolved.SuperType = static_cast<SuperWeaponType>(item.ID);
        return true;
    }

    resolved.Choice = Fetch_Techno_Type(item.Type, item.ID);
    return resolved.Choice != nullptr;
}


/**
 *  Returns the player's factory for the given production type and flags.
 *
 *  @author: ZivDero
 */
FactoryClass* Fetch_Player_Factory(RTTIType type, ProductionFlags flags)
{
    return Extension::Fetch(PlayerPtr)->Fetch_Factory(type, flags);
}


/**
 *  Returns how many queued objects should be abandoned for the active modifiers.
 *
 *  @author: ZivDero
 */
int Count_Queued_Abandon(FactoryClass& factory, const TechnoTypeClass& choice)
{
    if (Key_Down(VK_SHIFT)) {
        return factory.Total_Queued(choice);
    }

    if (Key_Down(VK_CONTROL)) {
        return std::min(5, factory.Total_Queued(choice));
    }

    return 1;
}


/**
 *  Clears pending placement state when it references a techno object.
 *
 *  @author: ZivDero
 */
void Clear_Pending_Techno_Placement()
{
    if (Map.PendingObjectPtr && Map.PendingObjectPtr->Is_Techno()) {
        Map.PendingObjectPtr = nullptr;
        Map.PendingObject = nullptr;
        Map.PendingHouse = HOUSE_NONE;
        Map.Set_Cursor_Shape(nullptr);
    }
}


/**
 *  Plays the appropriate produce voice for the requested item type.
 *
 *  @author: ZivDero
 */
void Speak_Produce_Voice(RTTIType type)
{
    Speak(type == RTTI_INFANTRYTYPE ? VOX_TRAINING : VOX_BUILDING);
}


/**
 *  Queues one or more produce events for the specified item.
 *
 *  @author: ZivDero
 */
void Queue_Produce_Event(RTTIType type, int id, ProductionFlags flags, int count)
{
    for (int i = 0; i < count; ++i) {
        OutList.Add(EventClassExt(PlayerPtr->Fetch_Heap_ID(), EVENT_PRODUCE, type, id, flags).As_Event());
    }
}


/**
 *  Queues one or more abandon events for the specified item.
 *
 *  @author: ZivDero
 */
void Queue_Abandon_Event(RTTIType type, int id, ProductionFlags flags, int count)
{
    for (int i = 0; i < count; ++i) {
        OutList.Add(EventClassExt(PlayerPtr->Fetch_Heap_ID(), EVENT_ABANDON, type, id, flags).As_Event());
    }
}


/**
 *  Queues a suspend event for the specified item.
 *
 *  @author: ZivDero
 */
void Queue_Suspend_Event(RTTIType type, int id, ProductionFlags flags)
{
    OutList.Add(EventClassExt(PlayerPtr->Fetch_Heap_ID(), EVENT_SUSPEND, type, id, flags).As_Event());
}


/**
 *  Queues a placement event for a completed production output.
 *
 *  @author: ZivDero
 */
void Queue_Place_Event(int owner, RTTIType type, ProductionFlags flags)
{
    OutList.Add(EventClassExt(owner, EVENT_PLACE, type, CELL_NONE, flags).As_Event());
}


/***************************************************************************
**  Cameo action execution
***************************************************************************/


/**
 *  Handles input for a superweapon cameo.
 *
 *  @author: ZivDero
 */
void Handle_Superweapon_Action(const ResolvedCameoAction& action, unsigned flags)
{
    if (action.SuperType == SUPER_NONE) {
        return;
    }

    if (flags & GadgetClass::RIGHTPRESS) {
        Map.TargettingType = SUPER_NONE;
    }

    if (!(flags & GadgetClass::LEFTPRESS) || action.SuperType >= PlayerPtr->SuperWeapon.Count()) {
        return;
    }

    SuperClass* super = PlayerPtr->SuperWeapon[action.SuperType];
    if (super == nullptr) {
        return;
    }

    if (!super->Can_Place()) {
        super->Impatient_Click();
        return;
    }

    if (super->Class->Action != ACTION_NONE) {
        Map.TargettingType = action.SuperType;
        Unselect_All();
        Speak(VOX_SELECT_TARGET);
        return;
    }

    OutList.Add(EventClass(PlayerPtr->Fetch_Heap_ID(), EVENT_SPECIAL_PLACE, super->Class->HeapID, CELL_NONE));
}


/**
 *  Handles right-click interaction on a techno cameo.
 *
 *  @author: ZivDero
 */
void Handle_Techno_Right_Press(SidebarComponent& sidebar, const ResolvedCameoAction& action)
{
    FactoryClass* factory = action.LinkedFactory;

    if (factory != nullptr) {
        Clear_Pending_Techno_Placement();

        if (!factory->Is_Building()) {
            Speak(VOX_CANCELED);
            Queue_Abandon_Event(action.Type, action.ID, action.Flags, Count_Queued_Abandon(*factory, *action.Choice));
        } else {
            Speak(VOX_SUSPENDED);
            Queue_Suspend_Event(action.Type, action.ID, action.Flags);
        }
        return;
    }

    factory = Fetch_Player_Factory(action.Type, action.Flags);
    if (factory != nullptr && factory->Is_Queued(*action.Choice)) {
        Queue_Abandon_Event(action.Type, action.ID, action.Flags, Count_Queued_Abandon(*factory, *action.Choice));
    }
}


/**
 *  Handles placement or release of a completed factory output.
 *
 *  @author: ZivDero
 */
void Handle_Completed_Factory_Output(const ResolvedCameoAction& action, FactoryClass& factory)
{
    TechnoClass* pending = factory.Get_Object();
    if (pending == nullptr) {
        if (factory.Get_Special_Item() != -1) {
            Map.TargettingType = SUPER_ANY;
        }
        return;
    }

    BuildingClass* builder = pending->Who_Can_Build_Me(false, false);
    if (builder == nullptr) {
        Queue_Abandon_Event(action.Type, action.ID, action.Flags, 1);
        Speak(VOX_NO_FACTORY);
        return;
    }

    if (pending->Fetch_RTTI() == RTTI_BUILDING) {
        PlayerPtr->Manual_Place(builder, static_cast<BuildingClass*>(pending));
        return;
    }

    Queue_Place_Event(pending->Owner(), action.Type, TechnoTypeClassExtension::Get_Production_Flags(pending));
}


/**
 *  Handles left-click interaction on a techno cameo.
 *
 *  @author: ZivDero
 */
void Handle_Techno_Left_Press(const ResolvedCameoAction& action)
{
    FactoryClass* factory = action.LinkedFactory;
    if (factory != nullptr && !factory->Is_Building() && !Extension::Fetch(factory)->IsHoldingExit) {
        if (factory->Has_Completed()) {
            Handle_Completed_Factory_Output(action, *factory);
        } else {
            Speak_Produce_Voice(action.Type);
            Queue_Produce_Event(action.Type, action.ID, action.Flags, 1);
        }
        return;
    }

    factory = Fetch_Player_Factory(action.Type, action.Flags);

    bool produce = false;
    if (factory != nullptr && (factory->Is_Building() || factory->Has_Production_Target())) {
        if (action.Type == RTTI_BUILDINGTYPE) {
            Speak(VOX_NO_FACTORY);
        } else {
            produce = true;
        }
    } else {
        Speak_Produce_Voice(action.Type);
        produce = true;
    }

    if (produce) {
        Queue_Produce_Event(action.Type, action.ID, action.Flags, Key_Down(VK_SHIFT) ? 5 : 1);
    }
}
}


/***************************************************************************
**  Lifecycle and setup
***************************************************************************/


/**
 *  Class constructor.
 *
 *  @author: ZivDero
 */
SidebarComponent::SidebarComponent() :
    ActiveView(nullptr),
    ActiveViewType(SIDEBAR_CLASSIC)
{
}


/**
 *  Class destructor.
 *
 *  @author: ZivDero
 */
SidebarComponent::~SidebarComponent()
{
    Shutdown();
}


/**
 *  One-time initialization. Called once at game startup.
 *
 *  @author: ZivDero
 */
void SidebarComponent::One_Time()
{
    SidebarViewType view_type = UIControls->BattleSidebarViewType;
    ActiveViewType = view_type;

    /**
     *  Create the view based on configuration.
     */
    if (ActiveView == nullptr) {
        switch (view_type) {
        case SIDEBAR_TABBED:
            Model.Init_Categories(4);
            ActiveView = new TabbedSidebarView(&Model);
            break;

        case SIDEBAR_CLASSIC:
        default:
            Model.Init_Categories(2);
            ActiveView = new ClassicSidebarView(&Model);
            break;
        }
    }

    ActionBar.Set_Sidebar_View_Type(ActiveViewType);

    if (ActiveView) {
        ActiveView->One_Time();
    }
}


/**
 *  Clears all state for a new scenario.
 *
 *  @author: ZivDero
 */
void SidebarComponent::Init_Clear()
{
    ActionBar.Init_Clear();
    Model.Init_Clear();

    if (ActiveView) {
        ActiveView->Init_Clear();
    }
}


/**
 *  Initializes IO gadgets (buttons, select controls).
 *
 *  @author: ZivDero
 */
void SidebarComponent::Init_IO()
{
    ActionBar.Init_IO();

    if (ActiveView) {
        ActiveView->Init_IO();
    }
}


/**
 *  Initializes for the current player house.
 *
 *  @author: ZivDero
 */
void SidebarComponent::Init_For_House()
{
    Model.Init_For_House();

    Prepare_Drawer();

    if (ActiveView) {
        ActiveView->Init_For_House();
    }

    ActionBar.Init_For_House();
}


/***************************************************************************
**  Runtime behavior
***************************************************************************/


/**
 *  Per-frame update. Updates models and forwards to active view.
 *
 *  @author: ZivDero
 */
void SidebarComponent::AI(KeyNumType& key, Point2D& mouse)
{
    if (Model.Needs_Recalc()) {
        Recalc();
    }

    Production_AI();
    ActionBar.AI(key);

    if (ActiveView) {
        ActiveView->AI(key, mouse);
    }
}


/**
 *  Renders the sidebar. Forwards to the active view.
 *
 *  @author: ZivDero
 */
void SidebarComponent::Draw()
{
    if (ActiveView) {
        ActiveView->Draw();
    }

    ActionBar.Draw();
}


/**
 *  Blits the sidebar surface to the screen.
 *
 *  @author: ZivDero
 */
void SidebarComponent::Blit(bool complete)
{
    if (ActiveView) {
        ActiveView->Blit(complete);
    }
}


/**
 *  Shuts down the sidebar and cleans up resources.
 *
 *  @author: ZivDero
 */
void SidebarComponent::Shutdown()
{
    if (ActiveView != nullptr) {
        Activate(0);
        delete ActiveView;
        ActiveView = nullptr;
    }
}


/***************************************************************************
**  Sidebar interaction API
***************************************************************************/


/**
 *  Adds a buildable item to the sidebar.
 *
 *  @author: ZivDero
 */
bool SidebarComponent::Add(RTTIType type, int id)
{
    return Model.Add(type, id);
}


/**
 *  Scrolls a sidebar column.
 *
 *  @author: ZivDero
 */
bool SidebarComponent::Scroll(bool up, int column)
{
    if (ActiveView) {
        return ActiveView->Scroll(up, column);
    }
    return false;
}


/**
 *  Scrolls a sidebar column by a full page.
 *
 *  @author: ZivDero
 */
bool SidebarComponent::Scroll_Page(bool up, int column)
{
    if (ActiveView) {
        return ActiveView->Scroll_Page(up, column);
    }
    return false;
}


/**
 *  Re-sorts all sidebar categories.
 *
 *  @author: ZivDero
 */
void SidebarComponent::Recalc()
{
    if (ActiveView) {
        ActiveView->Prepare_Model_Recalc();
    }

    Model.Recalc_All();

    if (ActiveView) {
        ActiveView->Finish_Model_Recalc();
    }
}


/**
 *  Links a factory to a buildable item.
 *
 *  @author: ZivDero
 */
bool SidebarComponent::Factory_Link(FactoryClass* factory, RTTIType type, int id)
{
    if (!Model.Is_On_Sidebar(type, id)) {
        return false;
    }

    Model.Link_Factory(factory, type, id);
    return true;
}


/**
 *  Abandons production of a buildable item.
 *
 *  @author: ZivDero
 */
bool SidebarComponent::Abandon_Production(RTTIType type, int id)
{
    if (!Model.Is_On_Sidebar(type, id)) {
        return false;
    }

    Model.Abandon_Production(type, id);
    return true;
}


/**
 *  Routes a cameo button click through the shared sidebar interaction rules.
 *
 *  @author: ZivDero
 */
bool SidebarComponent::Handle_Cameo_Action(SidebarStripView& strip, int slot, unsigned& flags)
{
    BuildItem* item = strip.Get_Visible_Item(slot);
    if (item == nullptr) {
        flags = 0;
        return false;
    }

    ResolvedCameoAction action{};
    if (!Resolve_Cameo_Action(*item, action)) {
        flags = 0;
        return false;
    }

    Map.Override_Mouse_Shape(MOUSE_NORMAL);

    if (flags & GadgetClass::LEFTUP) {
        flags &= ~GadgetClass::LEFTUP;
    }

    if (action.SuperType != SUPER_NONE) {
        Handle_Superweapon_Action(action, flags);
        return true;
    }

    if (action.Choice == nullptr) {
        flags = 0;
        return false;
    }

    if (flags & GadgetClass::RIGHTPRESS) {
        Handle_Techno_Right_Press(*this, action);
    }

    if (flags & GadgetClass::LEFTPRESS) {
        Handle_Techno_Left_Press(action);
    }

    return true;
}


/**
 *  Checks if an item is on the sidebar.
 *
 *  @author: ZivDero
 */
bool SidebarComponent::Is_On_Sidebar(RTTIType type, int id) const
{
    return Model.Is_On_Sidebar(type, id);
}


/**
 *  Changes the active tab in the tabbed view, if active.
 *
 *  @author: ZivDero
 */
bool SidebarComponent::Change_Tab(int index)
{
    if (ActiveView != nullptr) {
        return ActiveView->Change_Tab(index);
    }
    return false;
}


/**
 *  Nullifies factory pointers for any BuildItem whose Factory matches target.
 *
 *  @author: ZivDero
 */
void SidebarComponent::Detach(AbstractClass* target)
{
    Model.Detach(target);
}


/**
 *  Activates or deactivates the sidebar.
 *
 *  @author: ZivDero
 */
void SidebarComponent::Activate(int control)
{
    ActionBar.Activate(control);

    if (ActiveView) {
        ActiveView->Activate(control);
    }
}


/**
 *  Reflows the sidebar layout.
 *
 *  @author: ZivDero
 */
void SidebarComponent::Shift_Sidebar()
{
    ActionBar.Shift_Sidebar();

    if (ActiveView) {
        ActiveView->Shift_Sidebar();
    }
}


/**
 *  Returns help text for a gadget id, or nullptr.
 *
 *  @author: ZivDero
 */
const char *SidebarComponent::Help_Text(int gadget_id)
{
    if (ActiveView != nullptr) {
        return ActiveView->Help_Text(gadget_id);
    }
    return nullptr;
}


/**
 *  Returns the total number of visible sidebar buttons.
 *
 *  @author: ZivDero
 */
int SidebarComponent::Visible_Button_Count() const
{
    if (ActiveView) {
        return ActiveView->Visible_Button_Count();
    }
    return SidebarClass::StripClass::MAX_VISIBLE * 2;
}


/**
 *  Returns the number of visible buttons in one column.
 *
 *  @author: ZivDero
 */
int SidebarComponent::Visible_Buttons_Per_Column() const
{
    if (ActiveView) {
        return ActiveView->Visible_Buttons_Per_Column();
    }
    return SidebarClass::StripClass::MAX_VISIBLE;
}


/***************************************************************************
**  Persistence and internal helpers
***************************************************************************/


/**
 *  Saves the sidebar model to the given stream.
 *
 *  @author: ZivDero
 */
HRESULT SidebarComponent::Save(IStream* pStm) const
{
    return Model.Save(pStm);
}


/**
 *  Loads the sidebar model from the given stream.
 *
 *  @author: ZivDero
 */
HRESULT SidebarComponent::Load(IStream* pStm)
{
    return Model.Load(pStm);
}


/**
 *  Relinks factory pointers to sidebar items after a game load. Iterates the
 *  global Factories vector and reconnects each player-owned factory to the
 *  matching sidebar item, then triggers a full recalc.
 *
 *  @author: ZivDero
 */
void SidebarComponent::Relink_Factories()
{
    for (int i = 0; i < Factories.Count(); i++) {
        FactoryClass* factory = Factories[i];
        if (factory == nullptr || factory->Get_House() != PlayerPtr) {
            continue;
        }

        TechnoClass* object = factory->Get_Object();
        if (object == nullptr) {
            continue;
        }

        RTTIType rtti = object->Class_Of()->Fetch_RTTI();
        int id = object->Class_Of()->Fetch_Heap_ID();

        if (Model.Is_On_Sidebar(rtti, id)) {
            Model.Link_Factory(factory, rtti, id);
        }
    }

    Recalc();
}


/**
 *  Recreates the sidebar palette conversion drawer used by sidebar art.
 *
 *  @author: ZivDero
 */
void SidebarComponent::Prepare_Drawer()
{
    if (Debug_Map) {
        return;
    }

    PaletteClass pal("SIDEBAR.PAL");

    delete SidebarDrawer;
    SidebarDrawer = new ConvertClass(&pal, &pal, VisibleSurface, 1);
}


/**
 *  Updates production state across all sidebar items.
 *
 *  @author: ZivDero
 */
void SidebarComponent::Production_AI()
{
    for (int category_index = 0; category_index < Model.Category_Count(); ++category_index) {
        BuildCategory& category = Model.Get_Category(category_index);

        for (int item_index = 0; item_index < category.Items.Count(); ++item_index) {
            BuildItem& item = category.Items[item_index];
            FactoryClass* factory = item.Factory;
            if (factory && (factory->Has_Changed() || Extension::Fetch(factory)->IsHoldingExit)) {
                if (factory->Has_Completed()) {

                    /**
                     *  Construction has been completed. Announce this fact to the player and
                     *  try to get the object to automatically leave the factory. Buildings are
                     *  the main exception to the ability to leave the factory under their own
                     *  power.
                     */
                    TechnoClass* pending = factory->Get_Object();
                    if (pending != nullptr) {
                        switch (pending->RTTI) {
                        case RTTI_UNIT:
                        case RTTI_INFANTRY:
                        case RTTI_AIRCRAFT:
                            OutList.Add(EventClassExt(pending->Owner(), EVENT_PLACE, pending->RTTI, CELL_NONE, TechnoTypeClassExtension::Get_Production_Flags(pending)).As_Event());
                            break;

                        case RTTI_BUILDING:
                            Speak(VOX_CONSTRUCTION);
                            if (ActiveView) ActiveView->Notify_Production_Complete(category_index);
                            break;

                        default:
                            break;
                        }
                    }
                }
            }
        }
    }
}
