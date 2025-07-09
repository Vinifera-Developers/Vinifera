/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          HOUSEEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended HouseClass class.
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

#include "abstractext.h"
#include "house.h"
#include "housetype.h"


class DECLSPEC_UUID(UUID_HOUSE_EXTENSION)
HouseClassExtension final : public AbstractClassExtension
{
public:
    /**
     *  IPersist
     */
    IFACEMETHOD(GetClassID)(CLSID *pClassID);

    /**
     *  IPersistStream
     */
    IFACEMETHOD(Load)(IStream *pStm);
    IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

public:
    HouseClassExtension(const HouseClass *this_ptr = nullptr);
    HouseClassExtension(const NoInitClass &noinit);
    virtual ~HouseClassExtension();

    virtual int Get_Object_Size() const override;
    virtual void Detach(AbstractClass * target, bool all = true) override;
    virtual void Object_CRC(CRCEngine &crc) const override;

    virtual const char *Name() const override { return reinterpret_cast<const HouseClass *>(This())->Class->Name(); }
    virtual const char *Full_Name() const override { return reinterpret_cast<const HouseClass *>(This())->Class->Full_Name(); }
    
    virtual HouseClass *This() const override { return reinterpret_cast<HouseClass *>(AbstractClassExtension::This()); }
    virtual const HouseClass *This_Const() const override { return reinterpret_cast<const HouseClass *>(AbstractClassExtension::This_Const()); }
    virtual RTTIType Fetch_RTTI() const override { return RTTI_HOUSE; }

    FactoryClass* Fetch_Factory(RTTIType rtti, ProductionFlags flags) const;
    void Set_Factory(RTTIType rtti, FactoryClass* factory, ProductionFlags flags);
    int* Factory_Counter(RTTIType rtti, ProductionFlags flags);
    int Factory_Count(RTTIType rtti, ProductionFlags flags) const;
    ProdFailType Suspend_Production(RTTIType type, ProductionFlags flags);
    ProdFailType Begin_Production(RTTIType type, int id, bool resume, ProductionFlags flags);
    ProdFailType Abandon_Production(RTTIType type, int id, ProductionFlags flags);
    bool Place_Object(RTTIType type, Cell const& cell, ProductionFlags flags);
    void Update_Factories(RTTIType rtti, ProductionFlags flags);
    TechnoTypeClass const* Suggest_New_Object(RTTIType objecttype, ProductionFlags flags) const;

    int AI_Unit();
    int AI_Naval_Unit();

    bool Has_Prerequisite(int prerequisite);
    bool Has_Prerequisite(PrerequisiteGroupType group);
    bool Has_Prerequisite(StructType building);

    void Put_Storage_Pointers();
    static void Load_Unit_Trackers(HouseClass* house, IStream* pStm);
    static void Save_Unit_Trackers(HouseClass* house, IStream* pStm);

public:
    /**
     *  Replacement Tiberium storage.
     */
    VectorClass<int> TiberiumStorage;

    /**
     *  Replacement Weed storage.
     */
    VectorClass<int> WeedStorage;

    /**
     *  Record the number of naval factories active.
     */
    int NavalFactories;

    /**
     *  For human controlled houses, only one type of naval unit can be produced
     *  at any one instant. This is the factory object controlling this production.
     */
    FactoryClass* NavalFactory;

    /**
     *  The type of the naval unit the AI is currently scheduled to build.
     */
    UnitType BuildNavalUnit;
};
