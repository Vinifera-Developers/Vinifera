/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SIDEBAR_MODEL.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Sidebar data model implementation.
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

#include "sidebar_model.h"

#include "abstract.h"
#include "buildingtype.h"
#include "extension.h"
#include "factory.h"
#include "house.h"
#include "housetype.h"
#include "optionsext.h"
#include "sidebar.h"
#include "supertype.h"
#include "technotypeext.h"
#include "tibsun_globals.h"
#include "vinifera_defines.h"

#include <algorithm>


/***************************************************************************
**  BuildItem
***************************************************************************/


/**
 *  Is the item currently being produced?
 *
 *  @author: ZivDero
 */
bool BuildItem::Is_In_Production() const
{
    return Factory != nullptr && Factory->Is_Building();
}


/**
 *  Returns the production completion percentage (0-100).
 *
 *  @author: ZivDero
 */
int BuildItem::Completion_Percent() const
{
    if (Factory == nullptr) {
        return 0;
    }

    return Factory->Completion();
}


/**
 *  Has production of this item completed?
 *
 *  @author: ZivDero
 */
bool BuildItem::Is_Completed() const
{
    return Factory != nullptr && Factory->Has_Completed();
}


/**
 *  Is production of this item suspended?
 *
 *  @author: ZivDero
 */
bool BuildItem::Is_On_Hold() const
{
    return Factory != nullptr && Factory->IsSuspended;
}


/**
 *  Returns the number of items queued in the factory.
 *
 *  @author: ZivDero
 */
int BuildItem::Queue_Count() const
{
    if (Factory == nullptr) {
        return 0;
    }

    const TechnoTypeClass* techno = Fetch_Techno_Type(Type, ID);
    if (techno == nullptr) {
        return 0;
    }

    return Factory->Total_Queued(*techno);
}


/***************************************************************************
**  BuildCategory
***************************************************************************/


/**
 *  Class constructor.
 *
 *  @author: ZivDero
 */
BuildCategory::BuildCategory() :
    IsDirty(true)
{
}


/**
 *  Adds an item to this category if not already present.
 *
 *  @author: ZivDero
 */
bool BuildCategory::Add(RTTIType type, int id)
{
    if (Is_On_Sidebar(type, id)) {
        return false;
    }

    BuildItem item;
    item.Type = type;
    item.ID = id;
    item.Factory = nullptr;

    Items.Add(item);
    IsDirty = true;
    return true;
}


/**
 *  Removes an item from this category.
 *
 *  @author: ZivDero
 */
bool BuildCategory::Remove(RTTIType type, int id)
{
    int index = Find_Index(type, id);
    if (index < 0) {
        return false;
    }

    Items.Delete(index);
    IsDirty = true;
    return true;
}


/**
 *  Links a factory to a specific item in this category.
 *
 *  @author: ZivDero
 */
void BuildCategory::Link_Factory(FactoryClass* factory, RTTIType type, int id)
{
    int index = Find_Index(type, id);
    if (index >= 0) {
        Items[index].Factory = factory;
    }
}


/**
 *  Removes all items from this category.
 *
 *  @author: ZivDero
 */
void BuildCategory::Clear()
{
    Items.Clear();
    IsDirty = true;
}


/**
 *  Comparison function for sorting buildable items.
 *
 *  Sorting order:
 *    - Super weapons first (ordered by recharge time, then ID)
 *    - Within same RTTI type:
 *      - Buildings: non-defenses, then walls, then gates, then defenses
 *      - Units: non-naval before naval
 *      - Player side items before foreign side items
 *    - Different RTTI types: Specials > Infantry > Aircraft > Units
 *    - Final tiebreaker: by ID
 *
 *  @author: ZivDero
 */
