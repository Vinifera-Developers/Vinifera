/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SIDEBAREXT.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Extended SidebarClass class.
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
#include "sidebarext.h"
#include "sidebarext.h"
#include "tibsun_globals.h"
#include "tibsun_defines.h"
#include "ccini.h"
#include "noinit.h"
#include "swizzle.h"
#include "scenarioext.h"
#include "vinifera_saveload.h"
#include "asserthandler.h"
#include "debughandler.h"
#include "drawshape.h"
#include "language.h"
#include "tooltip.h"
#include "mouse.h"
#include "wwmouse.h"


GadgetClass* SidebarClassExtension::LastHovered;

/**
 *  Class constructor.
 *  
 *  @author: ZivDero
 */
SidebarClassExtension::SidebarClassExtension(const SidebarClass *this_ptr) :
    GlobalExtensionClass(this_ptr)
{
    //if (this_ptr) EXT_DEBUG_TRACE("SidebarClassExtension::SidebarClassExtension - 0x%08X\n", (uintptr_t)(ThisPtr));

    int max_visible = Max_Visible(true);

    for (int i = 0; i < SIDEBAR_TAB_COUNT; i++)
    {
        new (&Column[i]) SidebarClass::StripClass(NoInitClass());
        Column[i].X = SidebarClass::COLUMN_ONE_X;
        Column[i].Y = COLUMN_Y;
        Column[i].Size = Rect(SidebarClass::COLUMN_ONE_X, SidebarClass::COLUMN_ONE_Y, SidebarClass::StripClass::OBJECT_WIDTH, SidebarClass::StripClass::OBJECT_HEIGHT * max_visible);
    }
}


/**
 *  Class no-init constructor.
 *  
 *  @author: ZivDero
 */
SidebarClassExtension::SidebarClassExtension(const NoInitClass &noinit) :
    GlobalExtensionClass(noinit)
{
    //EXT_DEBUG_TRACE("SidebarClassExtension::SidebarClassExtension(NoInitClass) - 0x%08X\n", (uintptr_t)(ThisPtr));
}


/**
 *  Class destructor.
 *  
 *  @author: ZivDero
 */
SidebarClassExtension::~SidebarClassExtension()
{
    //EXT_DEBUG_TRACE("SidebarClassExtension::~SidebarClassExtension - 0x%08X\n", (uintptr_t)(ThisPtr));
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: ZivDero
 */
HRESULT SidebarClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("SidebarClassExtension::Load - 0x%08X\n", (uintptr_t)(This()));

    HRESULT hr = GlobalExtensionClass::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) SidebarClassExtension(NoInitClass());

    /**
     *  We need to swizzle the factory pointers to restore their link to buildables.
     */
    for (int i = 0; i < SIDEBAR_TAB_COUNT; i++)
    {
        for (int j = 0 ; j < SidebarClass::StripClass::MAX_BUILDABLES; j++)
        {
            VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(Column[i].Buildables[j].Factory, "Factory");
        }
    }

    /**
     *  Reinitialize all the UI elements.
     *  If we've loaded a savegame, they will be erased by the constructor.
     */

    Init_IO();
    Set_Dimensions();
    Init_For_House();
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: ZivDero
 */
HRESULT SidebarClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("SidebarClassExtension::Save - 0x%08X\n", (uintptr_t)(This()));

    HRESULT hr = GlobalExtensionClass::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: ZivDero
 */
