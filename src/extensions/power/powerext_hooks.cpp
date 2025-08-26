/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          POWEREXT_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for the extended PowerClass.
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
#include "powerext_hooks.h"

#include "building.h"
#include "drawshape.h"
#include "extension_globals.h"
#include "hooker.h"
#include "house.h"
#include "language.h"
#include "mouse.h"
#include "newsidebar.h"
#include "optionsext.h"
#include "power.h"
#include "tooltip.h"
#include "uicontrol.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class PowerClassExt : public PowerClass
{
public:
    int _Max_Power_Height();
    int _Desired_Power_Height();
    int _Desired_Levels(int& green, int& yellow, int& red);
    void _Draw_It(bool complete);
    void _Set_Dimensions();
};


int PowerClassExt::_Max_Power_Height()
{
    return (UIControls->SidebarControls.ObjectHeight * NewSidebarClass::Max_Visible() + UIControls->SidebarControls.PowerHeightFudge) / UIControls->SidebarControls.PowerPipHeight;
}


int PowerClassExt::_Desired_Power_Height()
{
    int max_pips = Max_Power_Height();
    int drain = 0;
    int power = 0;

    for (int i = 0; i < Buildings.Count(); i++) {
        if (Buildings[i]->House == PlayerPtr) {
            drain += Buildings[i]->Class->Drain;
            power += Buildings[i]->Class->Power;
        }
    }

    int empty_pips = 400.0 / (drain + power + 400.0) * max_pips;
    empty_pips = std::max(empty_pips, 0);
    empty_pips = std::min(empty_pips, max_pips - 1);

    return max_pips - empty_pips;
}


int PowerClassExt::_Desired_Levels(int& green, int& yellow, int& red)
{
    int max_pips = Max_Power_Height();
    int desired_pips = Desired_Power_Height();

    int drain = 0;
    int power = 0;

    for (int i = 0; i < Buildings.Count(); i++) {
        BuildingClass* bptr = Buildings[i];
        drain += bptr->Class->Drain;
        power += bptr->Class->Power;
    }

    double power_delta = PlayerPtr->Power_Output() - PlayerPtr->Power_Drain();

    double green_power = 0.0;
    double yellow_power = 100.0;

    if (power_delta < 0.0) {
        yellow_power = 0.0;
        green_power = 0.0;
    } else {
        if (power_delta < 100.0) {
            yellow_power = power_delta;
        }
        green_power = power_delta - yellow_power;
    }

    double red_fraction = 1.0;
    double green_fraction = 0.0;
    double yellow_fraction = 0.0;

    double total_power = PlayerPtr->Power_Drain() + yellow_power + green_power;

    if (total_power > 0.0) {
        red_fraction = PlayerPtr->Power_Drain() / total_power;
        green_fraction = green_power / total_power;
        yellow_fraction = yellow_power / total_power;
    }

    red = desired_pips * red_fraction;
    yellow = desired_pips * yellow_fraction;
    green = desired_pips * green_fraction;

    red += desired_pips * green_fraction - green + (desired_pips * yellow_fraction - yellow) + (desired_pips * red_fraction - red) + 0.01;

    return max_pips;
}


void PowerClassExt::_Draw_It(bool complete)
{
    if (complete || PowerClass::IsToRedraw) {

        if (Map.IsSidebarActive) {
            PowerClass::IsToRedraw = false;
            RedrawSidebar = true;

            Rect rect = SidebarSurface->Get_Rect();
            int x = UIControls->SidebarControls.PowerPosition.X;
            int y = SidebarRect.Y + UIControls->SidebarControls.PowerPosition.Y;

            int num = Max_Power_Height() - RedPipCount - YellowPipCount - GreenPipCount;

            int index;
            for (index = 0; index < num; index++) {
                Draw_Shape(*SidebarSurface, *SidebarDrawer, PowerPipShape, POWER_PIP_EMPTY, Point2D(x, y), rect, SHAPE_WIN_REL);
                y += UIControls->SidebarControls.PowerPipHeight;
            }

            index = 0;
            if (FlashCount > 0) {
                if ((FlashCount % 2) == 0) {
                    Draw_Shape(*SidebarSurface, *SidebarDrawer, PowerPipShape, POWER_PIP_WHITE, Point2D(x, y), rect, SHAPE_WIN_REL);
                    y += UIControls->SidebarControls.PowerPipHeight;
                    index++;
                }
            }

            if (GreenPipCount > 0) {
                while (index < GreenPipCount) {
                    Draw_Shape(*SidebarSurface, *SidebarDrawer, PowerPipShape, POWER_PIP_GREEN, Point2D(x, y), rect, SHAPE_WIN_REL);
                    y += UIControls->SidebarControls.PowerPipHeight;
                    index++;
                }
                index = 0;
            }

            if (YellowPipCount > 0) {
                while (index < YellowPipCount) {
                    Draw_Shape(*SidebarSurface, *SidebarDrawer, PowerPipShape, POWER_PIP_YELLOW, Point2D(x, y), rect, SHAPE_WIN_REL);
                    y += UIControls->SidebarControls.PowerPipHeight;
                    index++;
                }
                index = 0;
            }

            if (RedPipCount > 0) {
                while (index < RedPipCount) {
                    Draw_Shape(*SidebarSurface, *SidebarDrawer, PowerPipShape, POWER_PIP_RED, Point2D(x, y), rect, SHAPE_WIN_REL);
                    y += UIControls->SidebarControls.PowerPipHeight;
                    index++;
                }
                index = 0;
            }
        }
    }

    RadarClass::Draw_It(complete);
}


void PowerClassExt::_Set_Dimensions(void)
{
    RadarClass::Set_Dimensions();
    if (ToolTips != nullptr) {

        ToolTip tt;
        tt.Text = TXT_NONE;
        tt.ID = GADGET_POWER;
        tt.Region.X = SidebarRect.X + UIControls->SidebarControls.PowerPosition.X;
        tt.Region.Y = SidebarRect.Y + UIControls->SidebarControls.PowerPosition.Y;
        tt.Region.Width = UIControls->SidebarControls.PowerWidth;
        tt.Region.Height = UIControls->SidebarControls.ObjectHeight * NewSidebarClass::Max_Visible() + UIControls->SidebarControls.PowerHeightFudge;

        ToolTips->Remove(tt.ID);
        ToolTips->Add(&tt);
    }
}


/**
 *  Main function for patching the hooks.
 */
void PowerClassExtension_Hooks()
{
    Patch_Jump(0x005AB240, &PowerClassExt::_Max_Power_Height);
    Patch_Jump(0x005AB140, &PowerClassExt::_Desired_Power_Height);
    Patch_Jump(0x005AB260, &PowerClassExt::_Desired_Levels);
    Patch_Jump(0x005AB460, &PowerClassExt::_Draw_It);
    Patch_Jump(0x005ABCC0, &PowerClassExt::_Set_Dimensions);
}