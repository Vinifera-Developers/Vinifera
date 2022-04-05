/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TACTICALEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended Tactical class.
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
#include "tacticalext.h"
#include "tactical.h"
#include "wwcrc.h"
#include "tibsun_globals.h"
#include "colorscheme.h"
#include "rgb.h"
#include "wwfont.h"
#include "foot.h"
#include "unit.h"
#include "unittype.h"
#include "session.h"
#include "scenario.h"
#include "asserthandler.h"
#include "debughandler.h"


TacticalMapExtension *TacticalExtension = nullptr;


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
TacticalMapExtension::TacticalMapExtension(Tactical *this_ptr) :
    Extension(this_ptr),

    IsInfoTextSet(false),
    InfoTextBuffer(),
    InfoTextPosition(BOTTOM_LEFT),
    InfoTextNotifySound(VOC_NONE),
    InfoTextNotifySoundVolume(1.0f),
    InfoTextStyle(TPF_6PT_GRAD|TPF_DROPSHADOW),
    InfoTextTimer(0)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TacticalMapExtension constructor - 0x%08X\n", (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("TacticalMapExtension constructor - 0x%08X\n", (uintptr_t)(ThisPtr));

    IsInitialized = true;
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
TacticalMapExtension::TacticalMapExtension(const NoInitClass &noinit) :
    Extension(noinit),

    IsInfoTextSet(false),
    InfoTextTimer(noinit)
{
    IsInitialized = false;
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
TacticalMapExtension::~TacticalMapExtension()
{
    //EXT_DEBUG_TRACE("TacticalMapExtension destructor - 0x%08X\n", (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("TacticalMapExtension destructor - 0x%08X\n", (uintptr_t)(ThisPtr));

    IsInitialized = false;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *
 *  As TacticalMapExtension is static data, we do not need to request
 *  pointer remap of "ThisPtr" after loading has finished.
 *  
 *  @author: CCHyper
 */
HRESULT TacticalMapExtension::Load(IStream *pStm)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TacticalMapExtension::Load - 0x%08X\n", (uintptr_t)(ThisPtr));

    HRESULT hr = ExtensionBase::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    LONG id;
    hr = pStm->Read(&id, sizeof(id), nullptr);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    ULONG size = Size_Of();
    hr = pStm->Read(this, size, nullptr);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) TacticalMapExtension(NoInitClass());

    SWIZZLE_HERE_I_AM(id, this);

#ifndef NDEBUG
    EXT_DEBUG_INFO("TacticalExt Load: ID 0x%08X Ptr 0x%08X\n", id, this);
#endif

    return S_OK;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT TacticalMapExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TacticalMapExtension::Save - 0x%08X\n", (uintptr_t)(ThisPtr));

    HRESULT hr = Extension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: CCHyper
 */
int TacticalMapExtension::Size_Of() const
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TacticalMapExtension::Size_Of - 0x%08X\n", (uintptr_t)(ThisPtr));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void TacticalMapExtension::Detach(TARGET target, bool all)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TacticalMapExtension::Detach - 0x%08X\n", (uintptr_t)(ThisPtr));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void TacticalMapExtension::Compute_CRC(WWCRCEngine &crc) const
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("TacticalMapExtension::Compute_CRC - 0x%08X\n", (uintptr_t)(ThisPtr));
}


/**
 *  Draws the developer mode overlay.
 * 
 *  @authors: CCHyper
 */
