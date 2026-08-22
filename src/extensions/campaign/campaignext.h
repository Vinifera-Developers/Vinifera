/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended CampaignClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "abstracttypeext.h"
#include "campaign.h"


class DECLSPEC_UUID(UUID_CAMPAIGN_EXTENSION)
CampaignClassExtension final : public AbstractTypeClassExtension
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
        CampaignClassExtension(const CampaignClass *this_ptr = nullptr);
        CampaignClassExtension(const NoInitClass &noinit);
        virtual ~CampaignClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual CampaignClass *This() const override { return reinterpret_cast<CampaignClass *>(AbstractTypeClassExtension::This()); }
        virtual const CampaignClass *This_Const() const override { return reinterpret_cast<const CampaignClass *>(AbstractTypeClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_CAMPAIGN; }

        virtual bool Read_INI(CCINIClass& ini) override;

        HousesType Get_House() const;
        void Set_House(HousesType house);

    public:
        /**
         *  Is this campaign only available in Developer mode?
         */
        bool IsDebugOnly;

        /**
         *  The movie to play at start of this campaign.
         */
        char IntroMovie[64];

        /**
         *  The HOUSE (not side!) this campaign is played as.
         */
        HousesType _House;
        __declspec(property(get = Get_House, put = Set_House)) HousesType House;
};