static int __cdecl Build_Item_Compare(const void* p1, const void* p2)
{
    const auto* bt1 = static_cast<const BuildItem*>(p1);
    const auto* bt2 = static_cast<const BuildItem*>(p2);

    auto first_side = [](unsigned owners) -> int {
        int side = INT_MAX;
        for (int i = 0; i < HouseTypes.Count(); i++) {
            if (owners & 1 << i) {
                side = std::min<int>(HouseTypes[i]->Side, side);
            }
        }
        return side != INT_MAX ? side : SIDE_NONE;
    };

    auto is_side_owner = [](const HouseClass* house, unsigned owners) -> int {
        if (owners & 1 << house->ActLike) {
            return true;
        }

        const SideType side = house->Class->Side;
        for (int i = 0; i < HouseTypes.Count(); i++) {
            if (owners & 1 << i && HouseTypes[i]->Side == side) {
                return true;
            }
        }
        return false;
    };

    if (bt1->Type == bt2->Type) {

        /**
         *  If both are SWs, the one that recharges quicker goes first,
         *  otherwise sort by ID.
         */
        if (bt1->Type == RTTI_SPECIAL || bt1->Type == RTTI_SUPERWEAPONTYPE) {
            if (SuperWeaponTypes[bt1->ID]->RechargeTime != SuperWeaponTypes[bt2->ID]->RechargeTime) {
                return SuperWeaponTypes[bt1->ID]->RechargeTime - SuperWeaponTypes[bt2->ID]->RechargeTime;
            }

            return bt1->ID - bt2->ID;
        }

        const TechnoTypeClass* t1 = Fetch_Techno_Type(bt1->Type, bt1->ID);
        const TechnoTypeClass* t2 = Fetch_Techno_Type(bt2->Type, bt2->ID);

        /**
         *  If both are Buildings, non-defenses come first, then walls, then gates, then base defenses.
         */
        if (bt1->Type == RTTI_BUILDINGTYPE && OptionsExtension->SortDefensesAsLast) {
            const auto* b1 = static_cast<const BuildingTypeClass*>(t1);
            const auto* b2 = static_cast<const BuildingTypeClass*>(t2);

            const auto* ext1 = Extension::Fetch(t1);
            const auto* ext2 = Extension::Fetch(t2);

            enum {
                BCAT_NORMAL,
                BCAT_WALL,
                BCAT_GATE,
                BCAT_DEFENSE
            };

            int cat1 = b1->IsWall || b1->IsFirestormWall || b1->IsLaserFencePost || b1->IsLaserFence ? BCAT_WALL : b1->IsGate ? BCAT_GATE : ext1->IsSortCameoAsBaseDefense ? BCAT_DEFENSE : BCAT_NORMAL;
            int cat2 = b2->IsWall || b2->IsFirestormWall || b2->IsLaserFencePost || b2->IsLaserFence ? BCAT_WALL : b2->IsGate ? BCAT_GATE : ext2->IsSortCameoAsBaseDefense ? BCAT_DEFENSE : BCAT_NORMAL;

            if (cat1 != cat2) {
                return cat1 - cat2;
            }
        }

        /**
         *  If both are Units, non-naval units come first.
         */
        if (bt1->Type == RTTI_UNITTYPE) {
            const auto* ext1 = Extension::Fetch(t1);
            const auto* ext2 = Extension::Fetch(t2);

            if (ext1->IsNaval != ext2->IsNaval) {
                return static_cast<int>(ext1->IsNaval) - static_cast<int>(ext2->IsNaval);
            }
        }

        /**
         *  If your side owns one of the objects, but not another, yours comes first.
         */
        const int owns1 = is_side_owner(PlayerPtr, t1->Get_Ownable());
        const int owns2 = is_side_owner(PlayerPtr, t2->Get_Ownable());

        if (owns1 != owns2) {
            return owns2 - owns1;
        }

        /**
         *  If you don't own either of the objects, then sort by side index.
         */
        if (!owns1 && !owns2) {
            const int side1 = first_side(t1->Get_Ownable());
            const int side2 = first_side(t2->Get_Ownable());

            if (side1 != side2) {
                return side1 - side2;
            }
        }

        return bt1->ID - bt2->ID;
    }

    if (bt1->Type == RTTI_SPECIAL || bt1->Type == RTTI_SUPERWEAPONTYPE) {
        return -1;
    }

    if (bt2->Type == RTTI_SPECIAL || bt2->Type == RTTI_SUPERWEAPONTYPE) {
        return 1;
    }

    if (bt1->Type == RTTI_INFANTRYTYPE) {
        return -1;
    }

    if (bt2->Type == RTTI_INFANTRYTYPE) {
        return 1;
    }

    if (bt1->Type == RTTI_AIRCRAFTTYPE) {
        return -1;
    }

    if (bt2->Type == RTTI_AIRCRAFTTYPE) {
        return 1;
    }

    if (bt1->Type == RTTI_UNITTYPE) {
        return -1;
    }

    if (bt2->Type == RTTI_UNITTYPE) {
        return 1;
    }

    return bt1->ID - bt2->ID;
}


