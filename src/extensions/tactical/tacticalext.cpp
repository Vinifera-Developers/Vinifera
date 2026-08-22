/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended Tactical class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "tacticalext.h"

#include "beacon.h"
#include "colorscheme.h"
#include "debughandler.h"
#include "ebolt.h"
#include "extension.h"
#include "extension_globals.h"
#include "foot.h"
#include "house.h"
#include "housetype.h"
#include "mouse.h"
#include "optionsext.h"
#include "rgb.h"
#include "rulesext.h"
#include "scenario.h"
#include "scenarioext.h"
#include "sdlsurface.h"
#include "session.h"
#include "super.h"
#include "superext.h"
#include "supertype.h"
#include "supertypeext.h"
#include "tactical.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "uicontrol.h"
#include "unit.h"
#include "vinifera_globals.h"
#include "vinifera_saveload.h"
#include "vinifera_util.h"
#include "wwfont.h"
#include "wwmouse.h"

#include <windows.h>


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
TacticalExtension::TacticalExtension(const Tactical* this_ptr) :
    GlobalExtensionClass(this_ptr),
    IsInfoTextSet(false),
    InfoTextBuffer(),
    InfoTextPosition(BOTTOM_LEFT),
    InfoTextNotifySound(VOC_NONE),
    InfoTextNotifySoundVolume(1.0f),
    InfoTextStyle(TPF_6PT_GRAD | TPF_DROPSHADOW),
    InfoTextTimer(0),
    CellRedrawCount(0),
    IsTemplatedTextVisible(false),
    TemplatedTextIndex(""),
    TemplatedTextPosition(TOP_RIGHT),
    TemplatedTextColor(COLORSCHEME_NONE),
    TemplatedTextStyle(TPF_6PT_GRAD | TPF_DROPSHADOW),
    IsTemplatedTextCached(false),
    TemplatedTextCache {""},
    IsBeaconPlacementMode(false),
    IsEditingBeaconText(false),
    SubtitleCategoryCur(SUBTITLE_CATEGORY_SYSTEM),
    SubtitleFont(nullptr),
    SubtitleFontCacheHeight(0),
    SubtitleFontCacheWeight(0),
    LastHarvesterUnderAttackFrame(0)
{
    std::memset(CellRedraw, 0, sizeof(CellRedraw));
}


/**
 *  Class no-init constructor.
 *
 *  @author: CCHyper
 */
TacticalExtension::TacticalExtension(const NoInitClass& noinit) :
    GlobalExtensionClass(noinit),
    InfoTextTimer(noinit),
    TemplatedTextIndex(noinit)
{
}


/**
 *  Class destructor.
 *
 *  @author: CCHyper
 */
TacticalExtension::~TacticalExtension()
{
    Invalidate_Subtitle_Font();
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *
 *  @author: CCHyper
 */
HRESULT TacticalExtension::Load(IStream* pStm)
{
    HRESULT hr = GlobalExtensionClass::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) TacticalExtension(NoInitClass());

    for (int i = 0; i < CellRedrawCount; i++) {
        VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(CellRedraw[i], "CellRedraw");
    }

    return hr;
}


/**
 *  Saves an object to the specified stream.
 *
 *  @author: CCHyper
 */
