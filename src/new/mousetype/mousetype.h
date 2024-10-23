/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          MOUSETYPE.H
 *
 *  @author        CCHyper
 *
 *  @brief         Mouse cursor controls and overrides.
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

#include "always.h"
#include "iomap.h"
#include "point.h"
#include "wstring.h"


class CCINIClass;
class NoInitClass;


/**
 *  This type is used to control the frames and rates of the mouse
 *  pointer. Some mouse pointers are actually looping animations.
 */
class MouseTypeClass
{
    friend class MouseClassExt;

    public:
        MouseTypeClass(const char *name);
        MouseTypeClass(int start_frame, int frame_count, int frame_rate, int small_frame, int small_frame_count, int small_frame_rate, Point2D hotspot, Point2D small_hotspot);
        MouseTypeClass(const NoInitClass &noinit);
        virtual ~MouseTypeClass();

        Wstring Get_Name() const { return Name; }

        static void One_Time();

        static bool Read_INI(CCINIClass &ini);
#ifndef NDEBUG
        static bool Write_Default_INI(CCINIClass &ini);
#endif

        static const MouseTypeClass *As_Pointer(MouseType type);
        static const MouseTypeClass *As_Pointer(const char *name);
        static const MouseTypeClass &As_Reference(MouseType type);
        static const MouseTypeClass &As_Reference(const char *name);
        static MouseType From_Name(const char *name);
        static const char *Name_From(MouseType type);

    private:
        static MouseTypeClass *Find_Or_Make(const char *name);

    private:
        Wstring Name;

        /**
         *  Starting frame number.
         */
        int StartFrame;

        /**
         *  Number of animation frames.
         */
        int FrameCount;

        /**
         *  Frame delay between changing frames.
         */
        int FrameRate;

        /**
         *  Start frame number for small version (if any).
         */
        int SmallFrame;

        /**
         *  Number of animation frames for small version (if any).
         */
        int SmallFrameCount;

        /**
         *  Frame delay between changing frames for small version (if any).
         */
        int SmallFrameRate;

        /**
         *  Hotspot X and Y offset.
         */
        Point2D Hotspot;

        /**
         *  Hotspot X and Y offset for the small version (if any).
         */
        Point2D SmallHotspot;

    private:
        static MouseTypeClass MouseControl[MOUSE_COUNT];
        static const char *MouseTypeClass::MouseNames[MOUSE_COUNT];

    public:
        static MouseType CanMoveMouse;
        static MouseType NoMoveMouse;
        static MouseType CanAttackMouse;
        static MouseType StayAttackMouse;
};
