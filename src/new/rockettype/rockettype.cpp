/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ROCKETTYPE.CPP
 *
 *  @authors       ZivDero
 *
 *  @brief         Class containing configuration for AircraftType rockets.
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
#include "rockettype.h"
#include "ccini.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "asserthandler.h"
#include "vinifera_saveload.h"


 /**
  *  Basic constructor for rocket objects.
  *
  *  @author: ZivDero
  */
RocketTypeClass::RocketTypeClass()
{
    RocketTypes.Add(this);
}


/**
 *  Basic constructor for rocket objects.
 *
 *  @author: ZivDero
 */
RocketTypeClass::RocketTypeClass(const char* name) :
    IniName(""),
    PauseFrames(0),
    TiltFrames(0),
    PitchInitial(0.0f),
    PitchFinal(0.0f),
    TurnRate(0.0f),
    RaiseRate(0),
    Acceleration(0.0f),
    Altitude(0),
    Damage(0),
    EliteDamage(0),
    BodyLength(0),
    IsLazyCurve(false),
    IsCruiseMissile(false),
    CloseEnoughFactor(1.0),
    Type(nullptr),
    Warhead(nullptr),
    EliteWarhead(nullptr),
    TakeoffAnim(nullptr),
    TrailAnim(nullptr),
    TrailSpawnDelay(3),
    TrailAppearDelay(2),
    Inaccuracy(0)
{
    ASSERT_FATAL_PRINT(name != nullptr, "Invalid name for RocketType!");

    std::strncpy(IniName, name, sizeof(IniName));

    RocketTypes.Add(this);
}


/**
 *  Class destructor.
 *
 *  @author: ZivDero
 */
RocketTypeClass::~RocketTypeClass()
{
    RocketTypes.Delete(this);
}


/**
 *  Retrieves pointers to the supported interfaces on an object.
 *
 *  @author: CCHyper, tomsons26
 */
LONG RocketTypeClass::QueryInterface(REFIID riid, LPVOID* ppv)
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
ULONG RocketTypeClass::AddRef()
{
    //EXT_DEBUG_TRACE("RocketTypeClass::AddRef - 0x%08X\n", (uintptr_t)(this));

    return 1;
}


/**
 *  Decrements the reference count for an interface on a COM object.
 *
 *  @author: CCHyper
 */