/**
 *  Re-sorts the items in this category.
 *
 *  @author: ZivDero
 */
void BuildCategory::Recalc()
{
    if (Items.Count() > 1) {
        std::qsort(Items.begin(), Items.Count(), sizeof(BuildItem), Build_Item_Compare);
    }

    IsDirty = false;
}


/**
 *  Checks if an item is present in this category.
 *
 *  @author: ZivDero
 */
bool BuildCategory::Is_On_Sidebar(RTTIType type, int id) const
{
    return Find_Index(type, id) >= 0;
}


/**
 *  Returns the index of an item, or -1 if not found.
 *
 *  @author: ZivDero
 */
int BuildCategory::Find_Index(RTTIType type, int id) const
{
    for (int i = 0; i < Items.Count(); i++) {
        if (Items[i].Type == type && Items[i].ID == id) {
            return i;
        }
    }
    return -1;
}


/***************************************************************************
**  SidebarModel
***************************************************************************/


/**
 *  Class constructor.
 *
 *  @author: ZivDero
 */
SidebarModel::SidebarModel() :
    IsDirty(true),
    IsActive(false)
{
}


/**
 *  Initializes the model with the given number of build categories.
 *
 *  @author: ZivDero
 */
void SidebarModel::Init_Categories(int count)
{
    Categories.Resize(count);
    for (int i = 0; i < count; i++) {
        Categories.Add(BuildCategory());
    }
}


/**
 *  Clears all categories and resets state.
 *
 *  @author: ZivDero
 */
void SidebarModel::Init_Clear()
{
    for (auto& category : Categories) {
        category.Clear();
    }

    IsDirty = true;
    IsActive = false;
}


/**
 *  Initializes the model for the current player house.
 *  Sets up the configured number of categories.
 *
 *  @author: ZivDero
 */
void SidebarModel::Init_For_House()
{
    Init_Clear();
    IsActive = true;
}


/**
 *  Routes an RTTI type to its category index. Uses 4-category
 *  layout (Structure / Infantry / Unit / Special), matching the
 *  existing Vinifera tab routing.
 *
 *  @author: ZivDero
 */
int SidebarModel::Which_Category(RTTIType type, int id) const
{
    if (Categories.Count() < 1) {
        return -1;
    }

    /**
     *  For 2-category (classic) layout, use vanilla mapping:
     *  column 0 = structures, column 1 = everything else.
     */
    if (Categories.Count() == 2) {
        switch (type) {
        case RTTI_BUILDINGTYPE:
        case RTTI_BUILDING:
            return 0;
        default:
            return 1;
        }
    }

    /**
     *  For 4-category (tabbed) layout, route by type and production flags.
     */
    ProductionFlags flags = TechnoTypeClassExtension::Get_Production_Flags(type, id);

    switch (type) {
    case RTTI_BUILDINGTYPE:
    case RTTI_BUILDING:
        return 0;

    case RTTI_INFANTRYTYPE:
    case RTTI_INFANTRY:
        return 1;

    case RTTI_UNITTYPE:
    case RTTI_UNIT:
        if (flags & PRODFLAG_NAVAL) {
            return 3;
        } else {
            return 2;
        }

    case RTTI_AIRCRAFTTYPE:
    case RTTI_AIRCRAFT:
    case RTTI_SUPERWEAPONTYPE:
    case RTTI_SUPERWEAPON:
    case RTTI_SPECIAL:
    default:
        return std::min(3, Categories.Count() - 1);
    }
}


/**
 *  Adds a buildable item to the appropriate category.
 *
 *  @author: ZivDero
 */
bool SidebarModel::Add(RTTIType type, int id)
{
    int index = Which_Category(type, id);
    if (index < 0) {
        return false;
    }

    if (Categories[index].Add(type, id)) {
        IsDirty = true;
        return true;
    }
    return false;
}


/**
 *  Removes a buildable item from the appropriate category.
 *
 *  @author: ZivDero
 */
bool SidebarModel::Remove(RTTIType type, int id)
{
    int index = Which_Category(type, id);
    if (index < 0) {
        return false;
    }

    if (Categories[index].Remove(type, id)) {
        IsDirty = true;
        return true;
    }
    return false;
}


