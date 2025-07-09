/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ARMORTYPE.CPP
 *
 *  @authors       CCHyper, ZivDero
 *
 *  @brief         New ArmorType class.
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
#include "armortype.h"
#include "ccini.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "asserthandler.h"
#include "vinifera_saveload.h"


/**
 *  Basic constructor for armor objects.
 *
 *  @author: CCHyper
 */
ArmorTypeClass::ArmorTypeClass()
{
    ArmorTypes.Add(this);
}


/**
 *  Basic constructor for armor objects.
 *
 *  @author: CCHyper
 */
ArmorTypeClass::ArmorTypeClass(const char* name) :
    IniName(""),
    Modifier(1.0),
    ForceFire(true),
    PassiveAcquire(true),
    Retaliate(true),
    BaseArmor(ARMOR_NULL)
{
    ASSERT_FATAL_PRINT(name != nullptr, "Invalid name for ArmorType!");

    std::strncpy(IniName, name, sizeof(IniName));

    ArmorTypes.Add(this);
}


/**
 *  Class destructor.
 *
 *  @author: CCHyper
 */
ArmorTypeClass::~ArmorTypeClass()
{
    ArmorTypes.Delete(this);
}


/**
 *  Retrieves pointers to the supported interfaces on an object.
 *
 *  @author: CCHyper, tomsons26
 */
LONG ArmorTypeClass::QueryInterface(REFIID riid, LPVOID* ppv)
{
    /**
     *  Always set out parameter to NULL, validating it first.
     */
    if (ppv == nullptr) {
        return E_POINTER;
    }
    *ppv = nullptr;

    if (riid == __uuidof(IUnknown)) {
        *ppv = reinterpret_cast<IUnknown*>(this);
    }

    if (riid == __uuidof(IStream)) {
        *ppv = reinterpret_cast<IStream*>(this);
    }

    if (riid == __uuidof(IPersistStream)) {
        *ppv = static_cast<IPersistStream*>(this);
    }

    if (*ppv == nullptr) {
        return E_NOINTERFACE;
    }

    /**
     *  Increment the reference count and return the pointer.
     */
    reinterpret_cast<IUnknown*>(*ppv)->AddRef();

    return S_OK;
}


/**
 *  Increments the reference count for an interface pointer to a COM object.
 *
 *  @author: CCHyper
 */
ULONG ArmorTypeClass::AddRef()
{
    //EXT_DEBUG_TRACE("ArmorTypeClass::AddRef - 0x%08X\n", (uintptr_t)(this));

    return 1;
}


/**
 *  Decrements the reference count for an interface on a COM object.
 *
 *  @author: CCHyper
 */
