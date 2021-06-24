/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          WAVEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended WaveClass.
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
#include "waveext_hooks.h"
#include "waveext_init.h"
#include "wave.h"
#include "waveext.h"
#include "weapontype.h"
#include "weapontypeext.h"
#include "dsurface.h"
#include "tibsun_globals.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or deconstructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class WaveClassFake : public WaveClass
{
    public:
        void Draw_Sonic_Beam_Pixel_Intercept(int a1, int a2, int a3, unsigned short *buffer)
        {
            WaveClassExtension *waveext;
            waveext = WaveClassExtensions.find(this);
            if (waveext) {
                waveext->Draw_Sonic_Beam_Pixel(a1, a2, a3, buffer);
            } else {
                func_670370(a1, a2, a3, buffer);
            }
        }

        bool Generate_Tables_Intercept()
        {
            WaveClassExtension *waveext;
            waveext = WaveClassExtensions.find(this);
            if (waveext) {
                waveext->Calculate_Sonic_Beam_Tables();
            } else {
                func_670580();
            }
        }
};


/**
 *  Patch in the new Calculate_Sonic_Beam_Tables function.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_WaveClass_Default_Constructor_Calculate_Sonic_Beam_Tables_Patch)
{
    GET_REGISTER_STATIC(WaveClass *, this_ptr, esi);

    WaveClassExtension *waveext;
    waveext = WaveClassExtensions.find(this_ptr);
    if (waveext) {
        waveext->Calculate_Sonic_Beam_Tables();
    } else {
        WaveClass::func_670580();
    }

    JMP(0x006702B2);
}

DECLARE_PATCH(_WaveClass_Constructor_Calculate_Sonic_Beam_Tables_Patch)
{
    GET_REGISTER_STATIC(WaveClass *, this_ptr, esi);

    WaveClassExtension *waveext;
    waveext = WaveClassExtensions.find(this_ptr);
    if (waveext) {
        waveext->Calculate_Sonic_Beam_Tables();
    } else {
        WaveClass::func_670580();
    }

    JMP(0x00670007);
}


/**
 *  Setup the wave size vectors. This replaces the existing one-time
 *  code in the game and is now done on each wave creation.
 * 
 *  @author: CCHyper
 */
static void Wave_Setup_Size_Vectors(WaveClass *this_ptr)
{
    bool use_sonic_beam_defaults = true;

    WaveClassExtension *waveext;
    waveext = WaveClassExtensions.find(this_ptr);
    if (waveext) {

        /**
         *  Copy over this waves custom sonic beam values.
         */
        Wave_SizeVectors[WAVE_SONIC][0] = waveext->SonicBeamEndPinLeft;
        Wave_SizeVectors[WAVE_SONIC][1] = waveext->SonicBeamEndPinRight;
        Wave_SizeVectors[WAVE_SONIC][2] = waveext->SonicBeamStartPinLeft;
        Wave_SizeVectors[WAVE_SONIC][3] = waveext->SonicBeamStartPinRight;

        use_sonic_beam_defaults = false;
    }

    if (use_sonic_beam_defaults) {
        /**
         *  Original sonic beam values.
         */
        Wave_SizeVectors[WAVE_SONIC][0] = Vector3(-30.0, -100.0, 0.0);
        Wave_SizeVectors[WAVE_SONIC][1] = Vector3(-30.0, 100.0, 0.0);
        Wave_SizeVectors[WAVE_SONIC][2] = Vector3(30.0, -100.0, 0.0);
        Wave_SizeVectors[WAVE_SONIC][3] = Vector3(30.0, 100.0, 0.0);
    }

    /**
     *  Original game values.
     */
    Wave_SizeVectors[WAVE_BIG_LASER][0] = Vector3(-34.0, -44.0, 0.0);
    Wave_SizeVectors[WAVE_BIG_LASER][1] = Vector3(-34.0, 44.0, 0.0);
    Wave_SizeVectors[WAVE_BIG_LASER][2] = Vector3(34.0, -44.0, 0.0);
    Wave_SizeVectors[WAVE_BIG_LASER][3] = Vector3(34.0, 44.0, 0.0);

    Wave_SizeVectors[WAVE_LASER][0] = Vector3(-27.0, -34.0, 0.0);
    Wave_SizeVectors[WAVE_LASER][1] = Vector3(-27.0, 34.0, 0.0);
    Wave_SizeVectors[WAVE_LASER][2] = Vector3(27.0, -34.0, 0.0);
    Wave_SizeVectors[WAVE_LASER][3] = Vector3(27.0, 34.0, 0.0);
}


/**
 *  Patch in the new Generate_Tables.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_WaveClass_Setup_Wave_Size)
{
    GET_REGISTER_STATIC(WaveClass *, this_ptr, edi);

    Wave_Setup_Size_Vectors(this_ptr);

    JMP(0x00672414);
}


/**
 *  These patches allow us to store the firing weapon type pointer
 *  to allow us to fetch custom overrides. Nasty hack...
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Laser_Zap_Store_WeaponType_Ptr)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, edi);
    GET_REGISTER_STATIC(WeaponTypeClass *, weapontype, esi);

    Wave_TempWeaponTypePtr = weapontype;

    /**
     *  Stolen bytes/code.
     */
    _asm { mov dl, [esi+0x0E3] } // WeaponTypeClass.IsBigLaser

    JMP_REG(ecx, 0x006301DE);
}

DECLARE_PATCH(_TechnoClass_Fire_At_Store_WeaponType_Ptr)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    GET_REGISTER_STATIC(WeaponTypeClass *, weapontype, ebx);

    Wave_TempWeaponTypePtr = weapontype;

    /**
     *  Stolen bytes/code.
     */
    _asm { mov ecx, [ebp+8] }
    _asm { mov eax, [esi] }

    JMP_REG(edx, 0x006311B5);
}


/**
 *  Main function for patching the hooks.
 */
void WaveClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    WaveClassExtension_Init();

    Patch_Jump(0x006701E0, &_WaveClass_Default_Constructor_Calculate_Sonic_Beam_Tables_Patch);
    Patch_Jump(0x0066FF23, &_WaveClass_Constructor_Calculate_Sonic_Beam_Tables_Patch);

    Patch_Jump(0x00670370, &WaveClassFake::Draw_Sonic_Beam_Pixel_Intercept);

    /**
     *  Removes the one-time initialisation of the wave size vectors
     *  and replaces it with our own.
     */
    Patch_Jump(0x00672171, &_WaveClass_Setup_Wave_Size);

    /**
     *  Ulgy workaround to getting the WeaponType pointer for WaveClassExtension.
     */
    Patch_Jump(0x006301D8, &_TechnoClass_Laser_Zap_Store_WeaponType_Ptr);
    Patch_Jump(0x006311B0, &_TechnoClass_Fire_At_Store_WeaponType_Ptr);
}
