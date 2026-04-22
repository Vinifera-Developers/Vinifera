/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  New Prerequisite Group class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "objidl.h"
#include "tibsun_defines.h"
#include "typelist.h"
#include "vinifera_defines.h"

class BuildingTypeClass;
class CCINIClass;


class DECLSPEC_UUID(UUID_PREREQUISITE_GROUP)
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

    /**
     *  Converts the prerequisite group type to an integer as it appears in the Prerequisite= list.
     */
    static int Encode(PrerequisiteGroupType type)
    {
        if (type < PREREQ_GROUP_FIRST) {
            return STRUCT_FIRST;
        }
        return -(static_cast<int>(type) + 1);
    }

    /**
     *  Converts the integer as it appears in the Prerequisite= list to a prerequisite group type.
     */
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
