/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended SessionClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "extension.h"
#include "session.h"


class SessionClassExtension final : public GlobalExtensionClass<SessionClass>
{
    public:
        IFACEMETHOD(Load)(IStream *pStm);
        IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

    public:
        SessionClassExtension(const SessionClass *this_ptr);
        SessionClassExtension(const NoInitClass &noinit);
        virtual ~SessionClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual const char *Name() const override { return "Session"; }
        virtual const char *Full_Name() const override { return "Session"; }

        void Read_MultiPlayer_Settings();
        void Write_MultiPlayer_Settings();

    public:
        typedef struct ExtGameOptionsType
        {
            /**
             *  Should the MCV unit auto deploy on game start?
             */
            bool IsAutoDeployMCV;

            /**
             *  Are construction yards pre-placed on the map rather than a MCV given to the player?
             */
            bool IsPrePlacedConYards;

            /**
             *  Can players build their own structures adjacent to structures owned by their allies?
             */
            bool IsBuildOffAlly;

        } ExtGameOptionsType;

        ExtGameOptionsType ExtOptions;

        /**
         *  Is the message we're currently writing meant to be sent to allies only?
         */
        bool IsChatToAllies;

        /**
         *  If we're writing a private message, this is the name of its recipient.
         */
        char MessageRecipientName[32];
};
