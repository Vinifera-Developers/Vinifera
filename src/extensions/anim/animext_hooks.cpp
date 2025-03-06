/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ANIMEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended AnimClass.
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
#include "animext_hooks.h"
#include "tibsun_globals.h"
#include "tibsun_inline.h"
#include "anim.h"
#include "animext.h"
#include "animext_init.h"
#include "animtype.h"
#include "animtypeext.h"
#include "smudgetype.h"
#include "particle.h"
#include "particletype.h"
#include "particlesys.h"
#include "target.h"
#include "cell.h"
#include "rules.h"
#include "scenario.h"
#include "extension.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "combat.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class AnimClassExt final : public AnimClass
{
    public:
        LayerType _In_Which_Layer() const;
};


/**
 *  Reimplementation of AnimClass::In_Which_Layer.
 * 
 *  @author: CCHyper
 */
LayerType AnimClassExt::_In_Which_Layer() const
{
    if (Target_Legal(xObject)) {
        return LAYER_GROUND;
    }

    /**
     *  #issue-564
     * 
     *  Implements layer override for anims.
     * 
     *  @author: CCHyper
     */
    AnimTypeClassExtension *animtypeext = nullptr;
    animtypeext = Extension::Fetch<AnimTypeClassExtension>(Class);
    if (animtypeext->AttachLayer != LAYER_NONE) {
        return animtypeext->AttachLayer;
    }

    if (Class && Class->IsGroundLayer) {
        return LAYER_GROUND;
    }

    return LAYER_AIR;
}


/**
 *  Get the center coord of a anim. This is required as we can not allocate
 *  on the stack and Center_Coord returns Coordinate value.
 * 
 *  @author: CCHyper
 */
static Coordinate &Anim_Get_Center_Coord(AnimClass *this_ptr)
{
    return this_ptr->Center_Coord();
}


/**
 *  Spawn the attached particle systems. This is required as we can not
 *  allocate on the stack.
 * 
 *  @author: CCHyper
 */
static void Anim_Spawn_Particles(AnimClass *this_ptr)
{
    AnimTypeClassExtension *animtypeext;

    animtypeext = Extension::Fetch<AnimTypeClassExtension>(this_ptr->Class);
    if (animtypeext->ParticleToSpawn != PARTICLE_NONE) {

        for (int i = 0; i < animtypeext->NumberOfParticles; ++i) {

            Coordinate spawn_coord = this_ptr->Coord;

            /**
             *  Spawn a new particle at this anims coord.
             */
            MasterParticle->Spawn_Particle(
                (ParticleTypeClass *)ParticleTypeClass::As_Pointer(animtypeext->ParticleToSpawn),
                spawn_coord);

        }
    }
}


/**
 *  Calls the AnimClass extension start event processor.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AnimClass_Start_Ext_Patch)
{
    GET_REGISTER_STATIC(AnimClass *, this_ptr, esi);
    static AnimClassExtension *animext;

    animext = Extension::Fetch<AnimClassExtension>(this_ptr);

    animext->Start();
    
original_code:
    _asm { pop esi }
    _asm { pop ebx }
    _asm { add esp, 0x24 }
    _asm { retn }
}


/**
 *  Calls the AnimClass extension middle event processor.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AnimClass_Middle_Ext_Patch)
{
    GET_REGISTER_STATIC(AnimClass *, this_ptr, esi);
    static AnimClassExtension *animext;

    animext = Extension::Fetch<AnimClassExtension>(this_ptr);

    animext->Middle();
    
original_code:
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { add esp, 0x38 }
    _asm { retn }
}


/**
 *  Calls the AnimClass extension end event processor.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AnimClass_AI_End_Ext_Patch)
{
    GET_REGISTER_STATIC(AnimClass *, this_ptr, esi);
    static AnimClassExtension *animext;

    animext = Extension::Fetch<AnimClassExtension>(this_ptr);

    animext->End();
    
original_code:
    /**
     *  Restore expected register states.
     */
    _asm { mov esi, this_ptr }
    _asm { xor ebp, ebp}

    /**
     *  Stolen bytes/code.
     */
    _asm { mov edx, [esi+0x64] } // this->Class
    _asm { mov ecx, [edx+0x154] } // Class->ChainTo

    JMP_REG(edx, 0x00415B03);
}


