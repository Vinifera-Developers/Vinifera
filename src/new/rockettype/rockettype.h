/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ROCKETTYPE.H
 *
 *  @authors       ZivDero
 *
 *  @brief         Class containing condiguration for AircraftType rockets.
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
#include "verses.h"
#include "wstring.h"

class CCINIClass;


enum RocketType
{
    ROCKET_FIRST = 0,
    ROCKET_NONE = -1
};
DEFINE_ENUMERATION_OPERATORS(RocketType);


class DECLSPEC_UUID(UUID_ROCKETTYPE)
RocketTypeClass final : IPersistStream
{
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

    RocketTypeClass();
    RocketTypeClass(const char *name);
    virtual ~RocketTypeClass();

    char const* Name() const { return IniName; }
    bool Read_INI(CCINIClass& ini);

    static bool One_Time();

    static RocketType From_Name(const char *name);
    static const char *Name_From(RocketType type);

    static const RocketTypeClass *From_AircraftType(const AircraftTypeClass *type);
    static const RocketTypeClass *Find_Or_Make(const char *name);

private:
    /**
     *  The name of this rocket type, used for identification purposes.
     */
    char IniName[256];

public:
    int PauseFrames;
    int TiltFrames;
    double PitchInitial;
    double PitchFinal;
    double TurnRate;
    int RaiseRate;
    double Acceleration;
    int Altitude;
    int Damage;
    int EliteDamage;
    int BodyLength;
    bool LazyCurve;
    const AircraftTypeClass* Type;
    const WarheadTypeClass* Warhead;
    const WarheadTypeClass* EliteWarhead;
    const AnimTypeClass* TakeoffAnim;
    const AnimTypeClass* TrailAnim;
    bool IsCruiseMissile;
};
