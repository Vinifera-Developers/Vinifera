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

#include "extension.h"
#include "container.h"


class SessionClass;
class CCINIClass;


class SessionClassExtension final : public Extension<SessionClass>
{
    public:
        SessionClassExtension(SessionClass *this_ptr);
        SessionClassExtension(const NoInitClass &noinit);
        ~SessionClassExtension();

        virtual HRESULT Load(IStream *pStm) override { return S_OK; }
        virtual HRESULT Save(IStream *pStm, BOOL fClearDirty) override { return S_OK; }
        virtual int Size_Of() const override;

        virtual void Detach(TARGET target, bool all = true) override {}
        virtual void Compute_CRC(WWCRCEngine &crc) const override {}

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

        } ExtGameOptionsType;

        ExtGameOptionsType ExtOptions;
};


extern SessionClassExtension *SessionExtension;