/**
 *  #issue-568
 * 
 *  Implements SpawnParticle for anims.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AnimClass_Middle_SpawnParticle_Patch)
{
    GET_REGISTER_STATIC(AnimClass *, this_ptr, esi);
    static AnimTypeClassExtension *animtypeext;

    /**
     *  Spawn a new particle at this anims coord if one is set.
     */
    Anim_Spawn_Particles(this_ptr);

    /**
     *  Stolen bytes/code.
     */
    static int height;
    height = this_ptr->Get_Height();
    _asm { mov eax, [height] }

    JMP_REG(ecx, 0x00416076);
}


/**
 *  #issue-562
 * 
 *  Handles top level layer sorting for the new "AttachLayer" key.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AnimClass_Constructor_Layer_Set_Z_Height_Patch)
{
    GET_REGISTER_STATIC(AnimClass *, this_ptr, esi);
    static AnimTypeClassExtension *animtypeext;
    
    animtypeext = Extension::Fetch<AnimTypeClassExtension>(this_ptr->Class);

    /**
     *  Set the layer to the highest level if "air" or "top".
     */
    if (animtypeext->AttachLayer != LAYER_NONE
        && (animtypeext->AttachLayer == LAYER_AIR || animtypeext->AttachLayer == LAYER_TOP)) {
        this_ptr->Set_Z_Coord(Rule->FlightLevel);

    /**
     *  Original code.
     */
    } else if (!this_ptr->Class->IsGroundLayer) {
        this_ptr->Set_Z_Coord(Rule->FlightLevel);

    } else {
        this_ptr->Set_Height(0);
    }

    JMP(0x00413D63);
}


/**
 *  #issue-562
 * 
 *  Implements IsForceBigCraters to Anims.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AnimClass_Middle_Create_Crater_ForceBigCraters_Patch)
{
    GET_REGISTER_STATIC(AnimClass *, this_ptr, esi);
    GET_REGISTER_STATIC(int, width, ebp);
    GET_REGISTER_STATIC(int, height, edi);
    static AnimTypeClassExtension *animtypeext;
    static Coordinate *tmpcoord;
    static Coordinate coord;

    tmpcoord = &Anim_Get_Center_Coord(this_ptr);
    coord.X = tmpcoord->X;
    coord.Y = tmpcoord->Y;
    coord.Z = tmpcoord->Z;

    animtypeext = Extension::Fetch<AnimTypeClassExtension>(this_ptr->Class);

    /**
     *  Is this anim is to spawn big craters?
     */
    if (animtypeext->IsForceBigCraters) {
        SmudgeTypeClass::Create_Crater(coord, 300, 300, true);
    } else {
        SmudgeTypeClass::Create_Crater(coord, width, height, false);
    }

    JMP(0x00416113);
}


/**
 *  Patch to intercept the start of the AnimClass::AI function.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AnimClass_AI_Beginning_Patch)
{
    GET_REGISTER_STATIC(AnimClass *, this_ptr, esi);
    static AnimTypeClass *animtype;
    static AnimTypeClassExtension *animtypeext;
    static CellClass *cell;

    animtype = this_ptr->Class;
    animtypeext = Extension::Fetch<AnimTypeClassExtension>(animtype);

    /**
     *  We have flagged that this animation's type needs to have
     *  its biggest frame recalculated, do that.
     */
    if (animtype->Biggest == -1) {
        animtypeext->Set_Biggest_Frame();
    }

    /**
     *  Stolen bytes/code.
     */
    if (animtype->IsFlamingGuy) {
        this_ptr->Flaming_Guy_AI();
        this_ptr->ObjectClass::AI();
    }

    cell = this_ptr->Get_Cell_Ptr();
    
    /**
     *  #issue-560
     * 
     *  Implements IsHideIfNotTiberium for Anims.
     * 
     *  @author: CCHyper
     */
    if (animtypeext->IsHideIfNotTiberium) {
    
        if (!cell || !cell->Get_Tiberium_Value()) {
            this_ptr->IsInvisible = true;
        }
    
    }

    JMP_REG(edx, 0x00414EAA);
}