ULONG RocketTypeClass::Release()
{
    //EXT_DEBUG_TRACE("RocketTypeClass::Release - 0x%08X\n", (uintptr_t)(this));

    return 1;
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *
 *  @author: CCHyper
 */
HRESULT RocketTypeClass::GetClassID(CLSID* lpClassID)
{
    //EXT_DEBUG_TRACE("RocketTypeClass::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(this));

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
HRESULT RocketTypeClass::IsDirty()
{
    //EXT_DEBUG_TRACE("RocketTypeClass::IsDirty - 0x%08X\n", (uintptr_t)(this));

    return S_OK;
}


/**
 *  Loads the object from the stream and requests a new pointer to
 *  the class we extended post-load.
 *
 *  @author: ZivDero, CCHyper, tomsons26
 */
HRESULT RocketTypeClass::Load(IStream* pStm)
{
    //EXT_DEBUG_TRACE("RocketTypeClass::Load - 0x%08X\n", (uintptr_t)(this));

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

    new (this) RocketTypeClass(NoInitClass());

    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(Type, "Type");
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(Warhead, "Warhead");
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(EliteWarhead, "EliteWarhead");
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(TakeoffAnim, "TakeoffAnim");
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(TrailAnim, "TrailAnim");

    return hr;
}


/**
 *  Saves the object to the stream.
 *
 *  @author: CCHyper, tomsons26
 */
HRESULT RocketTypeClass::Save(IStream* pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("RocketTypeClass::Internal_Save - 0x%08X\n", (uintptr_t)(this));

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
LONG RocketTypeClass::GetSizeMax(ULARGE_INTEGER* pcbSize)
{
    //EXT_DEBUG_TRACE("RocketTypeClass::GetSizeMax - 0x%08X\n", (uintptr_t)(this));

    if (!pcbSize) {
        return E_POINTER;
    }

    pcbSize->LowPart = sizeof(*this) + sizeof(uint32_t); // Add size of swizzle "id".
    pcbSize->HighPart = 0;

    return S_OK;
}


/**
 *  Retrieves the RocketType for given name.
 *
 *  @author: ZivDero
 */
RocketType RocketTypeClass::From_Name(const char *name)
{
    ASSERT(name != nullptr);

    if (name != nullptr) {
        for (RocketType Rocket = ROCKET_FIRST; Rocket < RocketTypes.Count(); Rocket++) {
            if (std::strncmp(RocketTypes[Rocket]->IniName, name, sizeof(IniName)) == 0) {
                return Rocket;
            }
        }
    }

    return ROCKET_NONE;
}


/**
 *  Returns name for given Rocket type.
 *
 *  @author: ZivDero
 */
const char *RocketTypeClass::Name_From(RocketType type)
{
    ASSERT(type >= ROCKET_FIRST && type < RocketTypes.Count());

    return RocketTypes[type]->IniName;
}


/**
 *  Find the rocket that has this aircraft as its type.
 *
 *  @author: ZivDero
 */
const RocketTypeClass* RocketTypeClass::From_AircraftType(const AircraftTypeClass* type)
{
    if (!type)
        return nullptr;

    for (RocketType rocket = ROCKET_FIRST; rocket < RocketTypes.Count(); rocket++) {
        if (RocketTypes[rocket]->Type == type) {
            return RocketTypes[rocket];
        }
    }

    return nullptr;
}


/**
 *  Find or create a Rocket of the type specified.
 *
 *  @author: ZivDero
 */
const RocketTypeClass *RocketTypeClass::Find_Or_Make(const char *name)
{
    ASSERT(name != nullptr);

    for (RocketType rocket = ROCKET_FIRST; rocket < RocketTypes.Count(); rocket++) {
        if (std::strncmp(RocketTypes[rocket]->IniName, name, sizeof(IniName)) == 0) {
            return RocketTypes[rocket];
        }
    }

    RocketTypeClass *ptr = new RocketTypeClass(name);
    ASSERT(ptr != nullptr);
    return ptr;
}


/**
 *  Reads Rocket object data from an INI file.
 *
 *  @author: ZivDero
 */
bool RocketTypeClass::Read_INI(CCINIClass& ini)
{
    if (!ini.Is_Present(IniName)) {
        return false;
    }

    PauseFrames = ini.Get_Int(IniName, "PauseFrames", PauseFrames);
    TiltFrames = ini.Get_Int(IniName, "TiltFrames", TiltFrames);
    PitchInitial = ini.Get_Double(IniName, "PitchInitial", PitchInitial);
    PitchFinal = ini.Get_Double(IniName, "PitchFinal", PitchFinal);
    TurnRate = ini.Get_Double(IniName, "TurnRate", TurnRate);
    RaiseRate = ini.Get_Int(IniName, "RaiseRate", RaiseRate);
    Acceleration = ini.Get_Double(IniName, "Acceleration", Acceleration);
    Altitude = ini.Get_Int(IniName, "Altitude", Altitude);
    Damage = ini.Get_Int(IniName, "Damage", Damage);
    EliteDamage = ini.Get_Int(IniName, "EliteDamage", EliteDamage);
    BodyLength = ini.Get_Int(IniName, "BodyLength", BodyLength);
    IsLazyCurve = ini.Get_Bool(IniName, "LazyCurve", IsLazyCurve);
    IsCruiseMissile = ini.Get_Bool(IniName, "CruiseMissile", IsCruiseMissile);
    CloseEnoughFactor = ini.Get_Double(IniName, "CloseEnoughFactor", CloseEnoughFactor);
    Type = ini.Get_Aircraft(IniName, "Type", Type);
    Warhead = ini.Get_Warhead(IniName, "Warhead", Warhead);
    EliteWarhead = ini.Get_Warhead(IniName, "EliteWarhead", EliteWarhead);
    TakeoffAnim = ini.Get_Anim(IniName, "TakeoffAnim", TakeoffAnim);
    TrailAnim = ini.Get_Anim(IniName, "TrailAnim", TrailAnim);
    TrailSpawnDelay = ini.Get_Int(IniName, "TrailSpawnDelay", TrailSpawnDelay);
    TrailAppearDelay = ini.Get_Int(IniName, "TrailAppearDelay", TrailAppearDelay);
    Inaccuracy = ini.Get_Int(IniName, "Inaccuracy", Inaccuracy);

    return true;
}
