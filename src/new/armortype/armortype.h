/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ARMORTYPE.H
 *
 *  @authors       CCHyper, ZivDero
 *
 *  @brief         New ArmorType class.
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
#include "tibsun_defines.h"
#include "vinifera_defines.h"
#include "wstring.h"
#include "objidl.h"
#include "noinit.h"

class CCINIClass;


class DECLSPEC_UUID(UUID_ARMORTYPE)
ArmorTypeClass final : IPersistStream
{
public:
    /**
     *  IUnknown
     */
    IFACEMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);
    IFACEMETHOD_(ULONG, AddRef)();
    IFACEMETHOD_(ULONG, Release)();

    /**
     *  IPersist
     */
    IFACEMETHOD(GetClassID)(CLSID* pClassID);

    /**
     *  IPersistStream
     */
    IFACEMETHOD(IsDirty)();
    IFACEMETHOD(Load)(IStream* pStm);
    IFACEMETHOD(Save)(IStream* pStm, BOOL fClearDirty);
    IFACEMETHOD_(LONG, GetSizeMax)(ULARGE_INTEGER* pcbSize);

    ArmorTypeClass();
    ArmorTypeClass(const NoInitClass& noinit) {}
    ArmorTypeClass(const char *name);
    virtual ~ArmorTypeClass();

    char const* Name() const { return IniName; }
    bool Read_INI(CCINIClass& ini);

    static bool One_Time();

    static ArmorType From_Name(const char *name);
    static const char *Name_From(ArmorType type);

    static const ArmorTypeClass *Find_Or_Make(const char *name);

private:
    /**
     *  The name of this armor type, used for identification purposes.
     */
    char IniName[256];

public:
    /**
     *  The warhead damage is reduced depending on the the type of armor the
     *  defender has. This is the default value for this armor.
     */
    double Modifier;

    /**
     *  The warhead may be forbidden from targeting the defender depending the
     *  type of armor it has. This is the default value for this armor.
     */
    bool ForceFire;
    bool PassiveAcquire;
    bool Retaliate;

    /**
     *  The armor type that this armor is based on. Inherits its default values from this, if set.
     */
    ArmorType BaseArmor;
};