#if 0
/**
 *  Patch for setting initial anim values in the class constructor.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AnimClass_Constructor_Init_Class_Values_Patch)
{
    GET_REGISTER_STATIC(AnimClass *, this_ptr, esi);
    static AnimTypeClassExtension *animtypeext;

    /**
     *  Stolen bytes/code.
     */
    this_ptr->IsActive = true;

    /**
     *  #BUGFIX:
     * 
     *  This check was observed in Red Alert 2, so there must be an edge case
     *  where anims are created with a null type instance. So lets do that
     *  here and also report a warning to the debug log.
     */
    if (!this_ptr->Class) {
        goto destroy_anim;
    }

    /**
     *  #issue-561
     * 
     *  Implements ZAdjust override for Anims. This will only have an effect
     *  if the anim is created with a z-adjustment value of "0" (default value).
     * 
     *  @author: CCHyper
     */
    if (!this_ptr->ZAdjust) {
        animtypeext = Extension::Fetch<AnimTypeClassExtension>(this_ptr->Class);
        this_ptr->ZAdjust = animtypeext->ZAdjust;
    }

    /**
     *  Restore some registers.
     */
    _asm { mov ecx, this_ptr }
    _asm { mov edx, [ecx+0x64] } // this->Class
    _asm { mov ecx, edx }

    JMP_REG(edx, 0x00413C80);

    /**
     *  Report that the anim type instance was invalid.
     */
destroy_anim:
    DEBUG_WARNING("Anim: Invalid anim type instance!\n");

    /**
     *  Remove the anim from the game world.
     */
    this_ptr->entry_E4();
    
    _asm { mov esi, this_ptr }
    JMP_REG(edx, 0x00414157);
}
#endif


/**
 *  #issue-114
 * 
 *  Animations that use RandomLoopDelay incorrectly use the unsynchronized
 *  random-number generator to generate their loop delay. This causes such
 *  animations to cause sync errors in multiplayer.
 * 
 *  @author: CCHyper (based on research by Rampastring)
 */
DECLARE_PATCH(_AnimClass_AI_RandomLoop_Randomiser_BugFix_Patch)
{
    GET_REGISTER_STATIC(AnimClass *, this_ptr, esi);
    static AnimTypeClass *animtype;

    animtype = reinterpret_cast<AnimTypeClass *>(this_ptr->Class_Of());

    /**
     *  Generate a random delay using the network synchronized RNG.
     */
    this_ptr->Delay = Random_Pick(animtype->RandomLoopDelayMin, animtype->RandomLoopDelayMax);

    /**
     *  Return from the function.
     */
function_return:
    JMP(0x00415AF2);
}


/**
 *  A helper is required because of stack issues.
 */
static Coordinate & Center_Coord_Helper(ObjectClass * obj)
{
    static Coordinate coord;
    coord = obj->Center_Coord();
    return coord;
}


/**
 *  Allows the animation to explode
 *  when it has reached its largest stage.
 *
 *  @author: Rampastring, ZivDero
 */
DECLARE_PATCH(_AnimClass_Middle_Explosion_Patch)
{
    GET_REGISTER_STATIC(AnimClass*, this_ptr, esi);
    static AnimTypeClass* animtype;
    static AnimTypeClassExtension* animtypeext;

    animtype = this_ptr->Class;

    /*
     * If this animation is specified to do area damage, do the area damage effect now.
     */
    animtypeext = Extension::Fetch<AnimTypeClassExtension>(animtype);
    if (animtypeext->IsExplosive && animtype->Warhead) {
        Explosion_Damage(Center_Coord_Helper(this_ptr), animtypeext->ExplosionDamage, nullptr, animtype->Warhead, true);
    }

    /**
     *  Continue function execution.
     */
continue_function:
    _asm { lea  ecx, [esp + 18h] }
    _asm { mov  eax, [esi] }
    _asm { push ecx }
    JMP_REG(ecx, 0x00415F4F);
}


