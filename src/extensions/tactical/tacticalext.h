/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TACTICALEXT.H
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
#pragma once

#include "abstractext.h"
#include "extension.h"
#include "tactical.h"
#include "ttimer.h"
#include "stimer.h"
#include "wstring.h"
#include "point.h"
#include "textprint.h"
#include <objidl.h>


class HouseClass;
class CRCEngine;


enum InfoTextPosType {
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
};


class TacticalExtension final : public GlobalExtensionClass<Tactical>
{
public:
    IFACEMETHOD(Load)(IStream* pStm);
    IFACEMETHOD(Save)(IStream* pStm, BOOL fClearDirty);

public:
    TacticalExtension(const Tactical* this_ptr = nullptr);
    TacticalExtension(const NoInitClass& noinit);
    virtual ~TacticalExtension();

    virtual int Get_Object_Size() const override;
    virtual void Detach(AbstractClass* target, bool all = true) override;
    virtual void Object_CRC(CRCEngine& crc) const override;

    virtual const char* Name() const override { return "TacticalMap"; }
    virtual const char* Full_Name() const override { return "TacticalMap"; }

    void Set_Info_Text(const char* text);
    void Enable_Templated_Text(int label, ColorSchemeType color);
    void Disable_Templated_Text();
    void Clear_Templated_Text_Cache() { IsTemplatedTextCached = false; }

    void Draw_Version_Number_Text();

    void Draw_Debug_Overlay();
    void Draw_FrameStep_Overlay();

    void Draw_Information_Text();
    void Draw_Super_Timers();
    void Draw_Templated_Text();

    void Render_Post();
    void Flag_Cell(CellClass& cell);

    void Beacon_Mode_Control(int control);
    void Draw_Beacon_Text(std::string const& text, ColorScheme const& scheme, Point2D const& drawpoint, Rect const& cliprect, bool centered, int offset);

#ifndef NDEBUG
    bool Debug_Draw_Facings();
#endif

private:
    void Super_Draw_Timer(int row_index, ColorScheme* color, int time, const char* name, unsigned long* flash_time, bool* flash_state);

public:
    /**
     *  Has information text been set?
     */
    bool IsInfoTextSet;

    /**
     *  The information text to print on the screen.
     */
    char InfoTextBuffer[512];

    /**
     *  Where on the screen shall the text be printed?
     */
    InfoTextPosType InfoTextPosition;

    /**
     *  Sound to play when this text is initially drawn.
     */
    VocType InfoTextNotifySound;

    /**
     *  Volume at which to play the initial sound.
     */
    float InfoTextNotifySoundVolume;

    /**
     *  The font style of the print text.
     */
    TextPrintType InfoTextStyle;

    /**
     *  The lifetime timer for the information text.
     */
    CDTimerClass<MSTimerClass> InfoTextTimer;

    /**
     *  Replacement cell redraw list, as the vanilla one is too small for modern screen sizes.
     */
    CellClass* CellRedraw[128 * 128]; // Not a solid number, just enough to never cause problems.

    /**
     *  The number of cells in the array above, only used after loading the game!
     */
    int CellRedrawCount;

    /**
     *  Is the templated text currently shown?
     */
    bool IsTemplatedTextVisible;

    /**
     *  Index of the tutorial text to show as the templated text.
     */
    int TemplatedTextIndex;

    /**
     *  Where on the screen shall the templated text be printed?
     */
    InfoTextPosType TemplatedTextPosition;

    /**
     *  Which color scheme should the templated text use?
     */
    ColorSchemeType TemplatedTextColor;

    /**
     *  The font style of the templated text.
     */
    TextPrintType TemplatedTextStyle;

    /**
     *  Is there a cached string containing the formatted templated text?
     */
    bool IsTemplatedTextCached;

    /**
     *  The cached string containing the formatted templated text.
     */
    char TemplatedTextCache[512];

    /**
     *  Is the player currently placing a beacon?
     */
    bool IsBeaconPlacementMode;

    /**
     *  Is the player currently editing a beacon's text?
     */
    bool IsEditingBeaconText;
};
