/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SIDEBAR_COMPONENT.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Sidebar component implementation.
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

#include "sidebar_component.h"

#include "abstract.h"
#include "sidebar_classic_view.h"
#include "sidebar_config.h"
#include "sidebar_tabbed_view.h"
#include "sidebar_view.h"

#include "sidebar.h"


SidebarComponent Sidebar;


/**
 *  Class constructor.
 *
 *  @author: ZivDero
 */
SidebarComponent::SidebarComponent() :
    ActiveView(nullptr)
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
    BattleUI.Register(this);

    /**
     *  Create the view based on configuration.
     */
    if (ActiveView == nullptr) {
        switch (UIConfig.Sidebar.ViewType) {
        case SIDEBAR_TABBED:
            Model.Categories.Resize(4);
            for (int i = 0; i < 4; i++) {
                Model.Categories.Add(BuildCategory());
            }
            ActiveView = new TabbedSidebarView(&Model);
            break;

        case SIDEBAR_CLASSIC:
        default:
            Model.Categories.Resize(2);
            for (int i = 0; i < 2; i++) {
                Model.Categories.Add(BuildCategory());
            }
            ActiveView = new ClassicSidebarView(&Model);
            break;
        }
    }

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

    if (ActiveView) {
        ActiveView->Init_For_House();
    }
}


/**
 *  Per-frame update. Updates models and forwards to active view.
 *
 *  @author: ZivDero
 */
void SidebarComponent::AI(KeyNumType& key, Point2D& mouse)
{
    if (Model.IsDirty) {
        Model.Recalc_All();
    }

    if (ActiveView) {
        ActiveView->AI(key, mouse);
    }
}


/**
 *  Renders the sidebar. Forwards to the active view.
 *
 *  @author: ZivDero
 */
void SidebarComponent::Draw(bool complete)
{
    if (ActiveView) {
        ActiveView->Draw(complete);
    }
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
        delete ActiveView;
        ActiveView = nullptr;
    }
}


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
    Model.Recalc_All();
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
 *  Checks if an item is on the sidebar.
 *
 *  @author: ZivDero
 */
bool SidebarComponent::Is_On_Sidebar(RTTIType type, int id) const
{
    return Model.Is_On_Sidebar(type, id);
}


/**
 *  Flags the currently active strip for redraw.
 *
 *  @author: ZivDero
 */
void SidebarComponent::Flag_Current_Strip_To_Redraw()
{
    if (ActiveView != nullptr) {
        ActiveView->Flag_Current_Strip_To_Redraw();
    }
}


/**
 *  Flags the strip that owns the specified production type for redraw.
 *
 *  @author: ZivDero
 */
void SidebarComponent::Flag_Strip_To_Redraw(RTTIType type, ProductionFlags flags)
{
    if (ActiveView != nullptr) {
        ActiveView->Flag_Strip_To_Redraw(type, flags);
    }
}


/**
 *  Changes the active tab in the tabbed view, if active.
 *
 *  @author: ZivDero
 */
bool SidebarComponent::Change_Tab(int index)
{
    TabbedSidebarView* tabbed = dynamic_cast<TabbedSidebarView*>(ActiveView);
    if (tabbed != nullptr) {
        return tabbed->Change_Tab(static_cast<TabbedSidebarView::SidebarTabType>(index));
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
    if (ActiveView) {
        ActiveView->Activate(control);
    }
}


/**
 *  Recalculates sidebar dimensions.
 *
 *  @author: ZivDero
 */
void SidebarComponent::Set_Dimensions()
{
    if (ActiveView) {
        ActiveView->Set_Dimensions();
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
 *  Returns the number of visible rows.
 *
 *  @author: ZivDero
 */
int SidebarComponent::Max_Visible() const
{
    if (ActiveView) {
        return ActiveView->Max_Visible();
    }
    return SidebarClass::StripClass::MAX_VISIBLE;
}


/**
 *  Returns the number of visible items for one strip or for both columns.
 *
 *  @author: ZivDero
 */
int SidebarComponent::Max_Visible(bool one_strip) const
{
    int rows = Max_Visible();
    return one_strip ? rows : rows * 2;
}


/**
 *  Initializes the sidebar strips. Called during scenario init.
 *
 *  @author: ZivDero
 */
void SidebarComponent::Init_Strips()
{
    if (ActiveView) {
        ActiveView->Init_IO();
    }
}
