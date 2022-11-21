/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SESSIONEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended SessionClass class.
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

#include "always.h"
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

        virtual int Size_Of() const override;
        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

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
};
