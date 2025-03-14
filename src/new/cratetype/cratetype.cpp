/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CRATETYPE.CPP
 *
 *  @authors       ZivDero
 *
 *  @brief         New CrateType class.
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
#include "cratetype.h"

#include "anim.h"
#include "animtype.h"
#include "ccini.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "asserthandler.h"
#include "cell.h"
#include "combat.h"
#include "coord.h"
#include "extension_globals.h"
#include "foot.h"
#include "house.h"
#include "housetype.h"
#include "housetypeext.h"
#include "ionstorm.h"
#include "mouse.h"
#include "rules.h"
#include "session.h"
#include "tibsun_inline.h"
#include "unit.h"
#include "vinifera_saveload.h"
#include "layer.h"
#include "logic.h"
#include "sidebarext.h"
#include "strtrim.h"
#include "super.h"
#include "supertype.h"
#include "technoext.h"
#include "voc.h"
#include "vox.h"
#include "warheadtype.h"


char const* CrateEffectNames[CRATE_EFFECT_COUNT] = {
    "Money",
    "Unit",
    "HealBase",
    "Cloak",
    "FragExplosion",
    "MiddleExplosion",
    "AreaExplosion",
    "Squad",
    "Darkness",
    "Reveal",
    "Armor",
    "Speed",
    "Firepower",
    "GrantSW",
    "FireSW",
    "Invulnerability",
    "Veteran",
    "IonStorm",
    "Tiberium",
};

const CrateTypeClass::CrateResult CrateTypeClass::CrateResult::GotoMoney = { true, false, true };

/**
  *  Basic constructor for crate objects.
  *
  *  @author: CCHyper
  */
CrateTypeClass::CrateTypeClass()
{
    CrateTypes.Add(this);
}


/**
 *  Basic constructor for crate objects.
 *
 *  @author: CCHyper
 */
CrateTypeClass::CrateTypeClass(const char* name) :
    IniName(""),
    Effect(CRATE_EFFECT_NONE),
    Weight(0),
    Anim(nullptr),
    Speed(SPEED_TRACK),
    Voice(VOX_NONE),
    IsMoney(false),
    HasSetDefaults(false)
{
    ASSERT_FATAL_PRINT(name != nullptr, "Invalid name for CrateType!");

    std::strncpy(IniName, name, sizeof(IniName));

    CrateTypes.Add(this);
}


/**
 *  Class destructor.
 *
 *  @author: CCHyper
 */
CrateTypeClass::~CrateTypeClass()
{
    CrateTypes.Delete(this);
}


/**
 *  Retrieves pointers to the supported interfaces on an object.
 *
 *  @author: CCHyper, tomsons26
 */
LONG CrateTypeClass::QueryInterface(REFIID riid, LPVOID* ppv)
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
ULONG CrateTypeClass::AddRef()
{
    //EXT_DEBUG_TRACE("CrateTypeClass::AddRef - 0x%08X\n", (uintptr_t)(this));

    return 1;
}


/**
 *  Decrements the reference count for an interface on a COM object.
 *
 *  @author: CCHyper
 */
