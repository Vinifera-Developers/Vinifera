/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Mouse cursor controls and overrides.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "iomap.h"
#include "point.h"


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
        MouseTypeClass(const char* name);
        MouseTypeClass(const char* name, int start_frame, int frame_count, int frame_rate, int small_frame, int small_frame_count, int small_frame_rate, Point2D hotspot, Point2D small_hotspot);
        MouseTypeClass(const NoInitClass &noinit);
        virtual ~MouseTypeClass();

        static void One_Time();

        static bool Read_INI(CCINIClass &ini);
#ifndef NDEBUG
        static bool Write_Default_INI(CCINIClass &ini);
#endif

        static MouseType From_Name(const char *name);
        static const char *Name_From(MouseType type);

    private:
        static MouseTypeClass *Find_Or_Make(const char *name);

    private:
        std::string Name;

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
};
