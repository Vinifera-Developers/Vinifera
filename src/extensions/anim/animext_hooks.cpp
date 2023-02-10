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
#include "animext_init.h"
#include "animtype.h"
#include "animtypeext.h"
#include "building.h"
#include "smudgetype.h"
#include "smudge.h"
#include "particle.h"
#include "particletype.h"
#include "particlesys.h"
#include "mouse.h"
#include "target.h"
#include "cell.h"
#include "rules.h"
#include "scenario.h"
#include "extension.h"
#include "warheadtypeext.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

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
 * 
 *  #issue-520
 * 
 *  Implements "RA1 nuke style" area damage logic.
 *
 *  @author: Rampastring
*/
void DoAreaDamage(const ObjectClass* object_ptr, const int damageradius, const int rawdamage, const int damagepercentageatmaxrange,
    const int smudgechance, const int flamechance, const WarheadTypeClass* warhead, int unitDamageMultiplier) {
    Cell cell = Coord_Cell(object_ptr->Center_Coord());

    int				distance;	          // Distance to unit.
    ObjectClass* object;			      // Working object pointer.
    ObjectClass* objects[128];	      // Maximum number of objects that can be damaged.
    int             distances[128];       // Distances of the objects that can be damaged.
    int             count = 0;            // Number of objects to damage.


    // If we should create smudges, 
    // gather all valid smudgetypes for it
    SmudgeTypeClass* smudgetypes[6];
    int smudgetypecount = 0;
    if (smudgechance > 0) {

        for (int i = 0; i < SmudgeTypes.Count() && smudgetypecount < ARRAY_SIZE(smudgetypes); i++) {
            SmudgeTypeClass* smudgetype = SmudgeTypes[i];

            if (smudgetype->IsScorch) {
                smudgetypes[smudgetypecount] = smudgetype;
                smudgetypecount++;
            }
        }
    }

    for (int x = -damageradius; x <= damageradius; x++) {
        for (int y = -damageradius; y <= damageradius; y++) {
            int xpos = cell.X + x;
            int ypos = cell.Y + y;

            /*
            **	If the potential damage cell is outside of the map bounds,
            **	then don't process it. This unusual check method ensures that
            **	damage won't wrap from one side of the map to the other.
            */
            if ((unsigned)xpos > MAP_CELL_W) {
                continue;
            }
            if ((unsigned)ypos > MAP_CELL_H) {
                continue;
            }
            Cell tcell = XY_Cell(xpos, ypos);
            if (!Map.In_Radar(tcell)) continue;

            Coordinate tcellcoord = Cell_Coord(tcell);
            tcellcoord.X += CELL_LEPTON_W / 2;
            tcellcoord.Y += CELL_LEPTON_H / 2;

            object = Map[tcell].Cell_Occupier();
            while (object) {
                if (!object->IsToDamage) {
                    object->IsToDamage = true;
                    objects[count] = object;

                    if (object->What_Am_I() == RTTI_BUILDING) {
                        // Find the cell of the building that is closest 
                        // to the explosion point and use that as the reference point for the distance

                        BuildingClass* building = reinterpret_cast<BuildingClass*>(object);

                        Cell* occupy = building->Class->Occupy_List();
                        distances[count] = INT_MAX;

                        while (occupy->X != REFRESH_EOL && occupy->Y != REFRESH_EOL) {
                            Coordinate buildingcellcoord = building->Coord + Cell_Coord(*occupy, true) - Coordinate(CELL_LEPTON_W / 2, CELL_LEPTON_H / 2, 0);
                            distance = Distance(Cell_Coord(cell, true), buildingcellcoord);
                            distances[count] = std::min(distance, distances[count]);
                            occupy++;
                        }
                    }
                    else {
                        // For non-building objects, just check the distance directly
                        distances[count] = Distance(Cell_Coord(cell, true), object->Center_Coord());
                    }

                    count++;
                    if (count >= ARRAY_SIZE(objects)) break;
                }

                object = object->Next;
            }
            if (count >= ARRAY_SIZE(objects)) break;

            if (smudgechance > 0 && smudgetypecount > 0) {

                if (smudgechance >= 100 || Random_Pick(0, 100) < smudgechance) {
                    // Create a smudge on the cell
                    int smudgeindex = Random_Pick(0, smudgetypecount - 1);

                    SmudgeTypeClass* smudgetype = smudgetypes[smudgeindex];
                    new SmudgeClass(smudgetype, tcellcoord);
                }
            }

            if (flamechance > 0) {

                if (flamechance >= 100 || Random_Pick(0, 100) < flamechance) {
                    // Create a flame anim on the cell
                    AnimTypeClass* animtype = Rule->OnFire[Random_Pick(0, Rule->OnFire.Count() - 1)];
                    new AnimClass(animtype, tcellcoord);
                }
            }
        }
    }

    int maxdistance = damageradius * CELL_LEPTON_W;

    /*
    **	Sweep through the objects to be damaged and damage them.
    */
    for (int index = 0; index < count; index++) {
        object = objects[index];

        object->IsToDamage = false;
        if (object->IsActive) {
            distance = distances[index];

            float distancemult = (float)distance / (float)maxdistance;
            if (distancemult > 1.0f)
                distancemult = 1.0f;

            if (object->IsDown && !object->IsInLimbo) {
                int percentDecrease = (100 - damagepercentageatmaxrange) * distancemult;
                int damage = rawdamage - ((percentDecrease * rawdamage) / 100);

                // Adjust damage against units if necessary
                if (unitDamageMultiplier != 100 && object->Is_Foot()) {
                    damage = (damage * unitDamageMultiplier) / 100;
                }

                // We've taken the distance into account already
                object->Take_Damage(damage, 0, warhead, nullptr, false);
            }
        }
    }

    WarheadTypeClassExtension* warheadext = Extension::Fetch<WarheadTypeClassExtension>(warhead);

    if (warheadext) {

        /**
         *  If this warhead has screen shake values defined, then set the blitter
         *  offset values. GScreenClass::Blit will handle the rest for us.
         */
        if (warheadext->ShakePixelXLo > 0 || warheadext->ShakePixelXHi > 0) {
            Map.ScreenX = Sim_Random_Pick(warheadext->ShakePixelXLo, warheadext->ShakePixelXHi);
        }
        if (warheadext->ShakePixelYLo > 0 || warheadext->ShakePixelYHi > 0) {
            Map.ScreenY = Sim_Random_Pick(warheadext->ShakePixelYLo, warheadext->ShakePixelYHi);
        }
    }
}


