/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CRATETYPE.H
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
#pragma once 

#include "always.h"
#include "tibsun_defines.h"
#include "vinifera_defines.h"
#include "wstring.h"
#include "objidl.h"
#include "noinit.h"
#include "typelist.h"

class FootClass;
class CellClass;
class SuperWeaponTypeClass;
class InfantryTypeClass;
class WarheadTypeClass;
class UnitTypeClass;
class AnimTypeClass;
class CCINIClass;


enum CrateEffectType
{
    CRATE_EFFECT_MONEY,
    CRATE_EFFECT_UNIT,
    CRATE_EFFECT_HEAL_BASE,
    CRATE_EFFECT_CLOAK,
    CRATE_EFFECT_FRAG_EXPLOSION,
    CRATE_EFFECT_MIDDLE_EXPLOSION,
    CRATE_EFFECT_AREA_EXPLOSION,
    CRATE_EFFECT_SQUAD,
    CRATE_EFFECT_DARKNESS,
    CRATE_EFFECT_REVEAL,
    CRATE_EFFECT_ARMOR,
    CRATE_EFFECT_SPEED,
    CRATE_EFFECT_FIREPOWER,
    CRATE_EFFECT_GRANT_SW,
    CRATE_EFFECT_FIRE_SW,
    CRATE_EFFECT_INVULNERABILITY,
    CRATE_EFFECT_VETERAN,
    CRATE_EFFECT_ION_STORM,
    CRATE_EFFECT_TIBERIUM,

    CRATE_EFFECT_COUNT,
    CRATE_EFFECT_FIRST = 0,

    CRATE_EFFECT_NONE = -1,
};
DEFINE_ENUMERATION_OPERATORS(CrateEffectType);

extern char const* CrateEffectNames[CRATE_EFFECT_COUNT];

class DECLSPEC_UUID(UUID_CRATETYPE)
CrateTypeClass final : IPersistStream
{
private:
    struct CrateResult
    {
        static const CrateResult GotoMoney;

        CrateResult(bool can_move) : CanMove(can_move), ToSpeak(true), DoMoney(false) {}
        CrateResult(bool can_move, bool to_speak, bool do_money) : CanMove(can_move), ToSpeak(to_speak), DoMoney(do_money) {}

        bool CanMove;
        bool ToSpeak;
        bool DoMoney;
    };

public:
    /**
     *  IUnknown
     */
    IFACEMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);
    IFACEMETHOD_(ULONG, AddRef)();
    IFACEMETHOD_(ULONG, Release)();

    /**
     *  IPersist
     */
    IFACEMETHOD(GetClassID)(CLSID* pClassID);

    /**
     *  IPersistStream
     */
    IFACEMETHOD(IsDirty)();
    IFACEMETHOD(Load)(IStream* pStm);
    IFACEMETHOD(Save)(IStream* pStm, BOOL fClearDirty);
    IFACEMETHOD_(LONG, GetSizeMax)(ULARGE_INTEGER* pcbSize);

    CrateTypeClass();
    CrateTypeClass(const NoInitClass& noinit) {}
    CrateTypeClass(const char *name);
    virtual ~CrateTypeClass();

    char const* Name() const { return IniName; }
    bool Read_INI(CCINIClass& ini);
    void Init(CrateType type, char* powerup_string);
    
    static void Init_From_Powerups(CCINIClass& ini);

    bool Execute(FootClass* object, CellClass& where, bool force_mcv);

    static bool One_Time();

    static CrateType From_Name(const char *name);
    static const char *Name_From(CrateType type);

    static const CrateTypeClass *Find_Or_Make(const char *name);

private:
    CrateResult Do_Money(FootClass* object, CellClass& where) const;
    CrateResult Do_Unit(FootClass* object, CellClass& where, bool force_mcv) const;
    CrateResult Do_Heal_Base(FootClass* object, CellClass& where) const;
    CrateResult Do_Cloak(FootClass* object, CellClass& where) const;
    CrateResult Do_Frag_Explosion(FootClass* object, CellClass& where) const;
    CrateResult Do_Middle_Explosion(FootClass* object, CellClass& where) const;
    CrateResult Do_Area_Explosion(FootClass* object, CellClass& where) const;
    CrateResult Do_Squad(FootClass* object, CellClass& where) const;
    CrateResult Do_Darkness(FootClass* object, CellClass& where) const;
    CrateResult Do_Reveal(FootClass* object, CellClass& where) const;
    CrateResult Do_Armor(FootClass* object, CellClass& where) const;
    CrateResult Do_Speed(FootClass* object, CellClass& where) const;
    CrateResult Do_Firepower(FootClass* object, CellClass& where) const;
    CrateResult Do_Grant_SW(FootClass* object, CellClass& where) const;
    CrateResult Do_Fire_SW(FootClass* object, CellClass& where) const;
    CrateResult Do_Invuln(FootClass* object, CellClass& where) const;
    CrateResult Do_Veteran(FootClass* object, CellClass& where) const;
    CrateResult Do_Ion_Storm(FootClass* object, CellClass& where) const;
    CrateResult Do_Tiberium(FootClass* object, CellClass& where) const;

    void Set_Defaults();

private:
    /**
     *  The name of this crate type, used for identification purposes.
     */
    char IniName[256];

public:

    CrateEffectType Effect;
    int Weight;
    const AnimTypeClass* Anim;
    SpeedType Speed;
    VoxType Voice;
    VocType Sound;

    union
    {
        struct {
            int Amount;
            int MaxExtra; //900
        } Money;

        struct {
            const UnitTypeClass* Type; //Rule->UnitCrateType
            int MaxUnits; // 50
        } Unit;

        struct {
            int Damage;
            const WarheadTypeClass* Warhead; // C4Warhead
            int FragCount; // 5
            int FragRange; // 512
        } FragExplosion;

        struct {
            int Damage;
            const WarheadTypeClass* Warhead; // Rule->FlameDamage
        } MiddleExplosion;

        struct {
            int Damage;
            const WarheadTypeClass* Warhead; // "GAS"
        } AreaExplosion;

        struct {
            int MaxInfantry; // 100
        } Squad;

        struct {
            double Multiplier;
            int MaxStacking; // 1
        } Armor;

        struct {
            double Multiplier;
            int MaxStacking; // 1
        } Speed;

        struct {
            double Multiplier;
            int MaxStacking; // 1
        } Firepower;

        struct {
            SpecialWeaponType Type;
            bool OneTime;
        } GrantSW;

        struct {
            int Levels;
        } Veteran;

        struct {
            int Duration;
            int Warning;
        } IonStorm;

        struct {
            int AmountMinimum; // 10
            int AmountMaximum; // 20
            int Range; // CELL_LEPTON_W * 3
        } Tiberium;

    } Data;

private:
    bool IsMoney;
    bool HasSetDefaults;

};