/**
 *  Links a factory to a specific buildable item.
 *
 *  @author: ZivDero
 */
void SidebarModel::Link_Factory(FactoryClass* factory, RTTIType type, int id)
{
    int index = Which_Category(type, id);
    if (index >= 0) {
        Categories[index].Link_Factory(factory, type, id);
    }
}


/**
 *  Abandons production of an item by unlinking its factory.
 *
 *  @author: ZivDero
 */
void SidebarModel::Abandon_Production(RTTIType type, int id)
{
    int index = Which_Category(type, id);
    if (index >= 0) {
        Categories[index].Link_Factory(nullptr, type, id);
    }
}


/**
 *  Re-sorts all dirty categories.
 *
 *  @author: ZivDero
 */
void SidebarModel::Recalc_All()
{
    for (auto& category : Categories) {
        if (category.IsDirty) {
            category.Recalc();
        }
    }
    IsDirty = false;
}


/**
 *  Checks if an item is present anywhere on the sidebar.
 *
 *  @author: ZivDero
 */
bool SidebarModel::Is_On_Sidebar(RTTIType type, int id) const
{
    for (const auto& category : Categories) {
        if (category.Is_On_Sidebar(type, id)) {
            return true;
        }
    }
    return false;
}


/**
 *  Nullifies factory pointers for any BuildItem whose Factory matches target.
 *
 *  @author: ZivDero
 */
void SidebarModel::Detach(AbstractClass* target)
{
    for (auto& category : Categories) {
        for (auto& item : category.Items) {
            if (item.Factory == target) {
                item.Factory = nullptr;
            }
        }
    }
}


/**
 *  Saves the sidebar model state to the given stream.
 *  Writes category count, then for each category: item count,
 *  then each item's Type and ID. Factory pointers are not saved.
 *
 *  @author: ZivDero
 */
HRESULT SidebarModel::Save(IStream* pStm) const
{
    int cat_count = Categories.Count();
    HRESULT hr = pStm->Write(&cat_count, sizeof(cat_count), nullptr);
    if (FAILED(hr)) return hr;

    for (int c = 0; c < cat_count; c++) {
        const BuildCategory& cat = Categories[c];
        int item_count = cat.Items.Count();
        hr = pStm->Write(&item_count, sizeof(item_count), nullptr);
        if (FAILED(hr)) return hr;

        for (int i = 0; i < item_count; i++) {
            int type = static_cast<int>(cat.Items[i].Type);
            int id = cat.Items[i].ID;
            hr = pStm->Write(&type, sizeof(type), nullptr);
            if (FAILED(hr)) return hr;
            hr = pStm->Write(&id, sizeof(id), nullptr);
            if (FAILED(hr)) return hr;
        }
    }

    return S_OK;
}


/**
 *  Loads the sidebar model state from the given stream.
 *  Reads category count and items, populating the existing categories.
 *  Factory pointers are not loaded — they must be relinked post-load.
 *
 *  @author: ZivDero
 */
HRESULT SidebarModel::Load(IStream* pStm)
{
    int cat_count = 0;
    HRESULT hr = pStm->Read(&cat_count, sizeof(cat_count), nullptr);
    if (FAILED(hr)) return hr;

    for (int c = 0; c < cat_count && c < Categories.Count(); c++) {
        Categories[c].Clear();

        int item_count = 0;
        hr = pStm->Read(&item_count, sizeof(item_count), nullptr);
        if (FAILED(hr)) return hr;

        for (int i = 0; i < item_count; i++) {
            int type = 0;
            int id = 0;
            hr = pStm->Read(&type, sizeof(type), nullptr);
            if (FAILED(hr)) return hr;
            hr = pStm->Read(&id, sizeof(id), nullptr);
            if (FAILED(hr)) return hr;

            Categories[c].Add(static_cast<RTTIType>(type), id);
        }
    }

    /**
     *  If the stream had more categories than we have, skip the extra data.
     */
    for (int c = Categories.Count(); c < cat_count; c++) {
        int item_count = 0;
        hr = pStm->Read(&item_count, sizeof(item_count), nullptr);
        if (FAILED(hr)) return hr;

        for (int i = 0; i < item_count; i++) {
            int dummy[2];
            hr = pStm->Read(dummy, sizeof(dummy), nullptr);
            if (FAILED(hr)) return hr;
        }
    }

    IsDirty = true;
    return S_OK;
}
