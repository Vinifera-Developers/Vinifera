/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TABEXT_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for the extended TabClass.
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
#include "tabext_hooks.h"

#include "drawshape.h"
#include "extension_globals.h"
#include "hooker.h"
#include "language.h"
#include "mouse.h"
#include "optionsext.h"
#include "shapeset.h"
#include "tab.h"
#include "textprint.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class TabClassExt : public TabClass
{
public:
    void _Draw_It(bool complete);
    void _AI(KeyNumType& input, Point2D const& xy);
};

void TabClassExt::_Draw_It(bool complete)
{
    if (!Debug_Map) {

        /*
        **	Redraw the top bar imagery if flagged to do so or if the entire display needs
        **	to be redrawn.
        */
        if (complete || IsToRedraw) {

            int width = SidebarSurface->Get_Width();
            int rightx = width + CompositeSurface->Get_Width() - 1;
            int tab_height = OptionsExtension->SidebarControls.TabHeight;

            for (int x = TabShape->Get_Width(); x < CompositeSurface->Get_Width(); x += TabShape->Get_Width()) {
                Draw_Shape(*CompositeSurface, *SidebarDrawer, TabShape, 1, Point2D(x, 0), CompositeSurface->Get_Rect());
            }

            Draw_Shape(*CompositeSurface, *SidebarDrawer, TabShape, 0, Point2D(0, 0), VisibleRect);
            Draw_Credits_Tab();
            LogicalSurface->Draw_Line(Point2D(0, tab_height - 2), Point2D(rightx, tab_height - 2), TBLACK);
            Fancy_Text_Print(TXT_TAB_BUTTON_CONTROLS, *LogicalSurface, LogicalSurface->Get_Rect(), Point2D(OptionsExtension->SidebarControls.SidebarWidth / 2, 0), ColorSchemes[0], TBLACK, TPF_USE_GRAD_PAL | TPF_CENTER | TPF_METAL12);

            if (LogicalSurface != TileSurface) {
                TileSurface->Copy_From(Rect(0, 0, TileSurface->Get_Width(), tab_height), *LogicalSurface, Rect(0, 0, TileSurface->Get_Width(), tab_height));
            }
        }
    }

    if (!Debug_Map) {
        Credits.Graphic_Logic(complete || IsToRedraw);
        IsToRedraw = false;
    }

    SidebarClass::Draw_It(complete);
}

void TabClassExt::_AI(KeyNumType& input, Point2D const& xy)
{
    if (!Map.IsRubberBand) {
        if (xy.Y >= 0 && xy.Y < OptionsExtension->SidebarControls.TabHeight && xy.X < (VisibleSurface->Get_Width() - 1) && xy.X > 0) {

            bool ok = false;

            /*
            **	If the mouse is at the top of the screen, then the tab bars only work
            **	in certain areas. If the special scroll modification is not active, then
            **	the tabs never work when the mouse is at the top of the screen.
            */
            if (xy.Y > 0) {
                ok = true;
            }

            if (ok) {
                if (input == KN_LMOUSE) {
                    int sel = 0;
                    if (Options.SidebarSide != 0) {
                        if (xy.X >= OptionsExtension->SidebarControls.SidebarWidth) sel = -1;
                    } else {
                        if (xy.X <= VisibleRect.Width - OptionsExtension->SidebarControls.SidebarWidth || xy.X >= VisibleRect.Width) sel = -1;
                    }
                    if (sel >= 0) {
                        Set_Active(sel);
                        input = KN_NONE;
                    }
                }

                Override_Mouse_Shape(MOUSE_NORMAL, false);
            }
        }
    }

    if (MoneyFlashTimer == 1) {
        IsToRedraw = true;
        Flag_To_Redraw();
    }

    Credits.AI();
    SidebarClass::AI(input, const_cast<Point2D&>(xy));
}


/**
 *  Main function for patching the hooks.
 */
void TabClassExtension_Hooks()
{
    Patch_Jump(0x0060E980, &TabClassExt::_AI);
    Patch_Jump(0x0060E440, &TabClassExt::_Draw_It);
}