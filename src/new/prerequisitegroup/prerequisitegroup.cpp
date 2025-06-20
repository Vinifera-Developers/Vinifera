/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          PREREQUISITEGROUP.CPP
 *
 *  @authors       ZivDero
 *
 *  @brief         New Prerequisite Group class.
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
#include "prerequisitegroup.h"
#include "ccini.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "asserthandler.h"
#include "vinifera_saveload.h"


 /**
  *  Basic constructor for armor objects.
  *
  *  @author: ZivDero
  */
PrerequisiteGroupClass::PrerequisiteGroupClass()
{
    PrerequisiteGroups.Add(this);
}


/**
 *  Basic constructor for armor objects.
 *
 *  @author: ZivDero
 */
PrerequisiteGroupClass::PrerequisiteGroupClass(const char* name) :
    IniName(""),
    Prerequisites()
{
    ASSERT_FATAL_PRINT(name != nullptr, "Invalid name for PrerequisiteGroup!");

    std::strncpy(IniName, name, sizeof(IniName));

    PrerequisiteGroups.Add(this);
}


/**
 *  Class destructor.
 *
 *  @author: ZivDero
 */
PrerequisiteGroupClass::~PrerequisiteGroupClass()
{
    PrerequisiteGroups.Delete(this);
}


/**
 *  Retrieves pointers to the supported interfaces on an object.
 *
 *  @author: ZivDero
 */
LONG PrerequisiteGroupClass::QueryInterface(REFIID riid, LPVOID* ppv)
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
 *  @author: ZivDero
 */
ULONG PrerequisiteGroupClass::AddRef()
{
    //EXT_DEBUG_TRACE("PrerequisiteGroupClass::AddRef - 0x%08X\n", (uintptr_t)(this));

    return 1;
}


/**
 *  Decrements the reference count for an interface on a COM object.
 *
 *  @author: ZivDero
 */