HRESULT TacticalExtension::Save(IStream* pStm, BOOL fClearDirty)
{
    HRESULT hr = GlobalExtensionClass::Save(pStm, fClearDirty);
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
int TacticalExtension::Get_Object_Size() const
{
    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *
 *  @author: CCHyper
 */
void TacticalExtension::Object_CRC(CRCEngine& crc) const
{
}


/**
 *  Set the information text to be displayed.
 *
 *  @authors: CCHyper
 */
void TacticalExtension::Set_Info_Text(const char* text)
{
    std::strncpy(TacticalMapExtension->InfoTextBuffer, text, sizeof(TacticalMapExtension->InfoTextBuffer));
    TacticalMapExtension->InfoTextBuffer[sizeof(InfoTextBuffer) - 1] = '\0';
}


/**
 *  Enables the variable counter.
 *
 *  @authors: ZivDero
 */
void TacticalExtension::Enable_Templated_Text(std::string_view label, ColorSchemeType color)
{
    IsTemplatedTextVisible = true;
    TemplatedTextIndex = label;
    TemplatedTextColor = color;
    IsTemplatedTextCached = false;
}


/**
 *  Disables the variable counter.
 *
 *  @authors: ZivDero
 */
void TacticalExtension::Disable_Templated_Text()
{
    IsTemplatedTextVisible = false;
    IsTemplatedTextCached = false;
}


/**
 *  Draws the version number on screen for non-release builds.
 *
 *  @authors: CCHyper
 */
void TacticalExtension::Draw_Version_Number_Text()
{
#ifndef RELEASE
    if (!Vinifera_NoTacticalVersionString) {
#else
    if (false) {
#endif
        Vinifera_Draw_Version_Text(CompositeSurface);
    }
}


/**
 *  Draws the developer mode overlay.
 *
 *  @authors: CCHyper
 */
void TacticalExtension::Draw_Debug_Overlay()
{
    RGBClass rgb_black(0, 0, 0);
    unsigned color_black = DSurface::Build_Hicolor_Pixel(0, 0, 0);
    ColorScheme *text_color = Fetch_Scheme_By_Name("White");

    int padding = 2;

    char buffer[256];
    std::snprintf(buffer, sizeof(buffer),
        "[%s] %3d %3d 0x%08X",
        strupr(Scen->ScenarioName),
        Session.DesiredFrameRate,
        FramesPerSecond,
        CurrentObjects.Count() == 1 ? reinterpret_cast<int>(CurrentObjects.Fetch_Head()) : 0
    );

    /**
     * Fetch the text occupy area.
     */
    Rect text_rect;
    GradFont6Ptr->String_Pixel_Bounds(buffer, text_rect);

    /**
     *  Fill the background area.
     */
    Rect fill_rect;
    fill_rect.X = 160; // Width of Options tab, so we draw from there.
    fill_rect.Y = 0;
    fill_rect.Width = text_rect.Width + (padding + 1);
    fill_rect.Height = 16; // Tab bar height
    CompositeSurface->Fill_Rect(fill_rect, color_black);

    /**
     *  Move rects into position.
     */
    text_rect.X = fill_rect.X + padding;
    text_rect.Y = 0;
    text_rect.Width += padding;
    text_rect.Height += 3;

    /**
     *  Draw the overlay text.
     */
    Fancy_Text_Print(buffer, *CompositeSurface, CompositeSurface->Get_Rect(), Point2D(text_rect.X, text_rect.Y), text_color, COLOR_TBLACK, TPF_6PT_GRAD | TPF_NOSHADOW);

    /**
     *  Draw the current frame number.
     */
    std::snprintf(buffer, sizeof(buffer), "%d", Frame);
    GradFont6Ptr->String_Pixel_Bounds(buffer, text_rect);

    fill_rect.Width = text_rect.Width + (padding + 1);
    fill_rect.Height = 16;
    fill_rect.X = CompositeSurface->Get_Width() - fill_rect.Width;
    fill_rect.Y = 0;
    CompositeSurface->Fill_Rect(fill_rect, color_black);

    text_rect.X = CompositeSurface->Get_Width();
    text_rect.Y = 0;
    text_rect.Width += padding;
    text_rect.Height += 3;

    Fancy_Text_Print(buffer, *CompositeSurface, CompositeSurface->Get_Rect(), Point2D(text_rect.X, text_rect.Y), text_color, COLOR_TBLACK, TPF_RIGHT | TPF_6PT_GRAD | TPF_NOSHADOW);
}


#ifndef NDEBUG
/**
 *  Draws the current unit facing number.
 *
 *  @author: CCHyper
 */
bool TacticalExtension::Debug_Draw_Facings()
{
    if (CurrentObjects.Count() != 1) {
        return false;
    }

    ObjectClass* object = CurrentObjects.Fetch_Head();
    if (object->RTTI != RTTI_UNIT) {
        return false;
    }

    UnitClass* unit = reinterpret_cast<UnitClass*>(object);

    Point3D lept = unit->Class_Of()->Lepton_Dimensions();
    Point3D lept_center = Point3D(lept.X / 2, lept.Y / 2, lept.Z / 2);

    Point3D pix = unit->Class_Of()->Pixel_Dimensions();
    Point3D pixel_center = Point3D(pix.X / 2, pix.Y / 2, pix.Z / 2);

    Coord coord = unit->Center_Coord();

    Point2D screen = TacticalMap->func_60F150(coord);

    screen.X -= TacticalMap->field_5C.X;
    screen.Y -= TacticalMap->field_5C.Y;

    screen.X += TacticalRect.X;
    screen.Y += TacticalRect.Y;

    LogicalSurface->Fill_Rect(TacticalRect, Rect(screen.X, screen.Y, 2, 2), DSurface::Build_Hicolor_Pixel(255, 0, 0));

    TextPrintType style = TPF_CENTER | TPF_FULLSHADOW | TPF_6POINT;
    FontClass* font = Font_Ptr(style);

    screen.Y -= font->Get_Height() / 2;

    char buffer1[32];
    char buffer2[32];

    std::snprintf(buffer1, sizeof(buffer1), "%d", unit->PrimaryFacing.Current().Get_Dir());
    std::snprintf(buffer2, sizeof(buffer2), "%d", unit->PrimaryFacing.Current().Get_Raw());

    Simple_Text_Print(buffer1, *LogicalSurface, TacticalRect, screen, Fetch_Scheme_By_Name("White"), COLOR_TBLACK, style, 1);

    screen.Y += 10;
    Simple_Text_Print(buffer2, *LogicalSurface, TacticalRect, screen, Fetch_Scheme_By_Name("White"), COLOR_TBLACK, style, 1);

    return true;
}
#endif


/**
 *  Draws the overlay for frame step mode.
 *
 *  @authors: CCHyper
 */
void TacticalExtension::Draw_FrameStep_Overlay()
{
    RGBClass rgb_black(0, 0, 0);
    unsigned color_black = DSurface::Build_Hicolor_Pixel(0, 0, 0);
    ColorScheme *text_color = Fetch_Scheme_By_Name("White");

    int padding = 2;

    const char* text = "Frame Step Mode Enabled";

    /**
     * Fetch the text occupy area.
     */
    Rect text_rect;
    GradFont6Ptr->String_Pixel_Bounds(text, text_rect);

    /**
     *  Fill the background area.
     */
    Rect fill_rect;
    fill_rect.X = TacticalRect.X + TacticalRect.Width - text_rect.Width - (padding + 1);
    fill_rect.Y = 16; // Tab bar height
    fill_rect.Width = text_rect.Width + (padding + 1);
    fill_rect.Height = 16;
    CompositeSurface->Fill_Rect(fill_rect, color_black);

    /**
     *  Move rects into position.
     */
    text_rect.X = TacticalRect.X + TacticalRect.Width - 1;
    text_rect.Y = fill_rect.Y;
    text_rect.Width += padding;
    text_rect.Height += 3;

    /**
     *  Draw the overlay text.
     */
    Fancy_Text_Print(text, *CompositeSurface, CompositeSurface->Get_Rect(), Point2D(text_rect.X, text_rect.Y), text_color, COLOR_TBLACK, TPF_RIGHT | TPF_6PT_GRAD | TPF_NOSHADOW);
}


/**
 *  Draw the overlay information text if set.
 *
 *  @author: CCHyper
 */
void TacticalExtension::Draw_Information_Text()
{
    if (!IsInfoTextSet) {
        return;
    }

    int padding = 2;

    const char* text = InfoTextBuffer;

    /**
     * Fetch the text occupy area.
     */
    Rect text_rect;
    GradFont6Ptr->String_Pixel_Bounds(text, text_rect);

    Rect fill_rect;

    TextPrintType style = InfoTextStyle;
    int pos_x = 0;
    int pos_y = 0;

    switch (InfoTextPosition) {
    default:
    case InfoTextPosType::TOP_LEFT:
        pos_x = TacticalRect.X;
        pos_y = TacticalRect.Y;

        /**
         *  Move rects into position.
         */
        fill_rect.X = pos_x;
        fill_rect.Y = pos_y;
        fill_rect.Width = text_rect.Width + (padding + 1) + 2;
        fill_rect.Height = text_rect.Height + 1;

        text_rect.X = fill_rect.X + 2;
        text_rect.Y = fill_rect.Y;
        text_rect.Width += padding;
        text_rect.Height += 3;

        break;

    case InfoTextPosType::TOP_RIGHT:
        pos_x = TacticalRect.X + TacticalRect.Width - text_rect.Width;
        pos_y = TacticalRect.Y;

        /**
         *  Move rects into position.
         */
        fill_rect.X = pos_x - 5;
        fill_rect.Y = pos_y;
        fill_rect.Width = TacticalRect.X + TacticalRect.Width - text_rect.Width + 3;
        fill_rect.Height = text_rect.Height + 1;

        text_rect.X = TacticalRect.X + TacticalRect.Width - 2;
        text_rect.Y = fill_rect.Y;
        text_rect.Width += padding;
        text_rect.Height += 3;

        style |= TPF_RIGHT;
        break;

    case InfoTextPosType::BOTTOM_LEFT:
        pos_x = 0;
        pos_y = TacticalRect.Y + TacticalRect.Height - text_rect.Height;

        /**
         *  Move rects into position.
         */
        fill_rect.X = pos_x;
        fill_rect.Y = pos_y;
        fill_rect.Width = text_rect.Width + (padding + 1) + 2;
        fill_rect.Height = text_rect.Height + 1;

        text_rect.X = fill_rect.X + 2;
        text_rect.Y = fill_rect.Y;
        text_rect.Width += padding;
        text_rect.Height += 3;

        break;

    case InfoTextPosType::BOTTOM_RIGHT:
        pos_x = TacticalRect.X + TacticalRect.Width - text_rect.Width;
        pos_y = TacticalRect.Y + TacticalRect.Height - text_rect.Height;

        /**
         *  Move rects into position.
         */
        fill_rect.X = pos_x - 5;
        fill_rect.Y = pos_y;
        fill_rect.Width = TacticalRect.X + TacticalRect.Width - text_rect.Width + 3;
        fill_rect.Height = text_rect.Height + 1;

        text_rect.X = TacticalRect.X + TacticalRect.Width - 2;
        text_rect.Y = fill_rect.Y;
        text_rect.Width += padding;
        text_rect.Height += 3;

        style |= TPF_RIGHT;

        break;

    };

    /**
     *  Fill the background area.
     */
    CompositeSurface->Fill_Rect_Trans(fill_rect, RGBClass(0, 0, 0), 50);

    /**
     *  Draw the overlay text.
     */
    Fancy_Text_Print(text, *CompositeSurface, CompositeSurface->Get_Rect(), Point2D(text_rect.X, text_rect.Y), Fetch_Scheme_By_Name("White"), COLOR_TBLACK, style);
}


/**
 *  For drawing any new post-effects/systems.
 *
 *  @authors: CCHyper
 */
void TacticalExtension::Render_Post()
{
    /**
     *  Draw any new post effects here.
     */
    EBoltClass::Draw_All();
    BeaconManager.Draw(LogicalSurface, TacticalRect);

    /**
     *  Draw any overlay text.
     */
    Draw_Super_Timers();

    /**
     *  In beacon placement mode, holding modifier keys can give you a preset text
     *  (e.g. attack, defend). Draw it.
     */
    if (IsBeaconPlacementMode) {
        char const* beacon_text = BeaconManagerClass::Beacon_Preview_Text(BeaconManagerClass::Pick_Beacon_Placement_Action());
        if (beacon_text != nullptr) {
            Draw_Beacon_Text(beacon_text, *ColorSchemes[PlayerPtr->Scheme], Get_Mouse_Point(), VisibleRect, false, UIControls->BeaconPreviewTextOffset);
        }
    }
}


/**
 *  Prints a single super weapon timer to the tactical screen.
 *
 *  @authors: CCHyper
 */
void TacticalExtension::Super_Draw_Timer(int row_index, ColorScheme * color, int time, const char* name, unsigned long* flash_time, bool* flash_state)
{
    static FontClass* _font = nullptr;

    TextPrintType style = TPF_8POINT | TPF_RIGHT | TPF_NOSHADOW | TPF_METAL12 | TPF_SOLIDBLACK_BG;

    if (!_font) {
        _font = Font_Ptr(style);
    }

    char fullbuff[128];
    char namebuff[128];
    char timerbuff[128];
    int text_width = -1;
    int flash_delay = 500; // was 1000
    bool to_flash = false;
    unsigned color_black = DSurface::Build_Hicolor_Pixel(0, 0, 0);
    RGBClass rgb_black(0, 0, 0);
    ColorScheme *white_color = Fetch_Scheme_By_Name("White", 1);
    int background_tint = 50;

    long hours = (time / 60 / 60);
    long seconds = (time % 60);
    long minutes = (time / 60 % 60);

    if (hours) {
        std::snprintf(fullbuff, sizeof(fullbuff), "%s %d:%02d:%02d", name, hours, minutes, seconds);
        std::snprintf(namebuff, sizeof(namebuff), "%s", name);
        std::snprintf(timerbuff, sizeof(timerbuff), "%d:%02d:%02d", hours, minutes, seconds);
    }
    else {
        std::snprintf(fullbuff, sizeof(fullbuff), "%s %02d:%02d", name, minutes, seconds);
        std::snprintf(namebuff, sizeof(namebuff), "%s", name);
        std::snprintf(timerbuff, sizeof(timerbuff), "%02d:%02d", minutes, seconds);
    }

    /**
     *  Is it time to flash
     */
    if (!time) {
        if (flash_time && flash_state) {
            if (timeGetTime() >= *flash_time) {
                *flash_time = timeGetTime() + flash_delay;
                *flash_state = !*flash_state;
            }
            to_flash = *flash_state;
        }
    }

    Rect name_rect;
    _font->String_Pixel_Bounds(namebuff, name_rect);

    Rect timer_rect;
    _font->String_Pixel_Bounds(timerbuff, timer_rect);

    int font_width = _font->Get_Width();
    int font_height = _font->Get_Height();

    int y_pos = TacticalRect.Height - (row_index * (font_height + 2)) + 3;

    Point2D timer_point;
    timer_point.X = TacticalRect.Width - 4;
    timer_point.Y = y_pos;

    int x_offset = hours ? 56 : 38; // timer_rect.Width

    Point2D name_point;
    name_point.X = TacticalRect.Width - x_offset - 3;
    name_point.Y = y_pos;

    Rect fill_rect;
    fill_rect.X = TacticalRect.Width - (x_offset + name_rect.Width) - 4;
    fill_rect.Y = y_pos - 1;
    fill_rect.Width = x_offset + name_rect.Width + 2;
    fill_rect.Height = timer_rect.Height + 2;

    //CompositeSurface->Fill_Rect(CompositeSurface->Get_Rect(), fill_rect, color_black);
    CompositeSurface->Fill_Rect_Trans(fill_rect, rgb_black, background_tint);

    Fancy_Text_Print(timerbuff, *CompositeSurface, CompositeSurface->Get_Rect(), timer_point, to_flash ? white_color : color, COLOR_TBLACK, style);

    Fancy_Text_Print(namebuff, *CompositeSurface, CompositeSurface->Get_Rect(), name_point, color, COLOR_TBLACK, style);
}


/**
 *  Draws super weapon timers to the tactical screen.
 *
 *  @authors: CCHyper
 */
void TacticalExtension::Draw_Super_Timers()
{
    /**
     *  Super weapon timers are for multiplayer only.
     */
#if 0
    if (Session.Type == GAME_NORMAL) {
        return;
    }
#endif

    /**
     *  Does the game rules state that the super weapon timers should be shown?
     */
    if (!RuleExtension->IsShowSuperWeaponTimers) {
        return;
    }

    /**
     *  Has the user toggled the visibility of the super weapon timers?
     */
    if (!Vinifera_ShowSuperWeaponTimers) {
        return;
    }

    /**
     *  Non-release builds print the version information to the tactical view
     *  so we need to adjust the timers to print above this text.
     */
#ifdef RELEASE
    int row_index = 0;
#else
    int row_index = Vinifera_NoTacticalVersionString ? 0 : 3;
#endif

    /**
     *  Iterate over all active super weapons and print their recharge timers.
     */
    for (int i = 0; i < Supers.Count(); ++i) {

        SuperClass* super = Supers[i];
        SuperClassExtension* superext = Extension::Fetch(super);
        SuperWeaponTypeClassExtension* supertypeext = Extension::Fetch(super->Class);

        /**
         *  Should we show the recharge timer for this super?
         */
        if (!supertypeext->IsShowTimer) {
            continue;
        }

        if (super->House->Class->IsMultiplayPassive) {
            continue;
        }

        /**
         *  Skip supers that are disabled.
         */
        if (!super->IsPresent) {
            continue;
        }

        if (super->Control.Value() != super->Class->RechargeTime) {

            Super_Draw_Timer(
                row_index++,
                ColorSchemes[super->House->Scheme],
                super->Control.Value() / TICKS_PER_SECOND,
                super->Class->GivenName.c_str(),
                &superext->FlashTimeEnd,
                &superext->TimerFlashState
            );
        }
    }
}


/**
 *  Draw the templated text display if it's enabled.
 *
 *  @author: CCHyper, ZivDero
 */
void TacticalExtension::Draw_Templated_Text()
{
    if (!IsTemplatedTextVisible) {
        return;
    }

    int padding = 2;

    if (!Vinifera_TutorialText.contains(std::string(TemplatedTextIndex))) {
        return;
    }

    /**
     *  Substitute the placeholders in the tutorial string.
     */
    if (!IsTemplatedTextCached) {
        std::strncpy(TemplatedTextCache, ScenarioClassExtension::Substitute_Variable_Placeholders(Vinifera_TutorialText[std::string(TemplatedTextIndex)]).c_str(), sizeof(TemplatedTextCache));
        IsTemplatedTextCached = true;
    }

    /**
     *  Fetch the text occupy area.
     */
    Rect text_rect;
    GradFont6Ptr->String_Pixel_Bounds(TemplatedTextCache, text_rect);

    Rect fill_rect;

    TextPrintType style = TemplatedTextStyle;
    int pos_x = 0;
    int pos_y = 0;

    ColorSchemeType color = TemplatedTextColor;
    if (color < COLORSCHEME_FIRST || color >= ColorSchemes.Count()) {
        color = PlayerPtr->Scheme;
    }

    switch (TemplatedTextPosition) {
    default:
    case InfoTextPosType::TOP_LEFT:
        pos_x = TacticalRect.X;
        pos_y = TacticalRect.Y;

        /**
         *  Move rects into position.
         */
        fill_rect.X = pos_x;
        fill_rect.Y = pos_y;
        fill_rect.Width = text_rect.Width + (padding + 1) + 2;
        fill_rect.Height = text_rect.Height + 1;

        text_rect.X = fill_rect.X + 2;
        text_rect.Y = fill_rect.Y;
        text_rect.Width += padding;
        text_rect.Height += 3;

        break;

    case InfoTextPosType::TOP_RIGHT:
        pos_x = TacticalRect.X + TacticalRect.Width - text_rect.Width;
        pos_y = TacticalRect.Y;

        /**
         *  Move rects into position.
         */
        fill_rect.X = pos_x - 5;
        fill_rect.Y = pos_y;
        fill_rect.Width = TacticalRect.X + TacticalRect.Width - text_rect.Width + 3;
        fill_rect.Height = text_rect.Height + 1;

        text_rect.X = TacticalRect.X + TacticalRect.Width - 2;
        text_rect.Y = fill_rect.Y;
        text_rect.Width += padding;
        text_rect.Height += 3;

        style |= TPF_RIGHT;
        break;

    case InfoTextPosType::BOTTOM_LEFT:
        pos_x = 0;
        pos_y = TacticalRect.Y + TacticalRect.Height - text_rect.Height;

        /**
         *  Move rects into position.
         */
        fill_rect.X = pos_x;
        fill_rect.Y = pos_y;
        fill_rect.Width = text_rect.Width + (padding + 1) + 2;
        fill_rect.Height = text_rect.Height + 1;

        text_rect.X = fill_rect.X + 2;
        text_rect.Y = fill_rect.Y;
        text_rect.Width += padding;
        text_rect.Height += 3;

        break;

    case InfoTextPosType::BOTTOM_RIGHT:
        pos_x = TacticalRect.X + TacticalRect.Width - text_rect.Width;
        pos_y = TacticalRect.Y + TacticalRect.Height - text_rect.Height;

        /**
         *  Move rects into position.
         */
        fill_rect.X = pos_x - 5;
        fill_rect.Y = pos_y;
        fill_rect.Width = TacticalRect.X + TacticalRect.Width - text_rect.Width + 3;
        fill_rect.Height = text_rect.Height + 1;

        text_rect.X = TacticalRect.X + TacticalRect.Width - 2;
        text_rect.Y = fill_rect.Y;
        text_rect.Width += padding;
        text_rect.Height += 3;

        style |= TPF_RIGHT;

        break;

    };

    /**
     *  Fill the background area.
     */
    CompositeSurface->Fill_Rect_Trans(fill_rect, RGBClass(0, 0, 0), 50);

    /**
     *  Draw the overlay text.
     */
    Fancy_Text_Print(TemplatedTextCache, *CompositeSurface, CompositeSurface->Get_Rect(), Point2D(text_rect.X, text_rect.Y), ColorSchemes[color], COLOR_TBLACK, style);
}


/**
 *  Adds a cell to the to-redraw list.
 *
 *  @author: ZivDero
 *
 *  @note: Do not use this function by itself! Call Tactical::Flag_Cell instead.
 */
void TacticalExtension::Flag_Cell(CellClass& cell)
{
    if (TacticalMap->CellRedrawCount < std::size(CellRedraw) - 1) { // -1 because... reasons. It's that way in vanilla.
        CellRedraw[TacticalMap->CellRedrawCount] = &cell;
        TacticalMap->CellRedrawCount++;
    }
}


/**
 *  Toggles beacon mode (analogous to Sell_Mode_Control, etc.)
 *
 *  @author: ZivDero
 */
void TacticalExtension::Beacon_Mode_Control(int control)
{
    if (!RuleExtension->IsBeaconsEnabled) {
        return;
    }

    bool mode = IsBeaconPlacementMode;
    switch (control) {
    case 0:
        mode = false;
        break;

    case -1:
        mode = (IsBeaconPlacementMode == false);
        break;

    case 1:
        mode = true;
        break;
    }

    if (mode != IsBeaconPlacementMode && !Map.PendingObject) {
        Map.IsSellMode = false;
        Map.IsPowerMode = false;
        Map.IsWaypointMode = false;
        Map.IsRepairMode = false;
        Map.Set_Default_Mouse(MOUSE_NORMAL, false);
        if (mode) {
            IsBeaconPlacementMode = true;
            Unselect_All();
        } else {
            IsBeaconPlacementMode = false;
            Map.Revert_Mouse_Shape();
        }
    }
}


/**
 *  Draws beacon text.
 *
 *  @author: ZivDero
 */
void TacticalExtension::Draw_Beacon_Text(std::string const& text, ColorScheme& scheme, Point2D const& drawpoint, Rect const& cliprect, bool centered, int offset)
{
    FontClass* font = Font6Ptr;

    /**
     *  Determine the text bounds.
     */
    Rect text_rect;
    font->String_Pixel_Bounds(text.c_str(), text_rect);
    text_rect += drawpoint;
    text_rect.Y += offset;

    if (centered) {
        text_rect.X -= text_rect.Width / 2;
    }

    /**
     *  Determine the size of the box encompassing the text.
     *  Center the box if necessary.
     */
    Rect box_rect = text_rect;
    box_rect.X -= 4;
    box_rect.Y -= 4;
    box_rect.Width += 8;
    box_rect.Height += 8;

    RGBClass rgb = scheme.HSV;
    int fore = DSurface::Build_Hicolor_Pixel(rgb.Get_Red(), rgb.Get_Green(), rgb.Get_Blue());

    Rect visible_box_rect = Intersect(box_rect, cliprect);
    CompositeSurface->Fill_Rect_Trans(visible_box_rect, RGBClass(0, 0, 0), 50);
    CompositeSurface->Draw_Rect(visible_box_rect, fore);

    Fancy_Text_Print(text.c_str(), *CompositeSurface, cliprect, text_rect.TopLeft - cliprect.TopLeft, &scheme, COLOR_TBLACK, TPF_6POINT | TPF_NOSHADOW);
}


/**
 *  Sets the currently displayed VOX subtitle. Pushed by AudioVoxClass::AI when a
 *  VOX with non-empty Text= begins playing.
 *
 *  @author: ZivDero
 */
void TacticalExtension::Set_Subtitle(const char* text, SubtitleCategoryType cat)
{
    if (text == nullptr || *text == '\0') {
        SubtitleText.clear();
        return;
    }
    SubtitleText = text;
    SubtitleCategoryCur = cat;
}


/**
 *  Clears the currently displayed VOX subtitle. Pushed by AudioVoxClass when a
 *  VOX stops playing or is interrupted.
 *
 *  @author: ZivDero
 */
void TacticalExtension::Clear_Subtitle()
{
    SubtitleText.clear();
}


/**
 *  Frees the cached HFONT used by the subtitle renderer. Safe to call when
 *  no font has been created yet.
 *
 *  @author: ZivDero
 */
void TacticalExtension::Invalidate_Subtitle_Font()
{
    if (SubtitleFont) {
        DeleteObject(SubtitleFont);
        SubtitleFont = nullptr;
    }
    SubtitleFontCacheName.clear();
    SubtitleFontCacheHeight = 0;
    SubtitleFontCacheWeight = 0;
}


/**
 *  Decides whether the current subtitle should be drawn given the player's
 *  SubtitleMode preference (sun.ini) and the category of the active VOX.
 *
 *  @author: ZivDero
 */
bool TacticalExtension::Should_Show_Subtitle() const
{
    if (SubtitleText.empty()) return false;

    switch (OptionsExtension->SubtitleMode) {
    case OptionsClassExtension::SUBTITLE_MODE_NONE:
        return false;
    case OptionsClassExtension::SUBTITLE_MODE_ALL:
        return true;
    case OptionsClassExtension::SUBTITLE_MODE_SCENARIO:
        return SubtitleCategoryCur == SUBTITLE_CATEGORY_SCENARIO;
    case OptionsClassExtension::SUBTITLE_MODE_SYSTEM:
        return SubtitleCategoryCur == SUBTITLE_CATEGORY_SYSTEM;
    }
    return true;
}


/**
 *  Returns a cached HFONT built from the current ui.ini subtitle font fields.
 *  Rebuilds lazily if any of those fields have changed since the last call.
 *
 *  @author: ZivDero
 */
static HFONT Get_Or_Build_Subtitle_Font(TacticalExtension& ext)
{
    HFONT existing = static_cast<HFONT>(ext.SubtitleFont);
    const bool stale = existing == nullptr || ext.SubtitleFontCacheHeight != UIControls->SubtitleFontHeight || ext.SubtitleFontCacheWeight != UIControls->SubtitleFontWeight || ext.SubtitleFontCacheName != UIControls->SubtitleFontName;

    if (stale) {
        ext.Invalidate_Subtitle_Font();
        HFONT font = CreateFont(UIControls->SubtitleFontHeight, 0, 0, 0, UIControls->SubtitleFontWeight, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_SWISS | DEFAULT_PITCH, UIControls->SubtitleFontName.c_str());
        ext.SubtitleFont = font;
        ext.SubtitleFontCacheName = UIControls->SubtitleFontName;
        ext.SubtitleFontCacheHeight = UIControls->SubtitleFontHeight;
        ext.SubtitleFontCacheWeight = UIControls->SubtitleFontWeight;
        return font;
    }
    return existing;
}


/**
 *  Renders the current VOX subtitle at the bottom of the tactical view using
 *  Win32 GDI. Word-wraps to the available width inside TacticalRect minus the
 *  configured horizontal margin, and draws an 8-direction outline around the
 *  fill so the text stays legible over varied terrain.
 *
 *  @author: ZivDero
 */
void TacticalExtension::Draw_Subtitle()
{
    if (!Should_Show_Subtitle()) {
        return;
    }
    if (Debug_Map) {
        return;
    }
    if (!CompositeSurface || !CompositeSurface->Is_Direct_Draw()) {
        return;
    }

    HDC hdc = static_cast<SDLSurface*>(CompositeSurface)->GetDC();
    if (hdc == nullptr) {
        return;
    }

    HFONT font = Get_Or_Build_Subtitle_Font(*this);
    if (font == nullptr) {
        static_cast<SDLSurface*>(CompositeSurface)->ReleaseDC(hdc);
        return;
    }

    HGDIOBJ old_obj = SelectObject(hdc, font);
    SetBkMode(hdc, TRANSPARENT);

    const int avail_w = TacticalRect.Width - 2 * UIControls->SubtitleMarginX;
    if (avail_w <= 0) {
        SelectObject(hdc, old_obj);
        static_cast<SDLSurface*>(CompositeSurface)->ReleaseDC(hdc);
        return;
    }

    RECT calc {0, 0, avail_w, 0};
    DrawText(hdc, SubtitleText.c_str(), -1, &calc, DT_CALCRECT | DT_WORDBREAK | DT_CENTER | DT_NOPREFIX);
    const int text_h = calc.bottom - calc.top;

    RECT dst;
    dst.left = TacticalRect.X + UIControls->SubtitleMarginX;
    dst.right = TacticalRect.X + TacticalRect.Width - UIControls->SubtitleMarginX;
    dst.bottom = TacticalRect.Y + TacticalRect.Height - UIControls->SubtitleMarginBottom;
    dst.top = dst.bottom - text_h;

    const UINT flags = DT_WORDBREAK | DT_CENTER | DT_NOPREFIX;

    const int OW = UIControls->SubtitleOutlineWidth;
    if (OW > 0) {
        SetTextColor(hdc, RGB(UIControls->SubtitleOutlineColor.R, UIControls->SubtitleOutlineColor.G, UIControls->SubtitleOutlineColor.B));
        for (int dy = -OW; dy <= OW; dy += OW) {
            for (int dx = -OW; dx <= OW; dx += OW) {
                if (dx == 0 && dy == 0) continue;
                RECT r = dst;
                OffsetRect(&r, dx, dy);
                DrawText(hdc, SubtitleText.c_str(), -1, &r, flags);
            }
        }
    }

    SetTextColor(hdc, RGB(UIControls->SubtitleTextColor.R, UIControls->SubtitleTextColor.G, UIControls->SubtitleTextColor.B));
    DrawText(hdc, SubtitleText.c_str(), -1, &dst, flags);

    SelectObject(hdc, old_obj);
    static_cast<SDLSurface*>(CompositeSurface)->ReleaseDC(hdc);
}
