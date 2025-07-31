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

#include "always.h"
#include "extension.h"
#include "theme.h"


class CCINIClass;


class ThemeControlExtension final : public GlobalExtensionClass<ThemeClass::ThemeControl>
{
    public:
        IFACEMETHOD(Load)(IStream *pStm);
        IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

    public:
        ThemeControlExtension(const ThemeClass::ThemeControl *this_ptr);
        ThemeControlExtension(const NoInitClass &noinit);
        virtual ~ThemeControlExtension();

        /**
         *  ThemeControl extension does not require these to be used, but we
         *  implement them for completeness.
         */
        virtual int Get_Object_Size() const override;
        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual const char *Name() const override { return "ThemeControl"; }
        virtual const char *Full_Name() const override { return "ThemeControl"; }

        bool Read_INI(CCINIClass &ini);

    public:
        /**
         *  The addon required to be active for this theme to be available.
         */
        AddonType RequiredAddon;
};
