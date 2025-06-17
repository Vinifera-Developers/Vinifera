/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          PREREQUISITEGROUP.H
 *
 *  @authors       ZivDero
 *
 *  @brief         New Prerequisite Group class.
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
#include "typelist.h"

class BuildingTypeClass;
class CCINIClass;


class DECLSPEC_UUID(UUID_ARMORTYPE)
PrerequisiteGroupClass final : IPersistStream
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

    PrerequisiteGroupClass();
    PrerequisiteGroupClass(const NoInitClass& noinit) {}
    PrerequisiteGroupClass(const char *name);
    virtual ~PrerequisiteGroupClass();

    char const* Name() const { return IniName; }
    bool Read_INI(CCINIClass& ini);
    static bool Read_Global_INI(CCINIClass& ini);

    static int Encode(PrerequisiteGroupType type)
    {
        if (type < PREREQ_GROUP_FIRST) {
            return STRUCT_FIRST;
        }
        return -(static_cast<int>(type) + 1);
    }

    static PrerequisiteGroupType Decode(int number)
    {
        if (number >= STRUCT_FIRST) {
            return PREREQ_GROUP_NONE;
        }
        return static_cast<PrerequisiteGroupType>(-number - 1);
    }

    static bool One_Time();

    static PrerequisiteGroupType From_Name(const char *name);
    static const char *Name_From(PrerequisiteGroupType type);

    static const PrerequisiteGroupClass *Find_Or_Make(const char *name);

private:
    void Parse_String(char* string);

private:
    /**
     *  The name of this prerequisite group, used for identification purposes.
     */
    char IniName[256];

public:
    /**
     *  The list of buildings that satify this prerequisite.
     */
    TypeList<int> Prerequisites;
};
