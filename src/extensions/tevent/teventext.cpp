/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TEVENTEXT.CPP
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

#include "always.h"

#include "teventext.h"

#include "extension.h"
#include "tevent.h"
#include "vinifera_saveload.h"
#include "wwcrc.h"


TEventClass::EventDescriptionStruct TEventClassExtension::ExtActionDescriptions[EXT_TEVENT_COUNT - EXT_TEVENT_FIRST] = {
    { "Compare Global with Constant", "Compares a global variable with a constant using a selected operation." },
    { "Compare Global with Global", "Compares two global variables using a selected operation." },
    { "Compare Global with Local", "Compares a global variable with a local variable using a selected operation." },
    { "Global equals Constant", "Checks if a global variable equals a constant." },
    { "Global equals Global", "Checks if two global variables are equal." },
    { "Global equals Local", "Checks if a global variable equals a local variable." },
    { "Global greater than Constant", "Checks if a global variable is greater than a constant." },
    { "Global greater than Global", "Checks if one global variable is greater than another." },
    { "Global greater than Local", "Checks if a global variable is greater than a local variable." },
    { "Global less than Constant", "Checks if a global variable is less than a constant." },
    { "Global less than Global", "Checks if one global variable is less than another." },
    { "Global less than Local", "Checks if a global variable is less than a local variable." },
    { "Compare Local with Constant", "Compares a local variable with a constant using a selected operation." },
    { "Compare Local with Global", "Compares a local variable with a global variable using a selected operation." },
    { "Compare Local with Local", "Compares two local variables using a selected operation." },
    { "Local equals Constant", "Checks if a local variable equals a constant." },
    { "Local equals Global", "Checks if a local variable equals a global variable." },
    { "Local equals Local", "Checks if two local variables are equal." },
    { "Local greater than Constant", "Checks if a local variable is greater than a constant." },
    { "Local greater than Global", "Checks if a local variable is greater than a global variable." },
    { "Local greater than Local", "Checks if one local variable is greater than another." },
    { "Local less than Constant", "Checks if a local variable is less than a constant." },
    { "Local less than Global", "Checks if a local variable is less than a global variable." },
    { "Local less than Local", "Checks if one local variable is less than another." },
    { "Building does not exist", "Triggers when the building specified (owned by the house of this trigger) does not exist." },
};


/**
 *  Class constructor.
 *  
 *  @author: ZivDero
 */
TEventClassExtension::TEventClassExtension(const TEventClass *this_ptr) :
    AbstractClassExtension(this_ptr),
    IniNameArgument { "" }
{
    //if (this_ptr) EXT_DEBUG_TRACE("TEventClassExtension::TEventClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    TEventExtensions.Add(this);

    Data2.Value = 0;
    Data3.Value = 0;
}


/**
 *  Class no-init constructor.
 *  
 *  @author: ZivDero
 */
TEventClassExtension::TEventClassExtension(const NoInitClass &noinit) :
    AbstractClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("TEventClassExtension::TEventClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: ZivDero
 */
TEventClassExtension::~TEventClassExtension()
{
    //EXT_DEBUG_TRACE("TEventClassExtension::~TEventClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    TEventExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: ZivDero
 */
HRESULT TEventClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("TEventClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (lpClassID == nullptr) {
        return E_POINTER;
    }

    *lpClassID = __uuidof(this);

    return S_OK;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: ZivDero
 */
HRESULT TEventClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("TEventClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = AbstractClassExtension::Internal_Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) TEventClassExtension(NoInitClass());

    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: ZivDero
 */
HRESULT TEventClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("TEventClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = AbstractClassExtension::Internal_Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: ZivDero
 */
int TEventClassExtension::Get_Object_Size() const
{
    //EXT_DEBUG_TRACE("TEventClassExtension::Get_Object_Size - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: ZivDero
 */
void TEventClassExtension::Detach(AbstractClass * target, bool all)
{
    //EXT_DEBUG_TRACE("TEventClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: ZivDero
 */
void TEventClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("TEventClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Returns the name of the TEventType.
 *
 *  @author: ZivDero
 */
const char* TEventClassExtension::Event_Name(int event)
{
    if (event < TACTION_COUNT) {
        return TEventClass::Event_Name(static_cast<TEventType>(event));
    }

    if (event < EXT_TACTION_COUNT) {
        return ExtActionDescriptions[event - EXT_TACTION_FIRST].Name;
    }

    return "<invalid>";
}


/**
 *  Returns the description of the TEventType.
 *
 *  @author: ZivDero
 */
const char* TEventClassExtension::Event_Description(int event)
{
    if (event < TACTION_COUNT) {
        return TEventClass::Event_Description(static_cast<TEventType>(event));
    }

    if (event < EXT_TACTION_COUNT) {
        return ExtActionDescriptions[event - EXT_TACTION_FIRST].Description;
    }

    return "<invalid>";
}
