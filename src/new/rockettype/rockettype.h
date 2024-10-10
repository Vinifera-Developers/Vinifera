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
 *  @brief         Class containing condiguration for spawned rockets.
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
    /**
     *  How many frames the rocket pauses on the launcher before tilting?
     */
    int PauseFrames;

    /**
     *  How many frames it takes for the rocket to tilt to firing position?
     */
    int TiltFrames;

    /**
     *  Starting pitch of the rocket before tilting up (0 = horizontal, 1 = vertical).
     */
    double PitchInitial;

    /**
     *  Ending pitch of the rocket after tilting up, now it fires.
     */
    double PitchFinal;

    /**
     *  Pitch maneuverability of rocket in air.
     */
    double TurnRate;

    /**
     *  How much the missile will raise each turn on the launcher (for Cruise Missile only)
     */
    LEPTON RaiseRate;

    /**
     *  This much is added to the rocket's velocity each frame during launch.
     */
    double Acceleration;

    /**
     *  Cruising altitude in leptons: at this height rocket BEGINS leveling off.
     */
    int Altitude;

    /**
     *  The rocket does this much damage when it explodes.
     */
    int Damage;
    int EliteDamage;

    /**
     *  The body of the rocket is this many leptons long.
     */
    LEPTON BodyLength;

    /**
     *  The rocket's path is a big, lazy curve, like the V3 in Red Alert 2.
     */
    bool IsLazyCurve;

    /**
     *  The rocket is a cruise missile and instead of tilting, rises a bit before shooting off vertically, like the Boomer sub missiles in Red Alert 2.
     */
    bool IsCruiseMissile;

    /**
     *  The AircraftType of this rocket.
     */
    const AircraftTypeClass* Type;

    /**
     *  The warhead used when this rocket explodes.
     */
    const WarheadTypeClass* Warhead;
    const WarheadTypeClass* EliteWarhead;

    /**
     *  Takeoff and trail animations that this rocket uses.
     */
    const AnimTypeClass* TakeoffAnim;
    const AnimTypeClass* TrailAnim;
};