ULONG CrateTypeClass::Release()
{
    //EXT_DEBUG_TRACE("CrateTypeClass::Release - 0x%08X\n", (uintptr_t)(this));

    return 1;
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *
 *  @author: CCHyper
 */
HRESULT CrateTypeClass::GetClassID(CLSID* lpClassID)
{
    //EXT_DEBUG_TRACE("CrateTypeClass::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(this));

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
HRESULT CrateTypeClass::IsDirty()
{
    //EXT_DEBUG_TRACE("CrateTypeClass::IsDirty - 0x%08X\n", (uintptr_t)(this));

    return S_OK;
}


/**
 *  Loads the object from the stream and requests a new pointer to
 *  the class we extended post-load.
 *
 *  @author: CCHyper, tomsons26
 */
HRESULT CrateTypeClass::Load(IStream* pStm)
{
    //EXT_DEBUG_TRACE("CrateTypeClass::Internal_Load - 0x%08X\n", (uintptr_t)(this));

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

    new (this) CrateTypeClass(NoInitClass());

    return hr;
}


/**
 *  Saves the object to the stream.
 *
 *  @author: CCHyper, tomsons26
 */
HRESULT CrateTypeClass::Save(IStream* pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("CrateTypeClass::Internal_Save - 0x%08X\n", (uintptr_t)(this));

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
LONG CrateTypeClass::GetSizeMax(ULARGE_INTEGER* pcbSize)
{
    //EXT_DEBUG_TRACE("CrateTypeClass::GetSizeMax - 0x%08X\n", (uintptr_t)(this));

    if (!pcbSize) {
        return E_POINTER;
    }

    pcbSize->LowPart = sizeof(*this) + sizeof(uint32_t); // Add size of swizzle "id".
    pcbSize->HighPart = 0;

    return S_OK;
}


/**
 *  Retrieves the CrateType for given name.
 *
 *  @author: CCHyper
 */
CrateType CrateTypeClass::From_Name(const char *name)
{
    ASSERT(name != nullptr);

    if (name != nullptr) {
        for (CrateType crate = CRATE_FIRST; crate < CrateTypes.Count(); crate++) {
            if (std::strncmp(CrateTypes[crate]->IniName, name, sizeof(IniName)) == 0) {
                return crate;
            }
        }
    }

    return CRATE_NONE;
}


/**
 *  Returns name for given crate type.
 *
 *  @author: CCHyper
 */
const char *CrateTypeClass::Name_From(CrateType type)
{
    ASSERT(type >= CRATE_FIRST && type < CrateTypes.Count());

    return CrateTypes[type]->IniName;
}


/**
 *  Find or create a crate of the type specified.
 *
 *  @author: CCHyper
 */
const CrateTypeClass *CrateTypeClass::Find_Or_Make(const char *name)
{
    ASSERT(name != nullptr);

    for (CrateType crate = CRATE_FIRST; crate < CrateTypes.Count(); ++crate) {
        if (std::strncmp(CrateTypes[crate]->IniName, name, sizeof(IniName)) == 0) {
            return CrateTypes[crate];
        }
    }

    CrateTypeClass *ptr = new CrateTypeClass(name);
    ASSERT(ptr != nullptr);
    return ptr;
}


/**
 *  Reads crate object data from an INI file.
 *
 *  @author: ZivDero
 */
bool CrateTypeClass::Read_INI(CCINIClass& ini)
{
    Set_Defaults();

    if (!ini.Is_Present(IniName)) {
        return false;
    }

    char buffer[256];
    if (ini.Get_String(IniName, "Effect", buffer, std::size(buffer))) {
        if (strcasecmp(buffer, "none") == 0 || strcasecmp(buffer, "<none>") == 0) {
            Effect = CRATE_EFFECT_NONE;
            IsMoney = false;
        }
        else {
            for (CrateEffectType effect = CRATE_EFFECT_FIRST; effect < CRATE_EFFECT_COUNT; effect++) {
                if (std::strcmp(buffer, CrateEffectNames[effect]) == 0) {
                    Effect = effect;
                    IsMoney = false;
                    break;
                }
            }
        }
    }

    Weight = ini.Get_Int(IniName, "Weight", Weight);
    Anim = ini.Get_Anim(IniName, "Anim", Anim);
    Speed = ini.Get_SpeedType(IniName, "Speed", Speed);
    Voice = ini.Get_VoxType(IniName, "Voice", Voice);

    switch (Effect)
    {
    case CRATE_EFFECT_MONEY:
        Data.Money.Amount = ini.Get_Int(IniName, "Amount", Data.Money.Amount);
        Data.Money.MaxExtra = ini.Get_Int(IniName, "MaxExtra", Data.Money.MaxExtra);
        break;

    case CRATE_EFFECT_UNIT:
        Data.Unit.Type = ini.Get_Unit(IniName, "Unit", Data.Unit.Type);
        Data.Unit.MaxUnits = ini.Get_Int(IniName, "MaxUnits", Data.Unit.MaxUnits);
        break;

    case CRATE_EFFECT_FRAG_EXPLOSION:
        Data.FragExplosion.Damage = ini.Get_Int(IniName, "Damage", Data.FragExplosion.Damage);
        Data.FragExplosion.Warhead = ini.Get_Warhead(IniName, "Warhead", Data.FragExplosion.Warhead);
        Data.FragExplosion.FragCount = ini.Get_Int(IniName, "FragCount", Data.FragExplosion.FragCount);
        Data.FragExplosion.FragRange = ini.Get_Int(IniName, "FragRange", Data.FragExplosion.FragRange);

    case CRATE_EFFECT_MIDDLE_EXPLOSION:
        Data.MiddleExplosion.Damage = ini.Get_Int(IniName, "Damage", Data.MiddleExplosion.Damage);
        Data.MiddleExplosion.Warhead = ini.Get_Warhead(IniName, "Warhead", Data.MiddleExplosion.Warhead);

    case CRATE_EFFECT_AREA_EXPLOSION:
        Data.AreaExplosion.Damage = ini.Get_Int(IniName, "Damage", Data.AreaExplosion.Damage);
        Data.AreaExplosion.Warhead = ini.Get_Warhead(IniName, "Warhead", Data.AreaExplosion.Warhead);

    case CRATE_EFFECT_SQUAD:
        Data.Squad.MaxInfantry = ini.Get_Int(IniName, "MaxInfantry", Data.Squad.MaxInfantry);
        break;

    case CRATE_EFFECT_ARMOR:
        Data.Armor.Multiplier = ini.Get_Float(IniName, "Multiplier", Data.Armor.Multiplier);
        Data.Armor.MaxStacking = ini.Get_Int(IniName, "MaxStacking", Data.Armor.MaxStacking);
        break;

    case CRATE_EFFECT_SPEED:
        Data.Speed.Multiplier = ini.Get_Float(IniName, "Multiplier", Data.Speed.Multiplier);
        Data.Speed.MaxStacking = ini.Get_Int(IniName, "MaxStacking", Data.Speed.MaxStacking);
        break;

    case CRATE_EFFECT_FIREPOWER:
        Data.Firepower.Multiplier = ini.Get_Float(IniName, "Multiplier", Data.Firepower.Multiplier);
        Data.Firepower.MaxStacking = ini.Get_Int(IniName, "MaxStacking", Data.Firepower.MaxStacking);
        break;

    case CRATE_EFFECT_GRANT_SW:
        Data.GrantSW.Type = ini.Get_SpecialWeaponType(IniName, "Type", Data.GrantSW.Type);
        Data.GrantSW.OneTime = ini.Get_Bool(IniName, "OneTime", Data.GrantSW.OneTime);
        break;

    case CRATE_EFFECT_VETERAN:
        Data.Veteran.Levels = ini.Get_Int(IniName, "Levels", Data.Veteran.Levels);
        break;

    case CRATE_EFFECT_ION_STORM:
        Data.IonStorm.Duration = ini.Get_Int(IniName, "Duration", Data.IonStorm.Duration);
        Data.IonStorm.Warning = ini.Get_Int(IniName, "Warning", Data.IonStorm.Warning);
        break;

    case CRATE_EFFECT_TIBERIUM:
        Data.Tiberium.AmountMinimum = ini.Get_Int(IniName, "AmountMinimum", Data.Tiberium.AmountMinimum);
        Data.Tiberium.AmountMaximum = ini.Get_Int(IniName, "AmountMaximum", Data.Tiberium.AmountMaximum);
        Data.Tiberium.Range = ini.Get_Int(IniName, "Range", Data.Tiberium.Range);
        break;

    default:
        break;
    }

    return true;
}


void CrateTypeClass::Init(CrateType type, char* powerup_string)
{
    switch (type)
    {
    case CRATE_MONEY:
        Effect = CRATE_EFFECT_MONEY;
        break;

    case CRATE_UNIT:
        Effect = CRATE_EFFECT_UNIT;
        break;

    case CRATE_HEAL_BASE:
        Effect = CRATE_EFFECT_HEAL_BASE;
        break;

    case CRATE_CLOAK:
        Effect = CRATE_EFFECT_CLOAK;
        break;

    case CRATE_EXPLOSION:
        Effect = CRATE_EFFECT_FRAG_EXPLOSION;
        break;

    case CRATE_NAPALM:
        Effect = CRATE_EFFECT_MIDDLE_EXPLOSION;
        break;

    case CRATE_SQUAD:
        Effect = CRATE_EFFECT_NONE;
        IsMoney = true;
        break;

    case CRATE_DARKNESS:
        Effect = CRATE_EFFECT_DARKNESS;
        break;

    case CRATE_REVEAL:
        Effect = CRATE_EFFECT_REVEAL;
        break;

    case CRATE_ARMOR:
        Effect = CRATE_EFFECT_ARMOR;
        Voice = VOX_UPGRADE_ARMOR;
        break;

    case CRATE_SPEED:
        Effect = CRATE_EFFECT_SPEED;
        Voice = VOX_UPGRADE_SPEED;
        break;

    case CRATE_FIREPOWER:
        Effect = CRATE_EFFECT_FIREPOWER;
        Voice = VOX_UPGRADE_FIREPOWER;
        break;

    case CRATE_ICBM:
        Effect = CRATE_EFFECT_GRANT_SW;
        break;

    case CRATE_INVULN:
        Effect = CRATE_EFFECT_NONE;
        break;

    case CRATE_VETERAN:
        Effect = CRATE_EFFECT_VETERAN;
        break;

    case CRATE_ION_STORM:
        Effect = CRATE_EFFECT_NONE;
        break;

    case CRATE_GAS:
        Effect = CRATE_EFFECT_AREA_EXPLOSION;
        break;

    case CRATE_TIBERIUM:
        Effect = CRATE_EFFECT_TIBERIUM;
        break;

    case CRATE_POD:
        Effect = CRATE_EFFECT_NONE;
        break;

    default:
        break;
    }

    Set_Defaults();

    char* token = std::strtok(powerup_string, ",");
    int weight = 0;
    if (token) weight = std::atoi(token);

    token = std::strtok(nullptr, ",");
    AnimType anim = ANIM_NONE;
    if (token) anim = AnimTypeClass::From_Name(token);

    token = std::strtok(nullptr, ",");
    double data = CrateData[type];
    if (token) {
        if (strchr(token, '%')) {
            data = std::atof(token) * 0.01;
        }
        else {
            strtrim(token);
            data = std::atof(token);
        }
    }

    Weight = weight;
    Anim = (anim != ANIM_NONE) ? AnimTypes[anim] : nullptr;

    switch (type)
    {
    case CRATE_MONEY:
        Data.Money.Amount = static_cast<int>(data);
        break;

    case CRATE_EXPLOSION:
        Data.FragExplosion.Damage = static_cast<int>(data);
        break;

    case CRATE_NAPALM:
        Data.MiddleExplosion.Damage = static_cast<int>(data);
        break;

    case CRATE_ARMOR:
        Data.Armor.Multiplier = data;
        break;

    case CRATE_SPEED:
        Data.Speed.Multiplier = data;
        break;

    case CRATE_FIREPOWER:
        Data.Firepower.Multiplier = data;
        break;

    case CRATE_VETERAN:
        Data.Veteran.Levels = static_cast<int>(data);
        break;

    case CRATE_GAS:
        Data.AreaExplosion.Damage = static_cast<int>(data);
        break;

    default:
        break;
    }
}


/**
 *  Performs one time initialization of the crate type class.
 *
 *  @warning: Do not change this function, otherwise it will break support
 *            with the original game!
 *
 *  @author: CCHyper
 */
bool CrateTypeClass::One_Time()
{
    /**
     *  Create the default crate types.
     */

    for (int i = CRATE_FIRST; i < CRATE_COUNT; i++) {
        const CrateTypeClass* crate = new CrateTypeClass((Wstring(CrateNames[i]) + "Crate").Peek_Buffer());
        ASSERT(crate != nullptr);
    }

    return true;
}


/**
 *  Loads the values from [Powerups] into crates to mimick vanilla behavior.
 *
 *  @author: ZivDero
 */
void CrateTypeClass::Init_From_Powerups(CCINIClass& ini)
{
    const char* POWERUPS = "Powerups";
    char buffer[128];

    if (ini.Is_Present(POWERUPS)) {
        for (CrateType i = CRATE_FIRST; i < CRATE_COUNT; i++) {
            if (ini.Get_String(POWERUPS, CrateNames[i], buffer, std::size(buffer))) {
                CrateTypes[From_Name((Wstring(CrateNames[i]) + "Crate").Peek_Buffer())]->Init(i, buffer);
            }
        }
    }
}


/**
 *  Executes a crate's effect.
 *
 *  @author: ZivDero
 */
bool CrateTypeClass::Execute(FootClass* object, CellClass& where, bool force_mcv)
{
    if (IsMoney) {
        return CrateTypes[From_Name("MoneyCrate")]->Execute(object, where, false);
    }

    CrateResult result = true;
    switch (Effect) {
        /**
         *  Give the player money.
         */
    case CRATE_EFFECT_MONEY:
        result = Do_Money(object, where);
        break;

         /**
          *  Shroud the world in blackness.
          */
    case CRATE_EFFECT_DARKNESS:
        result = Do_Darkness(object, where);
        break;

        /**
         *  Reveal the entire map.
         */
    case CRATE_EFFECT_REVEAL:
        result = Do_Reveal(object, where);
        break;

        /**
         *  Try to create a unit where the crate was.
         */
    case CRATE_EFFECT_UNIT:
        result = Do_Unit(object, where, force_mcv);
        break;

        /**
         *  Create a squad of miscellaneous composition.
         */
    case CRATE_EFFECT_SQUAD:
        result = Do_Squad(object, where);
        break;

         /**
          *  A group of explosions are triggered around the crate.
          */
    case CRATE_EFFECT_FRAG_EXPLOSION:
        result = Do_Frag_Explosion(object, where);
        break;

        /**
         *  A napalm blast is triggered.
         */
    case CRATE_EFFECT_MIDDLE_EXPLOSION:
        result = Do_Middle_Explosion(object, where);
        break;

        /**
         *  All objects within a certain range will gain the ability to cloak.
         */
    case CRATE_EFFECT_CLOAK:
        result = Do_Cloak(object, where);
        break;

        /**
         *  All of the player's objects heal up.
         */
    case CRATE_EFFECT_HEAL_BASE:
        result = Do_Heal_Base(object, where);
        break;


    case CRATE_EFFECT_GRANT_SW:
        result = Do_Grant_SW(object, where);
        break;

    case CRATE_EFFECT_ARMOR:
        result = Do_Armor(object, where);
        break;

    case CRATE_EFFECT_SPEED:
        result = Do_Speed(object, where);
        break;

    case CRATE_EFFECT_FIREPOWER:
        result = Do_Firepower(object, where);
        break;

    case CRATE_EFFECT_VETERAN:
        result = Do_Veteran(object, where);
        break;

    case CRATE_EFFECT_AREA_EXPLOSION:
        result = Do_Area_Explosion(object, where);
        break;

    case CRATE_EFFECT_TIBERIUM:
        result = Do_Tiberium(object, where);
        break;

    default:
        break;
    }

    if (result.DoMoney) {
        return CrateTypes[From_Name("MoneyCrate")]->Execute(object, where, false);
    }

    /**
     *  Generate any corresponding animation associated with this crate powerup.
     */
    if (Anim) {
        new AnimClass(Anim, where.Cell_Coord());
    }

    if (object->Owning_House()->Is_Player_Control())
    {
        if (result.ToSpeak) {
            Speak(Voice);
            VocClass::Play(Sound, object->Center_Coord());
        }
    }

    return result.CanMove;
}


/**
 *  Execute the Money crate effect.
 *
 *  @author: ZivDero
 */
CrateTypeClass::CrateResult CrateTypeClass::Do_Money(FootClass* object, CellClass& where) const
{
    DEBUG_INFO("Crate at %d,%d contains money\n", where.Pos.X, where.Pos.Y);

    int force_money = 0;

    /**
     *  Solo play has money amount determined by rules.ini file.
     */
    if (Session.Type == GAME_NORMAL) {
        force_money = Rule->SoloCrateMoney;
    }

    int money;
    if (force_money > 0) {
        money = force_money;
    }
    else {
        money = Random_Pick(Data.Money.Amount, Data.Money.Amount + Data.Money.MaxExtra);
    }

    if (object->House->Is_Player_Control() && Session.Type == GAME_NORMAL) {
        PlayerPtr->Refund_Money(money);
    }
    else {
        object->House->Refund_Money(money);
    }

    return true;
}


/**
 *  Execute the Unit crate effect.
 *
 *  @author: ZivDero
 */
CrateTypeClass::CrateResult CrateTypeClass::Do_Unit(FootClass* object, CellClass& where, bool force_mcv) const
{
    DEBUG_INFO("Crate at %d,%d contains a unit\n", where.Pos.X, where.Pos.Y);

    UnitTypeClass const* utp = nullptr;

    /**
     *  Give the player an MCV if he has no base left but does have more than enough
     *  money to rebuild a new base. Of course, if he already has an MCV, then don't
     *  give him another one.
     */
    if (force_mcv) {
        utp = Rule->BaseUnit;
    }

    /**
     *  If the player has a base and a refinery, but no harvester, then give him
     *  a free one.
     */
    if (utp == nullptr && (object->House->BQuantity.Count_Of(Rule->BuildRefinery[0]->Type)) && !(object->House->UQuantity.Count_Of(Rule->HarvesterUnit[0]->Type))) {
        utp = Rule->HarvesterUnit[0];
    }

    /**
     *  Check for special unit type override value.
     */
    if (Data.Unit.Type != nullptr) {
        utp = Data.Unit.Type;
    }
    else if (Rule->UnitCrateType != nullptr) {
        utp = Rule->UnitCrateType;
    }

    /**
     *  If no unit type has been determined, then pick one at random.
     */
    while (utp == nullptr) {
        UnitType utype = Random_Pick(UNIT_FIRST, static_cast<UnitType>(UnitTypes.Count() - 1));
        if (&UnitTypeClass::As_Reference(utype) != Rule->BaseUnit || Session.Options.Bases) {
            utp = &UnitTypeClass::As_Reference(utype);
            if (utp->IsCrateGoodie && (utp->Ownable & 1 << object->Owning_House()->ID)) {
                break;
            }
            utp = nullptr;
        }
    }

    if (utp != nullptr) {
        UnitClass* goodie_unit = reinterpret_cast<UnitClass*>(utp->Create_One_Of(object->House));
        if (goodie_unit != nullptr) {
            if (goodie_unit->Unlimbo(where.Cell_Coord())) {
                return false;
            }

            /**
             *  Try to place the object into a nearby cell if something is preventing
             *  placement at the crate location.
             */
            Cell cell = Map.Nearby_Location(where.Pos, goodie_unit->Class->Speed);
            if (goodie_unit->Unlimbo(Cell_Coord(cell))) {
                return false;
            }
            delete goodie_unit;
            return CrateResult::GotoMoney;
        }
    }

    return true;
}


/**
 *  Execute the Heal Base crate effect.
 *
 *  @author: ZivDero
 */
CrateTypeClass::CrateResult CrateTypeClass::Do_Heal_Base(FootClass* object, CellClass& where) const
{
    DEBUG_INFO("Crate at %d,%d contains base healing\n", where.Pos.X, where.Pos.Y);
    for (int index = 0; index < Logic.Count(); index++) {
        ObjectClass* obj = Logic[index];

        if (obj && object->Is_Techno() && object->House == obj->Owning_House()) {
            int healby = obj->Strength - obj->Class_Of()->MaxStrength;
            obj->Take_Damage(healby, 0, Rule->C4Warhead, nullptr, true, true);
        }
    }

    return true;
}


/**
 *  Execute the Cloak crate effect.
 *
 *  @author: ZivDero
 */
CrateTypeClass::CrateResult CrateTypeClass::Do_Cloak(FootClass* object, CellClass& where) const
{
    DEBUG_INFO("Crate at %d,%d contains cloaking device\n", where.Pos.X, where.Pos.Y);
    for (int index = 0; index < DisplayClass::Layer[LAYER_GROUND].Count(); index++) {
        ObjectClass* obj = DisplayClass::Layer[LAYER_GROUND][index];

        if (obj && obj->Is_Techno() && Distance(where.Cell_Coord(), obj->Center_Coord()) < Rule->CrateRadius) {
            static_cast<TechnoClass*>(obj)->IsCloakable = true;
        }
    }

    return true;
}


/**
 *  Execute the Frag Explosion crate effect.
 *
 *  @author: ZivDero
 */
CrateTypeClass::CrateResult CrateTypeClass::Do_Frag_Explosion(FootClass* object, CellClass& where) const
{
    DEBUG_INFO("Crate at %d,%d contains explosives\n", where.Pos.X, where.Pos.Y);

    int damage = Data.FragExplosion.Damage;
    const WarheadTypeClass* warhead = Data.FragExplosion.Warhead ? Data.FragExplosion.Warhead : Rule->C4Warhead;
    object->Take_Damage(damage, 0, warhead, nullptr, true);

    for (int index = 0; index < Data.FragExplosion.FragCount; index++) {
        Coordinate frag_coord = Coord_Scatter(where.Cell_Coord(), Random_Pick(0, Data.FragExplosion.FragRange));
        Explosion_Damage(frag_coord, damage, nullptr, warhead, true);
        new AnimClass(Combat_Anim(damage, warhead, LAND_CLEAR, &frag_coord), frag_coord, Get_Explosion_Z(Coordinate()));
    }

    return true;
}


/**
 *  Execute the Middle Explosion crate effect.
 *
 *  @author: ZivDero
 */
CrateTypeClass::CrateResult CrateTypeClass::Do_Middle_Explosion(FootClass* object, CellClass& where) const
{
    DEBUG_INFO("Crate at %d,%d contains napalm\n", where.Pos.X, where.Pos.Y);
    Coordinate coord = Coord_Mid(where.Cell_Coord(), object->Center_Coord());
    new AnimClass(AnimTypes[0], coord);
    int damage = Data.MiddleExplosion.Damage;
    const WarheadTypeClass* warhead = Data.MiddleExplosion.Warhead ? Data.MiddleExplosion.Warhead : Rule->FlameDamage;
    object->Take_Damage(damage, 0, warhead, nullptr, true);
    Explosion_Damage(coord, damage, nullptr, warhead, true);

    return true;
}


/**
 *  Execute the Area Explosion crate effect.
 *
 *  @author: ZivDero
 */
CrateTypeClass::CrateResult CrateTypeClass::Do_Area_Explosion(FootClass* object, CellClass& where) const
{
    DEBUG_INFO("Crate at %d,%d contains poison gas\n", where.Pos.X, where.Pos.Y);
    const WarheadTypeClass* warhead = Data.AreaExplosion.Warhead ? Data.AreaExplosion.Warhead : WarheadTypes[WarheadTypeClass::From_Name("GAS")];
    Explosion_Damage(where.Center_Coord(), Data.AreaExplosion.Damage, nullptr, warhead, false);
    for (FacingType facing = FACING_FIRST; facing < FACING_COUNT; facing++) {
        Explosion_Damage(where.Adjacent_Cell(facing).Center_Coord(), Data.AreaExplosion.Damage, nullptr, warhead, false);
    }

    return true;
}


/**
 *  Execute the Squad crate effect.
 *
 *  @author: ZivDero
 */
CrateTypeClass::CrateResult CrateTypeClass::Do_Squad(FootClass* object, CellClass& where) const
{
    HouseTypeClassExtension* owner_type_ext = Extension::Fetch<HouseTypeClassExtension>(object->Owning_House()->Class);

    for (int i = 0; i < owner_type_ext->SquadCrateInf.Count(); i++) {
        for (int j = 0; j < owner_type_ext->SquadCrateNum[i]; j++) {
            if (!owner_type_ext->SquadCrateInf[i]->Create_And_Place(where.Pos, object->Owning_House()) && i == 0 && j == 0) {
                return CrateResult::GotoMoney;
            }
        }
    }

    return false;
}


/**
 *  Execute the Darkness crate effect.
 *
 *  @author: ZivDero
 */
CrateTypeClass::CrateResult CrateTypeClass::Do_Darkness(FootClass* object, CellClass& where) const
{
    DEBUG_INFO("Crate at %d,%d contains 'shroud'\n", where.Pos.X, where.Pos.Y);
    if (object->House->Is_Player_Control()) {
        Map.Shroud_The_Map();
    }

    return true;
}


/**
 *  Execute the Reveal crate effect.
 *
 *  @author: ZivDero
 */
CrateTypeClass::CrateResult CrateTypeClass::Do_Reveal(FootClass* object, CellClass& where) const
{
    DEBUG_INFO("Crate at %d,%d contains 'reveal'\n", where.Pos.X, where.Pos.Y);
    if (object->House->Is_Human_Control()) {
        Map.Reveal_The_Map();
    }

    return true;
}


/**
 *  Execute the Armor crate effect.
 *
 *  @author: ZivDero
 */
CrateTypeClass::CrateResult CrateTypeClass::Do_Armor(FootClass* object, CellClass& where) const
{
    DEBUG_INFO("Crate at %d,%d contains armor\n", where.Pos.X, where.Pos.Y);

    bool tospeak = false;
    for (int index = 0; index < DisplayClass::Layer[LAYER_GROUND].Count(); index++) {
        ObjectClass* obj = DisplayClass::Layer[LAYER_GROUND][index];

        if (obj != nullptr && obj->Is_Techno() && Distance(where.Cell_Coord(), obj->Center_Coord()) < Rule->CrateRadius && Extension::Fetch<TechnoClassExtension>(obj)->ArmorCrates < Data.Armor.MaxStacking) {
            static_cast<TechnoClass*>(obj)->ArmorBias *= Data.Armor.Multiplier;
            Extension::Fetch<TechnoClassExtension>(obj)->ArmorCrates++;
            if (obj->Owning_House()->Is_Player_Control()) tospeak = true;
        }
    }

    return {true, tospeak, false};
}


/**
 *  Execute the Speed crate effect.
 *
 *  @author: ZivDero
 */
CrateTypeClass::CrateResult CrateTypeClass::Do_Speed(FootClass* object, CellClass& where) const
{
    DEBUG_INFO("Crate at %d,%d contains speed\n", where.Pos.X, where.Pos.Y);

    bool tospeak = false;
    for (int index = 0; index < DisplayClass::Layer[LAYER_GROUND].Count(); index++) {
        ObjectClass* obj = DisplayClass::Layer[LAYER_GROUND][index];

        if (obj && obj->Is_Foot() && Distance(where.Cell_Coord(), obj->Center_Coord()) < Rule->CrateRadius && Extension::Fetch<TechnoClassExtension>(obj)->SpeedCrates < Data.Speed.MaxStacking && obj->What_Am_I() != RTTI_AIRCRAFT) {
            FootClass* foot = static_cast<FootClass*>(obj);
            foot->SpeedBias *= Data.Speed.Multiplier;
            Extension::Fetch<TechnoClassExtension>(obj)->SpeedCrates++;
            if (obj->Owning_House()->Is_Player_Control()) tospeak = true;
        }
    }

    return { true, tospeak, false };
}


/**
 *  Execute the Firepower crate effect.
 *
 *  @author: ZivDero
 */
CrateTypeClass::CrateResult CrateTypeClass::Do_Firepower(FootClass* object, CellClass& where) const
{
    DEBUG_INFO("Crate at %d,%d contains firepower\n", where.Pos.X, where.Pos.Y);

    bool tospeak = false;
    for (int index = 0; index < DisplayClass::Layer[LAYER_GROUND].Count(); index++) {
        ObjectClass* obj = DisplayClass::Layer[LAYER_GROUND][index];

        if (obj && obj->Is_Techno() && Distance(where.Cell_Coord(), obj->Center_Coord()) < Rule->CrateRadius && Extension::Fetch<TechnoClassExtension>(obj)->FirepowerCrates < Data.Firepower.MaxStacking) {
            static_cast<TechnoClass*>(obj)->FirepowerBias *= Data.Firepower.Multiplier;
            Extension::Fetch<TechnoClassExtension>(obj)->FirepowerCrates++;
            if (obj->Owning_House()->Is_Player_Control()) tospeak = true;
        }
    }

    return { true, tospeak, false };
}


/**
 *  Execute the Grant Super Weapon crate effect.
 *
 *  @author: ZivDero
 */
CrateTypeClass::CrateResult CrateTypeClass::Do_Grant_SW(FootClass* object, CellClass& where) const
{
    DEBUG_INFO("Crate at %d,%d contains ICBM\n", where.Pos.X, where.Pos.Y);

    const SpecialWeaponType spc = Data.GrantSW.Type == SPECIAL_NONE ? SuperWeaponTypeClass::From_Action(ACTION_NUKE_BOMB)->Type : Data.GrantSW.Type;
    if (object->House->SuperWeapon[spc]->Enable(Data.GrantSW.OneTime) && object->IsOwnedByPlayer) {
        Map.Add(RTTI_SPECIAL, spc);
        SidebarExtension->Flag_Strip_To_Redraw(RTTI_SPECIAL);
    }

    return true;
}


/**
 *  Execute the Fire Super Weapon crate effect.
 *
 *  @author: ZivDero
 */
CrateTypeClass::CrateResult CrateTypeClass::Do_Fire_SW(FootClass* object, CellClass& where) const
{
    return true;
}


/**
 *  Execute the Invulnerability crate effect.
 *
 *  @author: ZivDero
 */
CrateTypeClass::CrateResult CrateTypeClass::Do_Invuln(FootClass* object, CellClass& where) const
{
    return true;
}


/**
 *  Execute the Veteran crate effect.
 *
 *  @author: ZivDero
 */
CrateTypeClass::CrateResult CrateTypeClass::Do_Veteran(FootClass* object, CellClass& where) const
{
    DEBUG_INFO("Crate at %d,%d contains veterancy(TM)\n", where.Pos.X, where.Pos.Y);

    bool tospeak = false;
    for (int index = 0; index < DisplayClass::Layer[LAYER_GROUND].Count(); index++) {
        ObjectClass* obj = DisplayClass::Layer[LAYER_GROUND][index];

        /**
         *  #issue-161
         *
         *  Veterancy crate bonus does not check if a object is un-trainable
         *  before granting it the veterancy bonus.
         *
         *  @author: CCHyper (based on research by Iran)
         */
        if (obj != nullptr && obj->Is_Techno() && object->IsDown && obj->Techno_Type_Class()->IsTrainable && Distance(where.Cell_Coord(), obj->Center_Coord()) < Rule->CrateRadius) {

            for (int i = 0; i < Data.Veteran.Levels; i++) {
                VeterancyClass& veterancy = static_cast<TechnoClass*>(obj)->Veterancy;
                if (veterancy.Is_Veteran()) {
                    veterancy.Set_Elite(true);
                }
                if (veterancy.Is_Rookie()) {
                    veterancy.Set_Veteran(true);
                }
                if (veterancy.Is_Dumbass()) {
                    veterancy.Set_Rookie(true);
                }
            }
            if (obj->Owning_House()->Is_Player_Control()) tospeak = true;
        }
    }

    return {true, tospeak, false};
}


/**
 *  Execute the Ion Storm crate effect.
 *
 *  @author: ZivDero
 */
CrateTypeClass::CrateResult CrateTypeClass::Do_Ion_Storm(FootClass* object, CellClass& where) const
{
    if (!IonStorm_Is_Active()) {
        IonStorm_Start(Data.IonStorm.Duration * TICKS_PER_SECOND, Data.IonStorm.Warning * TICKS_PER_SECOND);
    }

    return true;
}


/**
 *  Execute the Tiberium crate effect.
 *
 *  @author: ZivDero
 */
CrateTypeClass::CrateResult CrateTypeClass::Do_Tiberium(FootClass* object, CellClass& where) const
{
    DEBUG_INFO("Crate at %d,%d contains tiberium\n", where.Pos.X, where.Pos.Y);
    const TiberiumType tib = static_cast<TiberiumType>(Random_Pick(0, Tiberiums.Count() - 1));
    int amount = Random_Pick(Data.Tiberium.AmountMaximum, Data.Tiberium.AmountMaximum);
    while (amount) {
        Coordinate frag_coord = Coord_Scatter(where.Center_Coord(), Random_Pick(0, Data.Tiberium.Range), true);
        Map[frag_coord].Place_Tiberium(tib, 1);
        amount--;
    }

    return true;
}


void CrateTypeClass::Set_Defaults()
{
    if (!HasSetDefaults) {
        switch (Effect)
        {
        case CRATE_EFFECT_MONEY:
            Data.Money.Amount = 0;
            Data.Money.MaxExtra = 900;
            break;

        case CRATE_EFFECT_UNIT:
            Data.Unit.Type = nullptr;
            Data.Unit.MaxUnits = 50;
            break;

        case CRATE_EFFECT_FRAG_EXPLOSION:
            Data.FragExplosion.Damage = 0;
            Data.FragExplosion.Warhead = nullptr;
            Data.FragExplosion.FragCount = 5;
            Data.FragExplosion.FragRange = CELL_LEPTON_W * 2;

        case CRATE_EFFECT_MIDDLE_EXPLOSION:
            Data.MiddleExplosion.Damage = 0;
            Data.MiddleExplosion.Warhead = nullptr;

        case CRATE_EFFECT_AREA_EXPLOSION:
            Data.AreaExplosion.Damage = 0;
            Data.AreaExplosion.Warhead = nullptr;

        case CRATE_EFFECT_SQUAD:
            Data.Squad.MaxInfantry = 100;
            break;

        case CRATE_EFFECT_ARMOR:
            Data.Armor.Multiplier = 1.0;
            Data.Armor.MaxStacking = 1;
            break;

        case CRATE_EFFECT_SPEED:
            Data.Speed.Multiplier = 1.0;
            Data.Speed.MaxStacking = 1;
            break;

        case CRATE_EFFECT_FIREPOWER:
            Data.Firepower.Multiplier = 1.0;
            Data.Firepower.MaxStacking = 1;
            break;

        case CRATE_EFFECT_GRANT_SW:
            Data.GrantSW.Type = SPECIAL_NONE;
            Data.GrantSW.OneTime = true;
            break;

        case CRATE_EFFECT_VETERAN:
            Data.Veteran.Levels = 1;
            break;

        case CRATE_EFFECT_ION_STORM:
            Data.IonStorm.Duration = Rule->IonStormDuration;
            Data.IonStorm.Warning = Rule->IonStormWarning;
            break;

        case CRATE_EFFECT_TIBERIUM:
            Data.Tiberium.AmountMinimum = 10;
            Data.Tiberium.AmountMaximum = 20;
            Data.Tiberium.Range = CELL_LEPTON_W * 3;
            break;

        default:
            break;
        }

        HasSetDefaults = true;
    }
}

