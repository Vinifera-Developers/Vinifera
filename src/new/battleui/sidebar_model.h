/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SIDEBAR_MODEL.H
 *
 *  @author        ZivDero
 *
 *  @brief         Sidebar data model. Holds the buildable items grouped
 *                 into categories, with production state queries and sorting.
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

#include "tibsun_defines.h"
#include "vector.h"

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
    int Completion_Percent() const;
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
    void Recalc_All();

    bool Is_On_Sidebar(RTTIType type, int id) const;
    void Detach(AbstractClass* target);

    int Category_Count() const { return Categories.Count(); }
    BuildCategory& Get_Category(int index) { return Categories[index]; }
    const BuildCategory& Get_Category(int index) const { return Categories[index]; }

    bool IsDirty;
    bool IsActive;

    int Route_To_Category(RTTIType type, int id) const;

    DynamicVectorClass<BuildCategory> Categories;
};
