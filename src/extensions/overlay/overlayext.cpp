/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          OVERLAYEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended OverlayClass class.
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
#include "overlayext.h"
#include "overlay.h"
#include "overlaytype.h"
#include "overlaytypeext.h"
#include "wwcrc.h"
#include "lightsource.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Provides the map for all OverlayClass extension instances.
 */
ExtensionMap<OverlayClass, OverlayClassExtension> OverlayClassExtensions;


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
OverlayClassExtension::OverlayClassExtension(OverlayClass *this_ptr) :
    Extension(this_ptr),

    LightSource(nullptr)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("OverlayClassExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("OverlayClassExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    IsInitialized = true;
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
OverlayClassExtension::OverlayClassExtension(const NoInitClass &noinit) :
    Extension(noinit)
{
    IsInitialized = false;
}


/**
 *  Class deconstructor.
 *  
 *  @author: CCHyper
 */
OverlayClassExtension::~OverlayClassExtension()
{
    //EXT_DEBUG_TRACE("OverlayClassExtension deconstructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("OverlayClassExtension deconstructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    if (LightSource) {
        LightSource->Disable();
        delete LightSource;
        LightSource = nullptr;
    }

    IsInitialized = false;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT OverlayClassExtension::Load(IStream *pStm)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("OverlayClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    HRESULT hr = Extension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) OverlayClassExtension(NoInitClass());

    LightSource = nullptr;
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT OverlayClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("OverlayClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    HRESULT hr = Extension::Save(pStm, fClearDirty);
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
int OverlayClassExtension::Size_Of() const
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("OverlayClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void OverlayClassExtension::Detach(TARGET target, bool all)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("OverlayClassExtension::Detach - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void OverlayClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("OverlayClassExtension::Compute_CRC - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
}


/**
 *  Unlimbo the overlay object onto the map. 
 * 
 *  @author: CCHyper
 */
bool OverlayClassExtension::Unlimbo(Coordinate &coord, DirType dir)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("OverlayClassExtension::Unlimbo - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    OverlayClassExtension *overlayext;
    OverlayTypeClassExtension *overlaytypeext;

    /**
     *  #issue-507
     * 
     *  Create the light source object when the overlay is placed
     *  into the game world.
     * 
     *  @author: CCHyper
     */

    /**
     *  Fetch the extended class instances if they exist.
     */
    overlayext = OverlayClassExtensions.find(ThisPtr);
    overlaytypeext = OverlayTypeClassExtensions.find(ThisPtr->Class);

    if (overlaytypeext && overlaytypeext->LightIntensity > 0) {

        if (overlayext && !overlayext->LightSource) {

            /**
             *  Create the light source object at the overlay coord.
             */
            LightSourceClass *light = new LightSourceClass(
                                                ThisPtr->Center_Coord(),
                                                overlaytypeext->LightVisibility,
                                                overlaytypeext->LightIntensity,
                                                overlaytypeext->LightRedTint,
                                                overlaytypeext->LightGreenTint,
                                                overlaytypeext->LightBlueTint
                                            );
            ASSERT(light != nullptr);

            if (light) {
                overlayext->LightSource = light;

                /**
                 *  Enable the light source.
                 */
                overlayext->LightSource->Enable();
            }

        }

    }

    return true;
}
