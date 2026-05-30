/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Sidebar data model. Holds the buildable items grouped
 *          into categories, with production state queries and sorting.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "tibsun_defines.h"
#include "vector.h"

#include <objidl.h>

class AbstractClass;
class FactoryClass;


/**
 *  A single buildable entry on the sidebar.
 */
struct BuildItem {
    RTTIType Type;
    int ID;
    FactoryClass* Factory;

    bool Is_In_Production() const;
    int Completion_Stage() const;
    bool Is_Completed() const;
    bool Is_On_Hold() const;
    int Queue_Count() const;

    bool operator==(const BuildItem& rhs) const { return Type == rhs.Type && ID == rhs.ID; }
    bool operator!=(const BuildItem& rhs) const { return !(*this == rhs); }
};


/**
 *  A group of buildable items displayed in one strip/tab.
 */
class BuildCategory
{
public:
    BuildCategory();

    bool Add(RTTIType type, int id);
    bool Remove(RTTIType type, int id);
    void Link_Factory(FactoryClass* factory, RTTIType type, int id);
    void Clear();
    void Recalc();

    bool Is_On_Sidebar(RTTIType type, int id) const;
    int Find_Index(RTTIType type, int id) const;

    bool operator==(const BuildCategory& rhs) const { return this == &rhs; }
    bool operator!=(const BuildCategory& rhs) const { return !(*this == rhs); }

    DynamicVectorClass<BuildItem> Items;
    bool IsDirty;
};


/**
 *  Top-level sidebar data model. Routes buildable items into categories,
 *  manages production links, and triggers re-sorting.
 */
class SidebarModel
{
public:
    SidebarModel();

    void Init_Clear();
    void Init_For_House();

    bool Add(RTTIType type, int id);
    bool Remove(RTTIType type, int id);
    void Link_Factory(FactoryClass* factory, RTTIType type, int id);
    void Abandon_Production(RTTIType type, int id);
    void Recalc_Dirty();
    void Recalc_All();

    bool Is_On_Sidebar(RTTIType type, int id) const;
    void Detach(AbstractClass* target);

    HRESULT Save(IStream* pStm) const;
    HRESULT Load(IStream* pStm);

    int Category_Count() const { return Categories.Count(); }
    BuildCategory& Get_Category(int index) { return Categories[index]; }
    const BuildCategory& Get_Category(int index) const { return Categories[index]; }

    void Init_Categories(int count);
    bool Needs_Recalc() const { return IsDirty; }
    void Flag_Dirty() { IsDirty = true; }
    bool Is_Active() const { return IsActive; }

    int Which_Category(RTTIType type, int id) const;

private:
    bool IsDirty;
    bool IsActive;

    DynamicVectorClass<BuildCategory> Categories;
};