void TacticalMapExtension::Draw_Debug_Overlay()
{
    ASSERT(ThisPtr != nullptr);

    RGBClass rgb_black(0,0,0);
    unsigned color_black = DSurface::RGBA_To_Pixel(0, 0, 0);
    ColorScheme *text_color = ColorScheme::As_Pointer("White");

    int padding = 2;

    char buffer[256];
    std::snprintf(buffer, sizeof(buffer),
        "[%s] %3d %3d 0x%08X",
        strupr(Scen->ScenarioName),
        Session.DesiredFrameRate,
        FramesPerSecond,
        CurrentObjects.Count() == 1 ? CurrentObjects.Fetch_Head() : 0
    );

    /**
     * Fetch the text occupy area.
     */
    Rect text_rect;
    GradFont6Ptr->String_Pixel_Rect(buffer, &text_rect);

    /**
     *  Fill the background area.
     */
    Rect fill_rect;
    fill_rect.X = 160; // Width of Options tab, so we draw from there.
    fill_rect.Y = 0;
    fill_rect.Width = text_rect.Width+(padding+1);
    fill_rect.Height = 16; // Tab bar height
    CompositeSurface->Fill_Rect(fill_rect, color_black);

    /**
     *  Move rects into position.
     */
    text_rect.X = fill_rect.X+padding;
    text_rect.Y = 0;
    text_rect.Width += padding;
    text_rect.Height += 3;

    /**
     *  Draw the overlay text.
     */
    Fancy_Text_Print(buffer, CompositeSurface, &CompositeSurface->Get_Rect(),
        &Point2D(text_rect.X, text_rect.Y), text_color, COLOR_TBLACK, TextPrintType(TPF_6PT_GRAD|TPF_NOSHADOW));

    /**
     *  Draw the current frame number.
     */
    std::snprintf(buffer, sizeof(buffer), "%d", Frame);
    GradFont6Ptr->String_Pixel_Rect(buffer, &text_rect);

    fill_rect.Width = text_rect.Width+(padding+1);
    fill_rect.Height = 16;
    fill_rect.X = CompositeSurface->Get_Width()-fill_rect.Width;
    fill_rect.Y = 0;
    CompositeSurface->Fill_Rect(fill_rect, color_black);

    text_rect.X = CompositeSurface->Get_Width();
    text_rect.Y = 0;
    text_rect.Width += padding;
    text_rect.Height += 3;

    Fancy_Text_Print(buffer, CompositeSurface, &CompositeSurface->Get_Rect(),
        &Point2D(text_rect.X, text_rect.Y), text_color, COLOR_TBLACK, TextPrintType(TPF_RIGHT|TPF_6PT_GRAD|TPF_NOSHADOW));
}


#ifndef NDEBUG
/**
 *  Draws the current unit facing number.
 * 
 *  @author: CCHyper
 */
bool TacticalMapExtension::Debug_Draw_Facings()
{
    ASSERT(ThisPtr != nullptr);

    if (CurrentObjects.Count() != 1) {
        return false;
    }

    ObjectClass *object = CurrentObjects.Fetch_Head();
    if (object->What_Am_I() != RTTI_UNIT) {
        return false;
    }

    UnitClass *unit = reinterpret_cast<UnitClass *>(object);

    Point3D lept = unit->Class_Of()->Lepton_Dimensions();
    Point3D lept_center = Point3D(lept.X/2, lept.Y/2, lept.Z/2);

    Point3D pix = unit->Class_Of()->Pixel_Dimensions();
    Point3D pixel_center = Point3D(pix.X/2, pix.Y/2, pix.Z/2);

    Coordinate coord = unit->Center_Coord();

    Point2D screen = TacticalMap->func_60F150(coord);

    screen.X -= TacticalMap->field_5C.X;
    screen.Y -= TacticalMap->field_5C.Y;

    screen.X += TacticalRect.X;
    screen.Y += TacticalRect.Y;

    TempSurface->Fill_Rect(TacticalRect, Rect(screen.X, screen.Y, 2, 2), DSurface::RGBA_To_Pixel(255,0,0));

    TextPrintType style = TPF_CENTER|TPF_FULLSHADOW|TPF_6POINT;
    WWFontClass *font = Font_Ptr(style);

    screen.Y -= font->Get_Char_Height()/2;

    char buffer1[32];
    char buffer2[32];

    std::snprintf(buffer1, sizeof(buffer1), "%d", unit->PrimaryFacing.Current().Get_Dir());
    std::snprintf(buffer2, sizeof(buffer2), "%d", unit->PrimaryFacing.Current().Get_Raw());

    Simple_Text_Print(buffer1, TempSurface, &TacticalRect, &screen, ColorScheme::As_Pointer("White"), style);

    screen.Y += 10;
    Simple_Text_Print(buffer2, TempSurface, &TacticalRect, &screen, ColorScheme::As_Pointer("White"), style);

    return true;
}
#endif


/**
 *  Draws the overlay for frame step mode.
 * 
 *  @authors: CCHyper
 */
void TacticalMapExtension::Draw_FrameStep_Overlay()
{
    ASSERT(ThisPtr != nullptr);

    RGBClass rgb_black(0,0,0);
    unsigned color_black = DSurface::RGBA_To_Pixel(0, 0, 0);
    ColorScheme *text_color = ColorScheme::As_Pointer("White");

    int padding = 2;

    const char *text = "Frame Step Mode Enabled";

    /**
     * Fetch the text occupy area.
     */
    Rect text_rect;
    GradFont6Ptr->String_Pixel_Rect(text, &text_rect);

    /**
     *  Fill the background area.
     */
    Rect fill_rect;
    fill_rect.X = TacticalRect.X+TacticalRect.Width-text_rect.Width-(padding+1);
    fill_rect.Y = 16; // Tab bar height
    fill_rect.Width = text_rect.Width+(padding+1);
    fill_rect.Height = 16;
    CompositeSurface->Fill_Rect(fill_rect, color_black);

    /**
     *  Move rects into position.
     */
    text_rect.X = TacticalRect.X+TacticalRect.Width-1;
    text_rect.Y = fill_rect.Y;
    text_rect.Width += padding;
    text_rect.Height += 3;

    /**
     *  Draw the overlay text.
     */
    Fancy_Text_Print(text, CompositeSurface, &CompositeSurface->Get_Rect(),
        &Point2D(text_rect.X, text_rect.Y), text_color, COLOR_TBLACK, TextPrintType(TPF_RIGHT|TPF_6PT_GRAD|TPF_NOSHADOW));
}


