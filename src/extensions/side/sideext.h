/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended SideClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "abstracttypeext.h"
#include "house.h"
#include "housetype.h"
#include "side.h"
#include "tibsun_globals.h"

#define OPTIONS_MENU_TEXT_DEFAULT_COLOR RGBClass(112, 255, 0)
#define SCREEN_TEXT_DEFAULT_COLOR       RGBClass(255, 255, 255)

class InfantryTypeClass;

class DECLSPEC_UUID(UUID_SIDE_EXTENSION)
SideClassExtension final : public AbstractTypeClassExtension
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
        SideClassExtension(const SideClass *this_ptr = nullptr);
        SideClassExtension(const NoInitClass &noinit);
        virtual ~SideClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual SideClass *This() const override { return reinterpret_cast<SideClass *>(AbstractTypeClassExtension::This()); }
        virtual const SideClass *This_Const() const override { return reinterpret_cast<const SideClass *>(AbstractTypeClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_SIDE; }

        virtual bool Read_INI(CCINIClass &ini) override;

        static const InfantryTypeClass* Get_Crew(SideType side);
        static const InfantryTypeClass* Get_Engineer(SideType side);
        static const InfantryTypeClass* Get_Technician(SideType side);
        static const InfantryTypeClass* Get_Disguise(SideType side);
        static int Get_Survivor_Divisor(SideType side);

        inline static const InfantryTypeClass* Get_Crew(const HouseClass* house) { return Get_Crew(house->Class->Side); }
        inline static const InfantryTypeClass* Get_Engineer(const HouseClass* house) { return Get_Engineer(house->Class->Side); }
        inline static const InfantryTypeClass* Get_Technician(const HouseClass* house) { return Get_Technician(house->Class->Side); }
        inline static const InfantryTypeClass* Get_Disguise(const HouseClass* house) { return Get_Disguise(house->Class->Side); }
        inline static int Get_Survivor_Divisor(const HouseClass* house) { return Get_Survivor_Divisor(house->Class->Side); }

    public:

        /**
         *  Color scheme to be used in the UI of this side.
         */
        ColorSchemeType UIColor;

        /**
         *  Color scheme to be used for hover-on effects of UI elements for this side.
         */
        ColorSchemeType HoverHighlightColor;

        /**
         *  Color scheme to be used for the tooltips of this side.
         */
        ColorSchemeType ToolTipColor;

        /**
         *  InfantryType used as this Side's crew.
         */
        const InfantryTypeClass* Crew;

        /**
         *  InfantryType used as this Side's engineer.
         */
        const InfantryTypeClass* Engineer;

        /**
         *  InfantryType used as this Side's technician.
         */
        const InfantryTypeClass* Technician;

        /**
         *  InfantryType used as this Side's disguise.
         */
        const InfantryTypeClass* Disguise;

        /**
         *  The number of survivors is divided by this much when calculating a building's number of survivors.
         */
        int SurvivorDivisor;

        /**
         *  BuildingType used as this Side's regular power plant.
         */
        const BuildingTypeClass* RegularPowerPlant;

        /**
         *  BuildingType used as this Side's advanced power plant.
         */
        const BuildingTypeClass* AdvancedPowerPlant;

        /**
         *  BuildingType used as this Side's power turbine.
         */
        const BuildingTypeClass* PowerTurbine;

        /**
         *  UnitType used as this Side's Hunter-Seeker.
         */
        const UnitTypeClass* HunterSeeker;

        /**
         *  Color to be used for options menu text for this side.
         */
        RGBClass OptionsMenuTextColor;

        /**
         *  Color to be used for screen text for this side.
         */
        RGBClass ScreenTextColor;
};
