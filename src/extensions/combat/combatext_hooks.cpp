/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          COMBATEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended combat functions.
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
#include "tibsun_inline.h"
#include "buildingext_hooks.h"
#include "combatext_hooks.h"
#include "vinifera_globals.h"
#include "combat.h"
#include "cell.h"
#include "overlaytype.h"
#include "rulesext.h"
#include "scenarioext.h"
#include "warheadtype.h"
#include "warheadtypeext.h"
#include "armortype.h"
#include "extension.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "verses.h"


/**
 *  Adjusts damage to reflect the nature of the target.
 *
 *  @author: 04/16/1994 JLB - Created.                                                 
 *           04/17/1994 JLB - Always does a minimum of damage.                         
 *           01/01/1995 JLB - Takes into account distance from damage source.          
 *           04/11/1996 JLB - Changed damage fall-off formula for less damage fall-off.
 *           ZivDero : Adjustments for Tiberian Sun
 */
int Vinifera_Modify_Damage(int damage, WarheadTypeClass* warhead, ArmorType armor, int distance)
{
    /**
     *	If there is no raw damage value to start with, then
     *	there can be no modified damage either.
     */
    if (!damage || Scen->SpecialFlags.IsInert || warhead == nullptr)
        return 0;

    /**
     *	Negative damage (i.e., heal) is always applied full strength, but only if the heal
     *	effect is close enough.
     */
    if (damage < 0)
    {
        if (distance < 0x008 && armor == ARMOR_NONE)
            return damage;

        return 0;
    }

    damage *= Verses::Get_Modifier(armor, warhead);

    /**
     *	Vanilla used to enforce a minimum of 1 damage here.
     */
#if 0
    if (damage <= 0)
        damage = 1;
#endif

    /**
     *	Reduce damage according to the distance from the impact point.
     */
    if (damage)
    {
        if (!warhead->SpreadFactor)
            distance /= PIXEL_LEPTON_W / 2;
        else
            distance /= warhead->SpreadFactor * (PIXEL_LEPTON_W / 2 + 1);

        distance = std::clamp(distance, 0, 16);

        if (distance)
            damage /= distance;

        /**
         *	Allow damage to drop to zero only if the distance would have
         *	reduced damage to less than 1/4 full damage. Otherwise, ensure
         *	that at least one damage point is done.
         */
        if (distance < 4)
            damage = std::max(damage, Rule->MinDamage);
    }

    damage = std::min(damage, Rule->MaxDamage);
    return damage;
}


/**
 *  convert a float to integer with the desired scale.
 * 
 *  @author CCHyper
 */
static int Scale_Float_To_Int(float value, int scale)
{
    value = std::clamp(value, 0.0f, 1.0f);
    return (value * scale);
}


/**
 *  #issue-410
 * 
 *  Implements IsWallAbsoluteDestroyer for WarheadTypes.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Explosion_Damage_IsWallAbsoluteDestroyer_Patch)
{
    GET_REGISTER_STATIC(OverlayTypeClass *, overlay, esi);
    GET_REGISTER_STATIC(const WarheadTypeClass *, warhead, ebx);
    GET_REGISTER_STATIC(CellClass *, cellptr, edi);
    GET_STACK_STATIC(int, strength, esp, 0x54);
    static const WarheadTypeClassExtension *warheadtypeext;

    /**
     *  Check to make sure that the warhead is of the kind that can destroy walls.
     */
    if (overlay->IsWall) {

        /**
         *  Is this warhead capable of instantly destroying the wall regardless
         *  of damage? If so, then pass -1 into Reduce_Wall to remove the wall
         *  section from the cell.
         */
        warheadtypeext = Extension::Fetch<WarheadTypeClassExtension>(warhead);
        if (warheadtypeext->IsWallAbsoluteDestroyer) {
            cellptr->Reduce_Wall(-1);

        /**
         *  Original check.
         */
        } else if (warhead->IsWallDestroyer || warhead->IsWoodDestroyer || overlay->Armor == ARMOR_WOOD) {
            cellptr->Reduce_Wall(strength);
        }

    }

    JMP_REG(ecx, 0x0045FAD0);
}


/**
 *  #issue-897
 *
 *  Implements IsIceDestruction scenario option for preventing destruction of ice,
 *  and IceStrength for configuring the chance for ice getting destroyed.
 *
 *  @author: Rampastring
 */
DECLARE_PATCH(_Explosion_Damage_IsIceDestruction_Patch)
{
    GET_REGISTER_STATIC(const WarheadTypeClass *, warhead, edi);
    GET_STACK_STATIC(int, strength, esp, 0x54);

    if (!ScenExtension->IsIceDestruction) {
        goto no_ice_destruction;
    }

    /**
     *  Stolen bytes/code here.
     */
    if (warhead->IsWallDestroyer || warhead->IsConventional) {

        /**
         *  Allow destroying ice if the strength of ice is 0 or the random number check allows it.
         */
        if (RuleExtension->IceStrength <= 0 || Random_Pick(0, RuleExtension->IceStrength) < strength) {
            goto allow_ice_destruction;
        }
    }

    /**
     *  Don't allow destroying ice, continue execution after ice-destruction logic.
     */
no_ice_destruction:
    JMP_REG(ecx, 0x004602DF);

    /**
     *  Allow destroying any potential ice on the cell.
     */
allow_ice_destruction:
    JMP_REG(ecx, 0x0046025C);
}


/**
 *  #issue-412
 * 
 *  Implements CombatLightSize for WarheadTypes.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Do_Flash_CombatLightSize_Patch)
{
    GET_REGISTER_STATIC(int, damage, ecx);
    GET_REGISTER_STATIC(const WarheadTypeClass *, warhead, edx);
    static const WarheadTypeClassExtension *warheadtypeext;
    static float light_size;
    static int flash_size;

    /**
     *  Fetch the extension instance.
     */
    warheadtypeext = Extension::Fetch<WarheadTypeClassExtension>(warhead);

    /**
     *  If no custom light size has been set, then just use the default code.
     *  This sets the size of the light based on the damage dealt by the Warhead.
     */
    if (warheadtypeext->CombatLightSize <= 0.0f) {

        /**
         *  Original code.
         */
        flash_size = (damage / 4);
        if (flash_size < 63) {
            if (flash_size <= 21) {
                flash_size = 21;
            }
        } else {
            flash_size = 63;
        }
    }


    /**
     *  Has a custom light size been set on the warhead?
     */
    if (warheadtypeext->CombatLightSize > 0.0f) {

        /**
         *  Clamp the light size and scale to expected size range.
         */
        light_size = warheadtypeext->CombatLightSize;
        if (light_size > 1.0f) {
            light_size = 1.0f;
        }
        flash_size = Scale_Float_To_Int(light_size, 63);
    }

    /**
     *  Set the desired flash size.
     */
    _asm { mov esi, flash_size }

    JMP(0x00460495);
}


/**
 *  Main function for patching the hooks.
 */
void CombatExtension_Hooks()
{
    Patch_Jump(0x0045FAA0, &_Explosion_Damage_IsWallAbsoluteDestroyer_Patch);
    Patch_Jump(0x00460244, &_Explosion_Damage_IsIceDestruction_Patch);
    Patch_Jump(0x00460477, &_Do_Flash_CombatLightSize_Patch);
    Patch_Jump(0x0045EB60, &Vinifera_Modify_Damage);
}