ULONG ArmorTypeClass::Release()
{
    //EXT_DEBUG_TRACE("ArmorTypeClass::Release - 0x%08X\n", (uintptr_t)(this));

    return 1;
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *
 *  @author: CCHyper
 */
HRESULT ArmorTypeClass::GetClassID(CLSID* lpClassID)
{
    //EXT_DEBUG_TRACE("ArmorTypeClass::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(this));

    if (lpClassID == nullptr) {
        return E_POINTER;
    }

    *lpClassID = __uuidof(this);

    return S_OK;
}


/**
 *  Determines whether an object has changed since it was last saved to its stream.
 *
 *  @author: CCHyper
 */
HRESULT ArmorTypeClass::IsDirty()
{
    //EXT_DEBUG_TRACE("ArmorTypeClass::IsDirty - 0x%08X\n", (uintptr_t)(this));

    return S_OK;
}


/**
 *  Loads the object from the stream and requests a new pointer to
 *  the class we extended post-load.
 *
 *  @author: CCHyper, tomsons26
 */
HRESULT ArmorTypeClass::Load(IStream* pStm)
{
    //EXT_DEBUG_TRACE("ArmorTypeClass::Internal_Load - 0x%08X\n", (uintptr_t)(this));

    if (!pStm) {
        return E_POINTER;
    }

    /**
     *  Load the unique id for this class.
     */
    LONG id = 0;
    HRESULT hr = pStm->Read(&id, sizeof(LONG), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    /**
     *  Register this instance to be available for remapping references to.
     */
    VINIFERA_SWIZZLE_REGISTER_POINTER(id, this, IniName);

    /**
     *  Read this class's binary blob data directly into this instance.
     */
    hr = pStm->Read(this, sizeof(*this), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    new (this) ArmorTypeClass(NoInitClass());

    return hr;
}


/**
 *  Saves the object to the stream.
 *
 *  @author: CCHyper, tomsons26
 */
HRESULT ArmorTypeClass::Save(IStream* pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("ArmorTypeClass::Internal_Save - 0x%08X\n", (uintptr_t)(this));

    if (!pStm) {
        return E_POINTER;
    }

    /**
     *  Fetch the save id for this instance.
     */
    const LONG id = reinterpret_cast<LONG>(this);

    //DEV_DEBUG_INFO("Writing id = 0x%08X.\n", id);

    HRESULT hr = pStm->Write(&id, sizeof(id), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    /**
     *  Write this class instance as a binary blob.
     */
    hr = pStm->Write(this, sizeof(*this), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Retrieves the size of the stream needed to save the object.
 *
 *  @author: CCHyper, tomsons26
 */
LONG ArmorTypeClass::GetSizeMax(ULARGE_INTEGER* pcbSize)
{
    //EXT_DEBUG_TRACE("ArmorTypeClass::GetSizeMax - 0x%08X\n", (uintptr_t)(this));

    if (!pcbSize) {
        return E_POINTER;
    }

    pcbSize->LowPart = sizeof(*this) + sizeof(uint32_t); // Add size of swizzle "id".
    pcbSize->HighPart = 0;

    return S_OK;
}


/**
 *  Retrieves the ArmorType for given name.
 *
 *  @author: CCHyper
 */
ArmorType ArmorTypeClass::From_Name(const char *name)
{
    ASSERT(name != nullptr);

    if (name != nullptr) {
        for (ArmorType armor = ARMOR_FIRST; armor < ArmorTypes.Count(); armor++) {
            if (std::strncmp(ArmorTypes[armor]->IniName, name, sizeof(IniName)) == 0) {
                return armor;
            }
        }
    }

    return ARMOR_NONE;
}


/**
 *  Returns name for given armor type.
 *
 *  @author: CCHyper
 */
const char *ArmorTypeClass::Name_From(ArmorType type)
{
    ASSERT(type >= ARMOR_FIRST && type < ArmorTypes.Count());

    return ArmorTypes[type]->IniName;
}


/**
 *  Find or create a armor of the type specified.
 *
 *  @author: CCHyper
 */
const ArmorTypeClass *ArmorTypeClass::Find_Or_Make(const char *name)
{
    ASSERT(name != nullptr);

    for (ArmorType armor = ARMOR_FIRST; armor < ArmorTypes.Count(); ++armor) {
        if (std::strncmp(ArmorTypes[armor]->IniName, name, sizeof(IniName)) == 0) {
            return ArmorTypes[armor];
        }
    }

    ArmorTypeClass *ptr = new ArmorTypeClass(name);
    ASSERT(ptr != nullptr);
    return ptr;
}


/**
 *  Reads armor object data from an INI file.
 *
 *  @author: ZivDero
 */
bool ArmorTypeClass::Read_INI(CCINIClass& ini)
{
    if (!ini.Is_Present(IniName)) {
        return false;
    }

    Modifier = ini.Get_Double(IniName, "Modifier", Modifier);
    ForceFire = ini.Get_Bool(IniName, "ForceFire", ForceFire);
    PassiveAcquire = ini.Get_Bool(IniName, "PassiveAcquire", PassiveAcquire);
    Retaliate = ini.Get_Bool(IniName, "Retaliate", Retaliate);
    BaseArmor = ini.Get_ArmorType(IniName, "BaseArmor", BaseArmor);

    return true;
}


/**
 *  Performs one time initialization of the armor type class.
 *
 *  @warning: Do not change this function, otherwise it will break support
 *            with the original game!
 *
 *  @author: CCHyper
 */
bool ArmorTypeClass::One_Time()
{
    ArmorTypeClass *armor = nullptr;

    /**
     *  Create the default armor types.
     */

    armor = new ArmorTypeClass(ArmorName[ARMOR_NONE]);
    ASSERT(armor != nullptr);

    armor = new ArmorTypeClass(ArmorName[ARMOR_WOOD]);
    ASSERT(armor != nullptr);

    armor = new ArmorTypeClass(ArmorName[ARMOR_ALUMINUM]);
    ASSERT(armor != nullptr);

    armor = new ArmorTypeClass(ArmorName[ARMOR_STEEL]);
    ASSERT(armor != nullptr);

    armor = new ArmorTypeClass(ArmorName[ARMOR_CONCRETE]);
    ASSERT(armor != nullptr);

    return true;
}