/**
 *  Allows the animation to perform area damage
 *  when it has reached its largest stage.
 *
 *  @author: Rampastring
 */
DECLARE_PATCH(_AnimClass_Middle_Area_Damage_Patch)
{
    GET_REGISTER_STATIC(AnimClass*, this_ptr, esi);
    static AnimTypeClass* animtype;
    static AnimTypeClassExtension* animtypeext;

    animtype = reinterpret_cast<AnimTypeClass*>(this_ptr->Class_Of());

    /*
     * If this animation is specified to do area damage, do the area damage effect now.
     */
    animtypeext = Extension::Fetch<AnimTypeClassExtension>(animtype);
    if (animtypeext->AreaDamage > 0 && animtypeext->AreaDamageRadius > 0) {
        ASSERT(animtype->Warhead != nullptr);

        if (animtype->Warhead) {
            DoAreaDamage(this_ptr,
                animtypeext->AreaDamageRadius,
                animtypeext->AreaDamage,
                animtypeext->AreaDamagePercentAtMaxRange,
                animtypeext->AreaDamageSmudgeChance,
                animtypeext->AreaDamageFlameChance,
                animtype->Warhead,
                animtypeext->AreaDamagePercentAgainstUnits);
        }
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
 *  Main function for patching the hooks.
 */
void AnimClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    AnimClassExtension_Init();

    Patch_Jump(0x00415ADA, &_AnimClass_AI_RandomLoop_Randomiser_BugFix_Patch);
    //Patch_Jump(0x00413C79, &_AnimClass_Constructor_Init_Class_Values_Patch); // Moved to AnimClassExtension due to patching conflict.
    Patch_Jump(0x00414E8F, &_AnimClass_AI_Beginning_Patch);
    Patch_Jump(0x004160FB, &_AnimClass_Middle_Create_Crater_ForceBigCraters_Patch);
    Patch_Jump(0x0041606C, &_AnimClass_Middle_SpawnParticle_Patch);
    Patch_Jump(0x00415D30, &AnimClassExt::_In_Which_Layer);
    Patch_Jump(0x00413D3E, &_AnimClass_Constructor_Layer_Set_Z_Height_Patch);
    Patch_Jump(0x00415F48, &_AnimClass_Middle_Area_Damage_Patch);
}