/**
 *  Draw the overlay information text if set.
 * 
 *  @author: CCHyper
 */
void TacticalMapExtension::Draw_Information_Text()
{
    ASSERT(ThisPtr != nullptr);

    RGBClass rgb_black(0,0,0);
    unsigned color_black = DSurface::RGBA_To_Pixel(0, 0, 0);
    ColorScheme *text_color = ColorScheme::As_Pointer("White");

    int padding = 2;

    if (!TacticalExtension) {
        return;
    }

    const char *text = TacticalExtension->InfoTextBuffer.Peek_Buffer();
    if (!text) {
        return;
    }

    /**
     * Fetch the text occupy area.
     */
    Rect text_rect;
    GradFont6Ptr->String_Pixel_Rect(text, &text_rect);

    Rect fill_rect;

    TextPrintType style = TacticalExtension->InfoTextStyle;
    int pos_x = 0;
    int pos_y = 0;

    switch (TacticalExtension->InfoTextPosition) {

        default:
        case InfoTextPosType::TOP_LEFT:
            pos_x = TacticalRect.X;
            pos_y = TacticalRect.Y;
            
            /**
             *  Move rects into position.
             */
            fill_rect.X = pos_x;
            fill_rect.Y = pos_y;
            fill_rect.Width = text_rect.Width+(padding+1)+2;
            fill_rect.Height = text_rect.Height+1;

            text_rect.X = fill_rect.X+2;
            text_rect.Y = fill_rect.Y;
            text_rect.Width += padding;
            text_rect.Height += 3;

            break;

        case InfoTextPosType::TOP_RIGHT:
            pos_x = TacticalRect.X+TacticalRect.Width-text_rect.Width;
            pos_y = TacticalRect.Y;
            
            /**
             *  Move rects into position.
             */
            fill_rect.X = pos_x-5;
            fill_rect.Y = pos_y;
            fill_rect.Width = TacticalRect.X+TacticalRect.Width-text_rect.Width+3;
            fill_rect.Height = text_rect.Height+1;

            text_rect.X = TacticalRect.X+TacticalRect.Width-2;
            text_rect.Y = fill_rect.Y;
            text_rect.Width += padding;
            text_rect.Height += 3;

            style |= TPF_RIGHT;
            break;

        case InfoTextPosType::BOTTOM_LEFT:
            pos_x = 0;
            pos_y = TacticalRect.Y+TacticalRect.Height-text_rect.Height;
            
            /**
             *  Move rects into position.
             */
            fill_rect.X = pos_x;
            fill_rect.Y = pos_y;
            fill_rect.Width = text_rect.Width+(padding+1)+2;
            fill_rect.Height = text_rect.Height+1;

            text_rect.X = fill_rect.X+2;
            text_rect.Y = fill_rect.Y;
            text_rect.Width += padding;
            text_rect.Height += 3;

            break;

        case InfoTextPosType::BOTTOM_RIGHT:
            pos_x = TacticalRect.X+TacticalRect.Width-text_rect.Width;
            pos_y = TacticalRect.Y+TacticalRect.Height-text_rect.Height;

            /**
             *  Move rects into position.
             */
            fill_rect.X = pos_x-5;
            fill_rect.Y = pos_y;
            fill_rect.Width = TacticalRect.X+TacticalRect.Width-text_rect.Width+3;
            fill_rect.Height = text_rect.Height+1;

            text_rect.X = TacticalRect.X+TacticalRect.Width-2;
            text_rect.Y = fill_rect.Y;
            text_rect.Width += padding;
            text_rect.Height += 3;

            style |= TPF_RIGHT;

            break;

    };

    /**
     *  Fill the background area.
     */
    CompositeSurface->Fill_Rect_Trans(fill_rect, rgb_black, 50);

    /**
     *  Draw the overlay text.
     */
    Fancy_Text_Print(text, CompositeSurface, &CompositeSurface->Get_Rect(),
        &Point2D(text_rect.X, text_rect.Y), text_color, COLOR_TBLACK, style);
}