int SidebarClassExtension::Size_Of() const
{
    //EXT_DEBUG_TRACE("SidebarClassExtension::Size_Of - 0x%08X\n", (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: ZivDero
 */
void SidebarClassExtension::Detach(TARGET target, bool all)
{
    //EXT_DEBUG_TRACE("SidebarClassExtension::Detach - 0x%08X\n", (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: ZivDero
 */
void SidebarClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("SidebarClassExtension::Compute_CRC - 0x%08X\n", (uintptr_t)(This()));
}


/**
 *  Initializes the new sidebar strips.
 *
 *  @author: ZivDero
 */
void SidebarClassExtension::Init_Strips()
{
    int max_visible = Max_Visible(true);

    for (int i = 0; i < SIDEBAR_TAB_COUNT; i++)
    {
        new (&Column[i]) SidebarClass::StripClass(NoInitClass());
        Column[i].X = SidebarClass::COLUMN_ONE_X;
        Column[i].Y = COLUMN_Y;
        Column[i].Size = Rect(SidebarClass::COLUMN_ONE_X, SidebarClass::COLUMN_ONE_Y, SidebarClass::StripClass::OBJECT_WIDTH, SidebarClass::StripClass::OBJECT_HEIGHT * max_visible);
    }
}


/**
 *  Initializes the new sidebar buttons.
 *
 *  @author: ZivDero
 */
void SidebarClassExtension::Init_IO()
{
    TabButtons[0].IsSticky = true;
    TabButtons[0].ID = BUTTON_TAB_1;
    TabButtons[0].Y = 148;
    TabButtons[0].DrawX = -480;
    TabButtons[0].DrawY = 3;
    TabButtons[0].IsSelected = false;
    TabButtons[0].IsDisabled = true;

    TabButtons[1].IsSticky = true;
    TabButtons[1].ID = BUTTON_TAB_2;
    TabButtons[1].Y = 148;
    TabButtons[1].DrawX = -480;
    TabButtons[1].DrawY = 3;
    TabButtons[1].IsSelected = false;
    TabButtons[1].IsDisabled = true;

    TabButtons[2].IsSticky = true;
    TabButtons[2].ID = BUTTON_TAB_3;
    TabButtons[2].Y = 148;
    TabButtons[2].DrawX = -480;
    TabButtons[2].DrawY = 3;
    TabButtons[2].IsSelected = false;
    TabButtons[2].IsDisabled = true;

    TabButtons[3].IsSticky = true;
    TabButtons[3].ID = BUTTON_TAB_4;
    TabButtons[3].Y = 148;
    TabButtons[3].DrawX = -480;
    TabButtons[3].DrawY = 3;
    TabButtons[3].IsSelected = false;
    TabButtons[3].IsDisabled = true;
}


/**
 *  Positions the new sidebar buttons and creates their tooltips.
 *
 *  @author: ZivDero
 */
void SidebarClassExtension::Set_Dimensions()
{
    TabButtons[0].Set_Position(SidebarRect.X + TAB_ONE_X_OFFSET, SidebarRect.Y + TAB_Y_OFFSET);
    TabButtons[0].Flag_To_Redraw();
    TabButtons[0].DrawX = -SidebarRect.X;

    TabButtons[1].Set_Position(SidebarRect.X + TAB_TWO_X_OFFSET, TabButtons[0].Y);
    TabButtons[1].Flag_To_Redraw();
    TabButtons[1].DrawX = -SidebarRect.X;

    TabButtons[2].Set_Position(SidebarRect.X + TAB_THREE_X_OFFSET, TabButtons[1].Y);
    TabButtons[2].Flag_To_Redraw();
    TabButtons[2].DrawX = -SidebarRect.X;

    TabButtons[3].Set_Position(SidebarRect.X + TAB_FOUR_X_OFFSET, TabButtons[2].Y);
    TabButtons[3].Flag_To_Redraw();
    TabButtons[3].DrawX = -SidebarRect.X;

    if (ToolTipHandler)
    {
        ToolTip tooltip;

        tooltip.Region = Rect(TabButtons[0].X, TabButtons[0].Y, TabButtons[0].Width, TabButtons[0].Height);
        tooltip.ID = BUTTON_TAB_1;
        tooltip.Text = TXT_NONE;
        ToolTipHandler->Remove(tooltip.ID);
        ToolTipHandler->Add(&tooltip);

        tooltip.Region = Rect(TabButtons[1].X, TabButtons[1].Y, TabButtons[1].Width, TabButtons[1].Height);
        tooltip.ID = BUTTON_TAB_2;
        tooltip.Text = TXT_NONE;
        ToolTipHandler->Remove(tooltip.ID);
        ToolTipHandler->Add(&tooltip);

        tooltip.Region = Rect(TabButtons[2].X, TabButtons[2].Y, TabButtons[2].Width, TabButtons[2].Height);
        tooltip.ID = BUTTON_TAB_3;
        tooltip.Text = TXT_NONE;
        ToolTipHandler->Remove(tooltip.ID);
        ToolTipHandler->Add(&tooltip);

        tooltip.Region = Rect(TabButtons[3].X, TabButtons[3].Y, TabButtons[3].Width, TabButtons[3].Height);
        tooltip.ID = BUTTON_TAB_4;
        tooltip.Text = TXT_NONE;
        ToolTipHandler->Remove(tooltip.ID);
        ToolTipHandler->Add(&tooltip);
    }
}


/**
 *  Loads new house-specific sidebar graphics.
 *
 *  @author: ZivDero
 */
void SidebarClassExtension::Init_For_House()
{
    TabButtons[0].Set_Shape(MFCC::RetrieveT<ShapeFileStruct>("TAB-BLD.SHP"));
    TabButtons[0].ShapeDrawer = SidebarDrawer;

    TabButtons[1].Set_Shape(MFCC::RetrieveT<ShapeFileStruct>("TAB-INF.SHP"));
    TabButtons[1].ShapeDrawer = SidebarDrawer;

    TabButtons[2].Set_Shape(MFCC::RetrieveT<ShapeFileStruct>("TAB-UNT.SHP"));
    TabButtons[2].ShapeDrawer = SidebarDrawer;

    TabButtons[3].Set_Shape(MFCC::RetrieveT<ShapeFileStruct>("TAB-SPC.SHP"));
    TabButtons[3].ShapeDrawer = SidebarDrawer;
}


/**
 *  Switches the sidebar tab.
 *
 *  @author: ZivDero
 */
bool SidebarClassExtension::Change_Tab(SidebarTabType index)
{
    // Don't switch to the same tab
    if (TabIndex == index)
        return false;

    // Do not switch to inactive tabs
    if (Column[index].BuildableCount < 1)
        return false;

    Column[TabIndex].Deactivate();
    TabButtons[TabIndex].Deselect();

    TabIndex = index;

    Column[TabIndex].Activate();
    TabButtons[TabIndex].Select();

    Map.IsToFullRedraw = true;
    return true;
}


/**
 *  Returns the first active sidebar tab, or SIDEBAR_TAB_NONE if there are none.
 *
 *  @author: ZivDero
 */
SidebarClassExtension::SidebarTabType SidebarClassExtension::First_Active_Tab()
{
    for (int i = 0; i < SIDEBAR_TAB_COUNT; i++)
    {
        if (Column[i].BuildableCount > 0)
            return (SidebarTabType)i;
    }

    return SIDEBAR_TAB_NONE;
}



/**
 *  Returns which tab a type belongs to.
 *
 *  @author: ZivDero
 */
SidebarClassExtension::SidebarTabType SidebarClassExtension::Which_Tab(RTTIType type)
{
    switch (type)
    {
    case RTTI_BUILDINGTYPE:
    case RTTI_BUILDING:
        return SIDEBAR_TAB_STRUCTURE;

    case RTTI_INFANTRYTYPE:
    case RTTI_INFANTRY:
        return SIDEBAR_TAB_INFANTRY;

    case RTTI_UNITTYPE:
    case RTTI_UNIT:
        return SIDEBAR_TAB_UNIT;

    case RTTI_AIRCRAFTTYPE:
    case RTTI_AIRCRAFT:
    case RTTI_SUPERWEAPONTYPE:
    case RTTI_SUPERWEAPON:
    case RTTI_SPECIAL:
    default:
        return SIDEBAR_TAB_SPECIAL;
    }
}


/**
 *  Class constructor.
 *
 *  @author: ZivDero
 */
SidebarClassExtension::TabButtonClass::TabButtonClass() :
ControlClass(0, 0, 0, 0, 0, LEFTPRESS | LEFTRELEASE, true),
DrawX(0),
DrawY(0),
ShapeDrawer(SidebarDrawer),
ShapeData(nullptr),
IsFlashing(false),
FlashTimer(0),
FlashFrame(0),
IsSelected(false),
IsDrawn(false)
{
}


/**
 *  Class constructor.
 *
 *  @author: ZivDero
 */
SidebarClassExtension::TabButtonClass::TabButtonClass(unsigned id, const ShapeFileStruct* shapes, int x, int y, ConvertClass* drawer, int w, int h) :
    ControlClass(id, x, y, w, h, LEFTPRESS | LEFTRELEASE, true),
DrawX(0),
DrawY(0),
ShapeDrawer(drawer),
ShapeData(shapes),
IsFlashing(false),
FlashTimer(0),
FlashFrame(0),
IsSelected(false),
IsDrawn(false)
{
}


/**
 *  Handles mouse clicks on the button.
 *
 *  @author: ZivDero
 */
bool SidebarClassExtension::TabButtonClass::Action(unsigned flags, KeyNumType& key)
{
    /*
    **	If there are no action flag bits set, then this must be a forced call. A forced call
    **	must never actually function like a real call, but rather only performs any necessary
    **	graphic updating.
    */
    if (!flags)
    {
        Flag_To_Redraw();
    }

    /*
    **	Handle the sticky state for this gadget. It must be processed here
    **	because the event flags might be cleared before the action function
    **	is called.
    */
    Sticky_Process(flags);

    /*
    **	Pass the mouse press.
    */
    if (flags & LEFTPRESS)
    {
        flags &= ~LEFTPRESS;
        ControlClass::Action(flags, key);
        key = KN_NONE;				        // erase the event
        return true;		                // stop processing other buttons now
    }

    /*
    **	Act on mouse release.
    */
    if (flags & LEFTRELEASE)
    {
        bool overbutton = (WWMouse->Get_Mouse_X() - X) < Width && (WWMouse->Get_Mouse_Y() - Y) < Height;
        if (!IsSelected && overbutton)
        {
            IsSelected = true;
            Flag_To_Redraw();
        }
        else
        {
            flags &= ~LEFTRELEASE;
        }
    }
    
    /*
    **	Do normal button processing. This ends up causing the button's ID number to
    **	be returned from the controlling Input() function.
    */
    return ControlClass::Action(flags, key);
}


/**
 *  Disables the button.
 *
 *  @author: ZivDero
 */
void SidebarClassExtension::TabButtonClass::Disable()
{
    IsSelected = false;
    Stop_Flashing();

    ControlClass::Disable();
}


/**
 *  Enables the button.
 *
 *  @author: ZivDero
 */
void SidebarClassExtension::TabButtonClass::Enable()
{
    IsSelected = false;
    Stop_Flashing();

    ControlClass::Enable();
}


/**
 *  The draw routine for the button.
 *
 *  @author: ZivDero
 */
bool SidebarClassExtension::TabButtonClass::Draw_Me(bool forced)
{
    if (!ControlClass::Draw_Me(forced))
        return false;

    if (!ShapeData)
        return false;

    if (!ShapeDrawer)
        return false;

    int shapenum;

    // A disabled tab always looks darkened
    if (IsDisabled)
    {
        shapenum = FRAME_DISABLED;
    }
    else if (IsSelected)
    {
        shapenum = FRAME_SELECTED;
    }
    else if (IsFlashing)
    {
        if (FlashTimer.Expired())
        {
            // If we're at the edge of flashing frames, restart
            if (FlashFrame == FLASH_FRAME_MAX)
                FlashFrame = FLASH_FRAME_MIN;
            else
                FlashFrame++;

            FlashTimer = FLASH_RATE;
        }

        shapenum = FlashFrame;
    }
    else
    {
        // Just the normal unselected tab
        shapenum = FRAME_NORMAL;
    }

    CC_Draw_Shape(SidebarSurface, ShapeDrawer, ShapeData, shapenum, &Point2D(X + DrawX, Y + DrawY), &ScreenRect, SHAPE_NORMAL, 0, 0, ZGRAD_GROUND, 1000, nullptr, 0, 0);

    if (MousedOver && !Scen->UserInputLocked && !IsDisabled && !IsSelected)
    {
        Rect hover_rect(X + DrawX, Y + DrawY, Width - 1, Height - 1);
        SidebarSurface->Draw_Rect(hover_rect, DSurface::RGB_To_Pixel(ColorSchemes[0]->HSV.operator RGBClass()));
    }

    IsDrawn = true;

    return true;
}


/**
 *  Sets the shape of the button.
 *
 *  @author: ZivDero
 */
void SidebarClassExtension::TabButtonClass::Set_Shape(const ShapeFileStruct* data, int width, int height)
{
    ShapeData = data;
    if (ShapeData)
    {
        Width = ShapeData->Get_Width();
        Height = ShapeData->Get_Height();
    }

    if (width != 0)
        Width = width;

    if (height != 0)
        Height = height;
}


/**
 *  Function that gets called when the mouse enters the button.
 *  Used for hover effects.
 *
 *  @author: Rampastring
 */
void SidebarClassExtension::TabButtonClass::On_Mouse_Enter()
{
    MousedOver = true;
    Map.IsToFullRedraw = true;
    Map.Flag_To_Redraw();
    RedrawSidebar = true;
}


/**
 *  Function that gets called when the mouse leaves the button.
 *  Used for hover effects.
 *
 *  @author: Rampastring
 */
void SidebarClassExtension::TabButtonClass::On_Mouse_Leave()
{
    MousedOver = false;
    Map.IsToFullRedraw = true;
    Map.Flag_To_Redraw();
    RedrawSidebar = true;
}


/**
 *  Makes the button start flashing.
 *
 *  @author: ZivDero
 */
void SidebarClassExtension::TabButtonClass::Start_Flashing()
{
    IsFlashing = true;
    FlashTimer.Start();
    FlashTimer = FLASH_RATE;
    FlashFrame = FLASH_FRAME_START;
}


/**
 *  Makes the button stop flashing.
 *
 *  @author: ZivDero
 */
void SidebarClassExtension::TabButtonClass::Stop_Flashing()
{
    IsFlashing = false;
    FlashTimer.Stop();
    FlashFrame = FLASH_FRAME_START;
}


/**
 *  Selects the button.
 *
 *  @author: ZivDero
 */
void SidebarClassExtension::TabButtonClass::Select()
{
    IsSelected = true;
}


/**
 *  Deselects the button.
 *
 *  @author: ZivDero
 */
void SidebarClassExtension::TabButtonClass::Deselect()
{
    IsSelected = false;
}


/**
 *  Function that gets called when the mouse enters the button.
 *  Used for hover effects.
 *
 *  @author: ZivDero
 */
void SidebarClassExtension::ViniferaSelectClass::On_Mouse_Enter()
{
    MousedOver = true;
    Map.IsToFullRedraw = true;
    Map.Flag_To_Redraw();
    RedrawSidebar = true;
}


/**
 *  Function that gets called when the mouse leaves the button.
 *  Used for hover effects.
 *
 *  @author: ZivDero
 */
void SidebarClassExtension::ViniferaSelectClass::On_Mouse_Leave()
{
    MousedOver = false;
    Map.IsToFullRedraw = true;
    Map.Flag_To_Redraw();
    RedrawSidebar = true;
}


/**
 *  Function that checks if the mouse has entered/left a button.
 *  This function is hooked into GadgetClass::Input()
 *
 *  @author: ZivDero, Rampastring
 */
void SidebarClassExtension::Check_Hover(GadgetClass* gadget, int mousex, int mousey)
{
    GadgetClass* to_enter = gadget->Extract_Gadget_At_Mouse(mousex, mousey);
    if (to_enter != LastHovered)
    {
        if (LastHovered)
        {
            // The hovered-on control can be an instance of either ViniferaSelectClass or TabButtonClass
            if (auto select = dynamic_cast<ViniferaSelectClass*>(LastHovered))
            {
                select->On_Mouse_Leave();
            }
            else if (auto select = dynamic_cast<TabButtonClass*>(LastHovered))
            {
                select->On_Mouse_Leave();
            }

            LastHovered = nullptr;
        }

        if (to_enter)
        {
            if (auto select = dynamic_cast<ViniferaSelectClass*>(to_enter))
            {
                LastHovered = select;
                select->On_Mouse_Enter();
            }
            else if (auto select = dynamic_cast<TabButtonClass*>(to_enter))
            {
                LastHovered = select;
                select->On_Mouse_Enter();
            }
        }
    }
}

