/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          FACTORYEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended FactoryClass class.
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
#include "factoryext.h"
#include "factory.h"
#include "wwcrc.h"
#include "extension.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
FactoryClassExtension::FactoryClassExtension(const FactoryClass *this_ptr) :
    AbstractClassExtension(this_ptr),
    IsHoldingExit(false),
    HasSpoken(false)
{
    //if (this_ptr) EXT_DEBUG_TRACE("FactoryClassExtension::FactoryClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    FactoryExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
FactoryClassExtension::FactoryClassExtension(const NoInitClass &noinit) :
    AbstractClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("FactoryClassExtension::FactoryClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
FactoryClassExtension::~FactoryClassExtension()
{
    //EXT_DEBUG_TRACE("FactoryClassExtension::~FactoryClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    FactoryExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT FactoryClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("FactoryClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (lpClassID == nullptr) {
        return E_POINTER;
    }

    *lpClassID = __uuidof(this);

    return S_OK;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT FactoryClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("FactoryClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = AbstractClassExtension::Internal_Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) FactoryClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT FactoryClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("FactoryClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = AbstractClassExtension::Internal_Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: CCHyper
 */
int FactoryClassExtension::Get_Object_Size() const
{
    //EXT_DEBUG_TRACE("FactoryClassExtension::Get_Object_Size - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void FactoryClassExtension::Detach(AbstractClass * target, bool all)
{
    //EXT_DEBUG_TRACE("FactoryClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void FactoryClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("FactoryClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}