/**
 *  Changes animations to use their Warhead= when dealing damage if one is specified
 *
 *  @author: ZivDero
 */
static void Do_Anim_Damage(AnimClass* anim, int damage)
{
    /*
     *  INVISO is hardcoded to use C4Warhead, let's leave that just in case.
     */
    if (std::strcmp(anim->Class->IniName, "INVISO") == 0) {
        Explosion_Damage(anim->Center_Coord(), damage, nullptr, Rule->C4Warhead);
    }
    /*
     *  If a Warhead= is set, use that.
     */
    else if (anim->Class->Warhead != nullptr) {
        Explosion_Damage(anim->Center_Coord(), damage, nullptr, anim->Class->Warhead);
    }
    /*
     *  Otherwise, default to FlameDamage2, like in vanilla.
     */
    else {
        Explosion_Damage(anim->Center_Coord(), damage, nullptr, Rule->FlameDamage2);
    }
}

DECLARE_PATCH(_AnimClass_AI_Warhead_Patch)
{
    GET_REGISTER_STATIC(AnimClass*, this_ptr, esi);
    GET_REGISTER_STATIC(int, damage, ebp);

    Do_Anim_Damage(this_ptr, damage);

    JMP(0x004159B2);
}


/**
 *  Main function for patching the hooks.
 */
void AnimClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    AnimClassExtension_Init();

    Patch_Jump(0x00415F38, &_AnimClass_Start_Ext_Patch);

    /**
     *  This patch removes duplicate return in AnimClass::Middle, so we only
     *  need to hook one place.
     */
    Patch_Jump(0x004162BD, 0x0041637C);

    /**
     *  Unfortunately, this manual patch is required because the code is optimised
     *  and reuses "this" (ESI), which we need for the ext patch.
     */
    Patch_Byte(0x0041636D, 0x8B); // mov esi, [esi+0x68] -> mov ebp, [esi+0x68]
    Patch_Byte(0x0041636D+1, 0x6E); // ^
    Patch_Byte(0x0041636D+2, 0x68); // ^
    Patch_Byte(0x00416370, 0x85); // test esi, esi -> test ebp, ebp
    Patch_Byte(0x00416370+1, 0xED); // ^
    Patch_Byte(0x00416374, 0x55); // push esi -> push ebp

    Patch_Jump(0x0041637C, &_AnimClass_Middle_Ext_Patch);

    Patch_Jump(0x00415AFA, &_AnimClass_AI_End_Ext_Patch);
    Patch_Jump(0x00415ADA, &_AnimClass_AI_RandomLoop_Randomiser_BugFix_Patch);
    //Patch_Jump(0x00413C79, &_AnimClass_Constructor_Init_Class_Values_Patch); // Moved to AnimClassExtension due to patching conflict.
    Patch_Jump(0x00414E8F, &_AnimClass_AI_Beginning_Patch);
    Patch_Jump(0x004160FB, &_AnimClass_Middle_Create_Crater_ForceBigCraters_Patch);
    Patch_Jump(0x0041606C, &_AnimClass_Middle_SpawnParticle_Patch);
    Patch_Jump(0x00415D30, &AnimClassExt::_In_Which_Layer);
    Patch_Jump(0x00413D3E, &_AnimClass_Constructor_Layer_Set_Z_Height_Patch);
    Patch_Jump(0x00415F48, &_AnimClass_Middle_Explosion_Patch);
    Patch_Jump(0x00415947, &_AnimClass_AI_Warhead_Patch);
}
