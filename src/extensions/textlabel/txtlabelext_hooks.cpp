/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TXTLABEL_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended TextLabelClass.
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
#include "txtlabelext_hooks.h"
#include "txtlabel.h"
#include "tibsun_globals.h"
#include "colorscheme.h"
#include "wwfont.h"
#include "uicontrol.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"
#include "vinifera_util.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class TextLabelClassExt final : public TextLabelClass
{
    public:
        bool _Draw_Me(bool forced = false);
};


/**
 *  Reimplementation of TextLabelClass::Draw_Me.
 * 
 *  @author: CCHyper
 */
bool TextLabelClassExt::_Draw_Me(bool forced)
{
    if (!GadgetClass::Draw_Me(forced)) {
        return false;
    }

    if (!ColorSchemes.Count()) {
        return false;
    }

    ColorScheme *scheme = ColorSchemes[Color];
    if (!scheme) {
        return false;
    }

    Point2D xy = Point2D(X, Y);
    Rect rect = LogicSurface->Get_Rect();
    TextPrintType style = Style;

    /**
     *  #issue-74
     * 
     *  Adds option to control the transparency of the text background.
     * 
     *  @author: CCHyper
     */
    if (UIControls->TextLabelBackgroundTransparency > 0) {

        RGBClass black_color(0,0,0);
        WWFontClass *font = Font_Ptr(style);

        Rect text_rect;
        font->String_Pixel_Rect(Text, &text_rect);

        /**
         *  Kludge to remove the space at the end of a line as it is being typed.
         */
        if (Text[std::strlen(Text)-1] == ' ') {
            text_rect.Width -= font->Char_Pixel_Width(' ');
        }

        /**
         *  Move the rect into place. Due to the returned rect of the text and
         *  how the text label is positioned, we need to manually adjust all
         *  all lines other than the first.
         */
        if (Y == TacticalRect.Y) {
            text_rect.Y += Y;
        } else {
            text_rect.Y += Y+2;
            text_rect.Height -= 2;
        }

        LogicSurface->Fill_Rect_Trans(text_rect, black_color,
            UIControls->TextLabelBackgroundTransparency);
    }

    /**
     *  #issue-74
     * 
     *  Adds option to set if the text has an outline or not.
     * 
     *  @author: CCHyper
     */
    if (!UIControls->IsTextLabelOutline) {
        style |= TPF_NOSHADOW;
    }

    if (PixWidth == -1) {
        Simple_Text_Print(Text, LogicSurface, &rect, &xy, scheme, COLOR_TBLACK, style);
    } else {
        Conquer_Clip_Text_Print(Text, LogicSurface, &rect, &xy, scheme, COLOR_TBLACK, style, PixWidth);
    }

#ifndef NDEBUG
    //DEV_DEBUG_INFO("Label: '%s'\n", Text);
#endif

    return true;
}


/**
 *  Main function for patching the hooks.
 */
void TextLabelClassExtension_Hooks()
{
    Patch_Jump(0x0064D120, &TextLabelClassExt::_Draw_Me);
}
