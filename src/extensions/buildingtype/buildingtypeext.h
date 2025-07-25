/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BUILDINGTYPEEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended BuildingTypeClass class.
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

#include "technotypeext.h"
#include "buildingtype.h"


class DECLSPEC_UUID(UUID_BUILDINGTYPE_EXTENSION)
BuildingTypeClassExtension final : public TechnoTypeClassExtension
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
    BuildingTypeClassExtension(const BuildingTypeClass *this_ptr = nullptr);
    BuildingTypeClassExtension(const NoInitClass &noinit);
    virtual ~BuildingTypeClassExtension();

    virtual int Get_Object_Size() const override;
    virtual void Detach(AbstractClass * target, bool all = true) override;
    virtual void Object_CRC(CRCEngine &crc) const override;

    virtual BuildingTypeClass *This() const override { return reinterpret_cast<BuildingTypeClass *>(TechnoTypeClassExtension::This()); }
    virtual const BuildingTypeClass *This_Const() const override { return reinterpret_cast<const BuildingTypeClass *>(TechnoTypeClassExtension::This_Const()); }
    virtual RTTIType Fetch_RTTI() const override { return RTTI_BUILDINGTYPE; }

    virtual bool Read_INI(CCINIClass &ini) override;

    void Fetch_Building_Normal_Image(TheaterType theater);

public:
    /**
     *  This is the sound effect to play when the animation of the gate is rising.
     */
    VocType GateUpSound;

    /**
     *  This is the sound effect to play when the animation of the gate is lowering.
     */
    VocType GateDownSound;

    /**
     *  Credits bonus when captured from a house with "IsMultiplayPassive" set.
     */
    unsigned ProduceCashStartup;
    
    /**
     *  Amount every give to/take from the house every delay.
     */
    int ProduceCashAmount;
    
    /**
     *  Frame delay between amounts.
     */
    unsigned ProduceCashDelay;
    
    /**
     *  The total budget for this building.
     */
    unsigned ProduceCashBudget;
    
    /**
     *  Is the capture bonus a "one one" special (further captures will not get the bonus)?
     */
    bool IsStartupCashOneTime;
    
    /**
     *  Reset the available budget when captured?
     */
    bool IsResetBudgetOnCapture;

    /**
     *  Is this building eligible for proximity checks by players who are its owner's allies?
     */
    bool IsEligibleForAllyBuilding;

    /**
     *  The percent chance for an engineer to exit this building as its crew.
     */
    int EngineerChance;

    /**
     *  Should the building hide its main shape during the special anims?
     *  Usually used for missile silos so that anims that don't completely
     *  hide the main shape don't look glitched.
     */
    bool IsHideDuringSpecialAnim;

    /**
     *  New shapes for roof door anims.
     */
    const ShapeSet* RoofDeployingAnim;
    const ShapeSet* RoofDoorAnim;
    const ShapeSet* UnderRoofDoorAnim;

    /**
     *  If this building is a factory, can it only produce those units that list it in BuiltAt=?
     */
    bool IsExclusiveFactory;
};
