/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TEVENTEXT.H
 *
 *  @author        ZivDero
 *
 *  @brief         Extended TEventClass class.
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
#include "extension.h"
#include "tevent.h"


class DECLSPEC_UUID(UUID_EVENT_EXTENSION)
TEventClassExtension final : public AbstractClassExtension
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
    TEventClassExtension(const TEventClass *this_ptr = nullptr);
    TEventClassExtension(const NoInitClass &noinit);
    virtual ~TEventClassExtension();

    virtual int Get_Object_Size() const override;
    virtual void Detach(AbstractClass * target, bool all = true) override;
    virtual void Object_CRC(CRCEngine &crc) const override;

    virtual TEventClass *This() const override { return reinterpret_cast<TEventClass *>(AbstractClassExtension::This()); }
    virtual const TEventClass *This_Const() const override { return reinterpret_cast<const TEventClass *>(AbstractClassExtension::This_Const()); }
    virtual RTTIType Fetch_RTTI() const override { return RTTI_UNIT; }

    /**
     *  Trigger events don't have names.
     */
    virtual const char* Name() const { return ""; }
    virtual const char* Full_Name() const { return ""; }

    static const char* Event_Name(int event);
    static const char* Event_Description(int event);

public:

    /**
     *  An INI name passed as an argument.
     */
    char IniNameArgument[32];

    /**
     *  Extra space for arguments.
     */
    union {
        int Value;
    } Data2;

    union {
        int Value;
    } Data3;

private:
    static TEventClass::EventDescriptionStruct ExtActionDescriptions[EXT_TEVENT_COUNT - EXT_TEVENT_FIRST];
};
