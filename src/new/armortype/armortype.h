/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  New ArmorType class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once 

#include "tibsun_defines.h"
#include "vinifera_defines.h"
#include "objidl.h"

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