ULONG PrerequisiteGroupClass::Release()
{
    //EXT_DEBUG_TRACE("PrerequisiteGroupClass::Release - 0x%08X\n", (uintptr_t)(this));

    return 1;
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *
 *  @author: ZivDero
 */
HRESULT PrerequisiteGroupClass::GetClassID(CLSID* lpClassID)
{
    //EXT_DEBUG_TRACE("PrerequisiteGroupClass::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(this));

    if (lpClassID == nullptr) {
        return E_POINTER;
    }

    *lpClassID = __uuidof(this);

    return S_OK;
}


/**
 *  Determines whether an object has changed since it was last saved to its stream.
 *
 *  @author: ZivDero
 */
HRESULT PrerequisiteGroupClass::IsDirty()
{
    //EXT_DEBUG_TRACE("PrerequisiteGroupClass::IsDirty - 0x%08X\n", (uintptr_t)(this));

    return S_OK;
}


/**
 *  Loads the object from the stream and requests a new pointer to
 *  the class we extended post-load.
 *
 *  @author: ZivDero
 */
HRESULT PrerequisiteGroupClass::Load(IStream* pStm)
{
    //EXT_DEBUG_TRACE("PrerequisiteGroupClass::Internal_Load - 0x%08X\n", (uintptr_t)(this));

    if (!pStm) {
        return E_POINTER;
    }

    Prerequisites.Clear();

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

    new (this) PrerequisiteGroupClass(NoInitClass());

    Prerequisites.Load(pStm);

    return hr;
}


/**
 *  Saves the object to the stream.
 *
 *  @author: ZivDero
 */
HRESULT PrerequisiteGroupClass::Save(IStream* pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("PrerequisiteGroupClass::Internal_Save - 0x%08X\n", (uintptr_t)(this));

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

    Prerequisites.Save(pStm);

    return hr;
}


/**
 *  Retrieves the size of the stream needed to save the object.
 *
 *  @author: ZivDero
 */
LONG PrerequisiteGroupClass::GetSizeMax(ULARGE_INTEGER* pcbSize)
{
    //EXT_DEBUG_TRACE("PrerequisiteGroupClass::GetSizeMax - 0x%08X\n", (uintptr_t)(this));

    if (!pcbSize) {
        return E_POINTER;
    }

    pcbSize->LowPart = sizeof(*this) + sizeof(uint32_t); // Add size of swizzle "id".
    pcbSize->HighPart = 0;

    return S_OK;
}


/**
 *  Retrieves the PrerequisiteGroupType for given name.
 *
 *  @author: ZivDero
 */
PrerequisiteGroupType PrerequisiteGroupClass::From_Name(const char* name)
{
    ASSERT(name != nullptr);

    if (name != nullptr) {
        for (PrerequisiteGroupType group = PREREQ_GROUP_FIRST; group < PrerequisiteGroups.Count(); group++) {
            if (std::strncmp(PrerequisiteGroups[group]->IniName, name, sizeof(IniName)) == 0) {
                return group;
            }
        }
    }

    return PREREQ_GROUP_NONE;
}


/**
 *  Returns name for given prerequisite group.
 *
 *  @author: ZivDero
 */
const char* PrerequisiteGroupClass::Name_From(PrerequisiteGroupType type)
{
    ASSERT(type >= PREREQ_GROUP_FIRST && type < PrerequisiteGroups.Count());

    return PrerequisiteGroups[type]->IniName;
}


/**
 *  Find or create a prerequisite group of the type specified.
 *
 *  @author: ZivDero
 */
const PrerequisiteGroupClass* PrerequisiteGroupClass::Find_Or_Make(const char* name)
{
    ASSERT(name != nullptr);

    for (PrerequisiteGroupType group = PREREQ_GROUP_FIRST; group < PrerequisiteGroups.Count(); group++) {
        if (std::strncmp(PrerequisiteGroups[group]->IniName, name, sizeof(IniName)) == 0) {
            return PrerequisiteGroups[group];
        }
    }

    PrerequisiteGroupClass* ptr = new PrerequisiteGroupClass(name);
    ASSERT(ptr != nullptr);
    return ptr;
}


/**
 *  Parse the prerequisite group from a string.
 *
 *  @author: ZivDero
 */
void PrerequisiteGroupClass::Parse_String(char* string)
{
    Prerequisites.Clear();
    char* token = std::strtok(string, ",");
    while (token != nullptr && *token != '\0') {
        int building = BuildingTypeClass::From_Name(token);
        if (building != STRUCT_NONE) {
            Prerequisites.Add(building);
        }

        token = std::strtok(nullptr, ",");
    }
}


/**
 *  Reads a prerequisite group object data from an INI file.
 *
 *  @author: ZivDero
 */
bool PrerequisiteGroupClass::Read_INI(CCINIClass& ini)
{
    static const char* const PREREQUISITE_GROUPS = "PrerequisiteGroups";

    char buffer[512];

    if (ini.Get_String(PREREQUISITE_GROUPS, IniName, "", buffer, sizeof(buffer)) > 0) {
        Parse_String(buffer);
    }

    return true;
}


/**
 *  Reads the vanilla prerequisite groups from [General].
 *
 *  @author: ZivDero
 */
bool PrerequisiteGroupClass::Read_Global_INI(CCINIClass& ini)
{
    static struct {
        char const* Entry;
        char const* Name;
    } _vanilla_prerequisites[7] = {
        { "PrerequisitePower", "POWER" },
        { "PrerequisiteBarracks", "BARRACKS" },
        { "PrerequisiteFactory", "FACTORY" },
        { "PrerequisiteRadar", "RADAR" },
        { "PrerequisiteTech", "TECH" },
        { "PrerequisiteGDIFactory", "GDIFACTORY" },
        { "PrerequisiteNodFactory", "NODFACTORY" },
    };

    char buffer[512];

    for (const auto& prereq : _vanilla_prerequisites) {
        PrerequisiteGroupClass* group = PrerequisiteGroups[From_Name(prereq.Name)];

        if (ini.Get_String("General", prereq.Entry, "", buffer, sizeof(buffer)) > 0) {
            group->Parse_String(buffer);
        }
    }

    return true;
}


/**
 *  Performs one time initialization of the prerequisite group class.
 *
 *  @warning: Do not change this function, otherwise it will break support
 *            with the original game!
 *
 *  @author: ZivDero
 */
bool PrerequisiteGroupClass::One_Time()
{
    static const char* _vanilla_prerequisites[7] = {
        "POWER",
        "BARRACKS",
        "FACTORY",
        "RADAR",
        "TECH",
        "GDIFACTORY",
        "NODFACTORY",
    };

    /**
     *  Create the default prerequisite groups.
     */
    for (const char* name : _vanilla_prerequisites) {
        PrerequisiteGroupClass* group = new PrerequisiteGroupClass(name);
        ASSERT(group != nullptr);
    }

    return true;
}
