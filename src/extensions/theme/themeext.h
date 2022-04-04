/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          THEMEEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended ThemeClass class.
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

#include "extension.h"
#include "container.h"
#include "theme.h"


class CCINIClass;


class ThemeControlExtension final : public Extension<ThemeClass::ThemeControl>
{
    public:
        ThemeControlExtension(ThemeClass::ThemeControl *this_ptr);
        ThemeControlExtension(const NoInitClass &noinit);
        ~ThemeControlExtension();

        virtual int Size_Of() const override;

        bool Read_INI(CCINIClass &ini);

    public:
        /**
         *  The addon required to be active for this theme to be available.
         */
        AddonType RequiredAddon;
};


extern ExtensionMap<ThemeClass::ThemeControl, ThemeControlExtension> ThemeControlExtensions;
