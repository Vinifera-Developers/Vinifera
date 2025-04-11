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
#include "coord.h"
#include "drawshape.h"
#include "mouse.h"
#include "voc.h"
#include "overlaytype.h"
#include "overlay.h"
#include "tactical.h"
#include "tiberium.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class AnimClassExt : public AnimClass
{
public:
    LayerType _In_Which_Layer() const;
    void _AI();
    void _Start();
    void _Middle();
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


/**
 *  Reimplementation of AnimClass::AI.
 *
 *  @author: ZivDero
 */
void AnimClassExt::_AI()
{
    const auto animext = Extension::Fetch<AnimClassExtension>(this);
    const auto animtypeext = Extension::Fetch<AnimTypeClassExtension>(Class);

    if (Class->IsFlamingGuy) {
        Flaming_Guy_AI();
        ObjectClass::AI();
    }

    CellClass* cellptr = Get_Cell_Ptr();

    /**
     *  #issue-560
     *
     *  Implements IsHideIfNotTiberium for Anims.
     *
     *  @author: CCHyper
     */
    if (animtypeext->IsHideIfNotTiberium) {
        if (!cellptr || !cellptr->Get_Tiberium_Value()) {
            IsInvisible = true;
        }
    }

    if (IsDebris) {
        int bounce_res = Bounce_AI();
        if (bounce_res == 2 || bounce_res == 1) {
            bool water = Map[Get_Coord()].Land_Type() == LAND_WATER;
            bool bridge = Get_Coord().Z >= Map.Get_Height_GL(Get_Coord()) + BRIDGE_LEPTON_HEIGHT;

            if (water && !bridge) {
                if (Class->IsMeteor) {
                    new AnimClass(Rule->SplashList[Rule->SplashList.Count() - 1], Get_Coord() + Coordinate(0, 0, 3));
                }
                else {
                    new AnimClass(Rule->Wake, Get_Coord());
                    new AnimClass(Rule->SplashList[0], Get_Coord() + Coordinate(0, 0, 3));
                }
            }
            else {
                if (Class->ExpireAnim != nullptr) {
                    Vector3 bouncecoord = Bounce.Coords;
                    new AnimClass(Class->ExpireAnim, Coordinate(bouncecoord.X, bouncecoord.Y, bouncecoord.Z), 0, 1, ShapeFlags_Type(SHAPE_CENTER | SHAPE_WIN_REL | SHAPE_FLAT), -30);
                    Explosion_Damage(Bounce.Get_Coord(), Class->Damage, nullptr, Class->Warhead);
                    Combat_Lighting(Bounce.Get_Coord(), Class->Damage, Class->Warhead);
                }
                if ((unsigned char)Class->ExpireSound != 0xFF) {
                    Static_Sound(Class->ExpireSound, Center_Coord());
                }
            }

            if (!water || bridge) {
                Coordinate coord = Bounce.Get_Coord();
                if (Class->Spawns != nullptr && Class->SpawnCount > 0) {
                    int count = Random_Pick(0, Class->SpawnCount) + Random_Pick(0, Class->SpawnCount);
                    for (int i = 0; i < count; i++) {
                        new AnimClass(Class->Spawns, coord);
                    }
                }

                if (Rule->CraterLevel != 0 && Class->IsMeteor && !bridge) {
                    Map.Deform(coord.As_Cell(), false);
                    if (Rule->CraterLevel > 1) {
                        for (int dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
                            if (dir % 2 != 0 || Rule->CraterLevel > 2) {
                                Map.Deform(Adjacent_Cell(coord.As_Cell(), (FacingType)dir), false);
                            }
                        }
                        if (Rule->CraterLevel > 3) {
                            Map.Deform(coord.As_Cell(), false);
                        }
                    }
                }

                Rect updaterect(0, 0, 0, 0);
                if (Class->IsTiberium && !bridge) {
                    for (int x = -Class->TiberiumSpreadRadius; x <= Class->TiberiumSpreadRadius; x++) {
                        for (int y = -Class->TiberiumSpreadRadius; y <= Class->TiberiumSpreadRadius; y++) {
                            if ((int)sqrt((double)x * (double)x + (double)y * (double)y) <= Class->TiberiumSpreadRadius) {
                                CellClass* cellptr = &Map[Adjacent_Cell(coord.As_Cell(), FacingType(x))];
                                if (cellptr->Can_Tiberium_Germinate(nullptr) && Class->TiberiumSpawnType != nullptr) {
                                    new OverlayClass(OverlayTypes[Class->TiberiumSpawnType->HeapID + Random_Pick(0, 3)], cellptr->Cell_Number());
                                    cellptr->OverlayData = Random_Pick(0, 2);
                                    Rect overlayrect = cellptr->Get_Overlay_Rect();
                                    overlayrect.Y -= TacticalRect.Y;
                                    updaterect = Union(updaterect, overlayrect);
                                }
                            }
                        }
                    }
                    TacticalMap->Register_Dirty_Area(updaterect);
                }
            }

            Remove_This();
            return;
        }
    }

    if (IsActive && !IsToDelete) {
        if (Class->TrailerAnim != nullptr && (Class->TrailerSeperation == 1 || (Frame % Class->TrailerSeperation == 0))) {
            new AnimClass(Class->TrailerAnim, Center_Coord(), 1);
        }
    }

    /*
    **	Special case check to make sure that building on top of a smoke marker
    **	causes the smoke marker to vanish.
    */
    if (Class == Rule->DropZoneAnim && Map[Center_Coord()].Cell_Building()) {
        IsToDelete = true;
    }

    /*
    **	Delete this animation and bail early if the animation is flagged to be deleted
    **	immediately.
    */
    if (IsToDelete) {
        Remove_This();
        return;
    }

    /*
    **	If this is a brand new animation, then don't process it the first logic pass
    **	since it might end up skipping the first animation frame before it has had a
    **	chance to draw it.
    */
    if (IsBrandNew) {
        IsBrandNew = false;
        return;
    }

    if (Delay) {
        Delay--;
        if (!Delay) {
            Start();
        }
    }
    else if (IsActive) {
        if (Class->IsVeins) {
            Vein_Attack_AI();
        }

        if (Class->IsAnimatedTiberium) {
            OverlayType overlay = Map[Center_Coord() - Coordinate(CELL_LEPTON_W * 1.5, CELL_LEPTON_H * 1.5, 0)].Overlay;
            if (overlay == OVERLAY_NONE || OverlayTypes[overlay]->CellAnim != Class) {
                IsToDelete = true;
            }
        }

        if (Class->Stages == -1) {
            Class->Stages = animtypeext->Stage_Count();
        }
        if (Class->LoopEnd == -1) {
            Class->LoopEnd = Class->Stages;
        }

        /*
        **	This is necessary because there is no recording of animations on the map
        **	and thus the animation cannot be intelligently flagged for redraw. Most
        **	animations move fast enough that they would need to be redrawn every
        **	game frame anyway so this isn't TOO bad.
        */
        Mark(MARK_UP_FORCED);

        if (!IsDisabled && animext->DamageStage.Graphic_Logic()) {

            /*
            **	If this animation is attached to another object and it is a
            **	damaging kind of animation, then do the damage to the other
            **	object.
            */
            if (Class->Damage > 0 && !IsDebris) {
                if (xObject != nullptr && xObject->RTTI == RTTI_TERRAIN) {
                    Accum += Class->Damage * 5;
                } else {
                    Accum += Class->Damage;
                }

                if (Accum >= 1 && !IsInert) {

                    /*
                    **	Administer the damage. If the object was destroyed by this anim,
                    **	then the attached damaging anim is also destroyed.
                    */
                    int damage = Accum;
                    Accum -= damage;
                    Do_Anim_Damage(this, damage);
                    if (!IsActive) {
                        return;
                    }
                }
            }
        }

        if (!IsDisabled && StageClass::Graphic_Logic()) {
            int stage = Fetch_Stage();

            /*
            **	During the biggest stage (covers the most ground), perform any ground altering
            **	action required. This masks craters and scorch marks, so that they appear
            **	naturally rather than "popping" into existence while in plain sight.
            */
            if (animtypeext->MiddleFrames.Count() && !IsDebris) {
                int frame = Class->Start + stage;
                const ShapeSet* image = Get_Image_Data();

                if (frame != 0) {
                    if (animtypeext->MiddleFrames.Is_Present(frame) || (animtypeext->MiddleFrames.Is_Present(-1) && image != nullptr && frame == image->Get_Count() / 2)) {
                        Middle();
                    }
                }
            }

            if (Class->IsPingPong) {
                if ((Loops <= 1 && (stage >= Class->Stages || stage == 0)) || (Loops > 1 && (stage >= Class->LoopEnd - Class->Start || stage == Class->Start))) {
                    Set_Step(-Fetch_Step());
                    return;
                }
            }

            /*
            **	Check to see if the last frame has been displayed. If so, then the
            **	animation either ends or loops.
            */
            if ((Loops <= 1 && stage >= Class->Stages) || (Loops > 1 && stage >= Class->LoopEnd - Class->Start) || (Class->IsReverse && stage <= Class->Start)) {

                /*
                **	Determine if this animation should loop another time. If so, then start the loop
                **	but if not, then proceed into the animation termination handler.
                */
                if (Loops && Loops != UCHAR_MAX) Loops--;
                if (Loops) {
                    if (Class->IsReverse) {
                        Set_Stage(Class->LoopEnd);
                    } else {
                        Set_Stage(Class->LoopStart - Class->Start);
                    }
                    if (Class->RandomLoopDelayMin != 0 || Class->RandomLoopDelayMax != 0) {

                        /**
                         *  #issue-114
                         *
                         *  Animations that use RandomLoopDelay incorrectly use the unsynchronized
                         *  random-number generator to generate their loop delay. This causes such
                         *  animations to cause sync errors in multiplayer.
                         *
                         *  @author: CCHyper (based on research by Rampastring)
                         */
                        Delay = Random_Pick(Class->RandomLoopDelayMin, Class->RandomLoopDelayMax);
                    }
                }
                else {

                    animext->End();

                    /*
                    **	The animation should end now, but first check to see if
                    **	it needs to chain into another animation. If so, then the
                    **	animation isn't technically over. It metamorphoses into the
                    **	new form.
                    */
                    if (Class->ChainTo != nullptr) {

                        Class = Class->ChainTo;

                        if (Class->Stages == -1) {
                            Class->Stages = animtypeext->Stage_Count();
                        }
                        if (Class->LoopEnd == -1) {
                            Class->LoopEnd = Class->Stages;
                        }

                        IsToDelete = false;
                        Loops = Class->Loops;
                        Accum = 0;
                        int delay = Class->Delay;
                        if (Class->RandomRateMin != 0 || Class->RandomRateMax != 0) {
                            delay = Random_Pick(Class->RandomRateMin, Class->RandomRateMax);
                        }
                        if (Class->IsNormalized) {
                            Set_Rate(Options.Normalize_Delay(delay));
                        } else {
                            Set_Rate(delay);
                        }
                        Set_Stage(Class->Start);
                        int damagedelay = animtypeext->DamageRate == -1 ? Fetch_Rate() : animtypeext->DamageRate;
                        animext->DamageStage.Set_Rate(damagedelay);
                        Start();
                    }
                    else {
                        Remove_This();
                    }
                }
            }
        }
    }
}


/**
 *  Reimplementation of AnimClass::Start.
 *
 *  @author: ZivDero
 */
void AnimClassExt::_Start()
{
    const auto animext = Extension::Fetch<AnimClassExtension>(this);
    const auto animtypeext = Extension::Fetch<AnimTypeClassExtension>(Class);

    Mark(MARK_UP_FORCED);

    if (!IsInert && Class->Sound != VOC_NONE) {

        /*
        **	Play the sound effect for this animation.
        */
        Static_Sound(Class->Sound, Center_Coord());
    }

    /*
    **	If the stage where collateral effects occur is the first stage of the animation, then
    **	perform this action now. Subsequent checks against this stage value starts with the
    **	second frame of the animation.
    */
    if (animtypeext->MiddleFrames.Is_Present(0)) {
        Middle();
    }

    if (!IsInert && Class->IsTiberiumChainReaction) {
        CellClass* cptr = &Map[Center_Coord()];
        TiberiumType tib = cptr->Tiberium_Type_Here();

        if (tib != TIBERIUM_NONE) {
            TiberiumClass* tiberium = Tiberiums[tib];
            cptr->Reduce_Tiberium(cptr->OverlayData + 1);

            if (tiberium->Debris.Count() > 0 && (abs(Scen->RandomNumber) % 3) == 0) {
                AnimClass* debris = new AnimClass(tiberium->Debris[Random_Pick(0, tiberium->Debris.Count() - 1)], Center_Coord() + Coordinate(0, 0, 10));
                debris->AlternativeDrawer = ColorSchemes[tiberium->Color]->Converter;
                debris->AlternativeBrightness = cptr->Brightness;
            }

            Explosion_Damage(Get_Coord(), Rule->TiberiumExplosionDamage, nullptr, Rule->C4Warhead, false);

            cptr->Recalc_Attributes();
            Map.Update_Cell_Zone(cptr->CellID);
            Map.Update_Cell_Subzones(cptr->CellID);
        }
    }

    animext->Start();
}


/**
 *  Spawn the attached particle systems. This is required as we can not
 *  allocate on the stack.
 *
 *  @author: CCHyper
 */
static void Anim_Spawn_Particles(AnimClass* this_ptr)
{
    AnimTypeClassExtension* animtypeext;

    animtypeext = Extension::Fetch<AnimTypeClassExtension>(this_ptr->Class);
    if (animtypeext->ParticleToSpawn != PARTICLE_NONE) {

        for (int i = 0; i < animtypeext->NumberOfParticles; ++i) {

            Coordinate spawn_coord = this_ptr->Coord;

            /**
             *  Spawn a new particle at this anims coord.
             */
            MasterParticle->Spawn_Particle(
                (ParticleTypeClass*)ParticleTypeClass::As_Pointer(animtypeext->ParticleToSpawn),
                spawn_coord);

        }
    }
}


/**
 *  Reimplementation of AnimClass::Middle.
 *
 *  @author: ZivDero
 */
void AnimClassExt::_Middle()
{
    const auto animext = Extension::Fetch<AnimClassExtension>(this);
    const auto animtypeext = Extension::Fetch<AnimTypeClassExtension>(Class);

    Cell cell = Center_Coord().As_Cell();
    CellClass* cellptr = &Map[cell];

    /*
     *  If this animation is specified to do area damage, do the area damage effect now.
     *  @author: Rampastring, ZivDero
     */
    if (animtypeext->ExplosionDamage > 0 && Class->Warhead) {
        Explosion_Damage(Center_Coord(), animtypeext->ExplosionDamage, nullptr, Class->Warhead, true);
    }

    int width = 30;
    int height = 30;

    ShapeSet const* shapefile = (ShapeSet const*)Get_Image_Data();
    if (shapefile != nullptr) {
        width = shapefile->Get_Rect(Class->Biggest).Width;
        height = shapefile->Get_Rect(Class->Biggest).Height;
    }

    /**
     *  #issue-568
     *
     *  Implements SpawnParticle for anims.
     *
     *  @author: CCHyper
     */
    Anim_Spawn_Particles(this);

    if (Height < 30) {

        /*
        **	If this animation leaves scorch marks (e.g., napalm), then do so at this time.
        */
        if (Class->IsScorcher && (!Class->IsCraterForming || (Random_Pick(0, INT_MAX - 1) / (double)(INT_MAX - 1)) < 0.5)) {
            SmudgeTypeClass::Create_Scorch(Center_Coord(), width, height);
        }

        /*
        **	Some animations leave a crater when they occur. Artillery is a good example.
        **	Craters always remove the Tiberium where they occur.
        */
        else if (Class->IsCraterForming) {

            /*
            **	Craters reduce the level of Tiberium in the cell.
            */
            cellptr->Reduce_Tiberium(6);

            /*
            **	If there already is a crater in the cell, then just expand the
            **	crater.
            */

            /**
             *  #issue-562
             *
             *  Implements IsForceBigCraters to Anims.
             *
             *  @author: CCHyper
             */
            if (animtypeext->IsForceBigCraters) {
                SmudgeTypeClass::Create_Crater(Center_Coord(), 300, 300, true);
            } else {
                SmudgeTypeClass::Create_Crater(Center_Coord(), width, height, false);
            }
        }
    }

    AnimClass* newanim;

    /*
    **	If this animation spawns side effects during its lifetime, then
    **	do so now. Usually, these side effects are in the form of other
    **	animations.
    */
    if (Class->IsFlameThrower) {
        new AnimClass(Rule->SmallFire, Map.Closest_Free_Spot(Coord_Scatter(Center_Coord(), 0x0040), true), 0, Random_Pick(1, 2));
        if (Percent_Chance(50)) {
            new AnimClass(Rule->SmallFire, Map.Closest_Free_Spot(Coord_Scatter(Center_Coord(), 0x00A0), true), 0, Random_Pick(1, 2));
        }
        if (Percent_Chance(50)) {
            new AnimClass(Rule->LargeFire, Map.Closest_Free_Spot(Coord_Scatter(Center_Coord(), 0x0070), true), 0, Random_Pick(1, 2));
        }
    }
    else if (Class->IsScorcher) {
        if (Height < 10) {
            LandType land = Map[Get_Coord()].Land_Type();
            if (land != LAND_WATER && land != LAND_BEACH && land != LAND_ICE && land != LAND_ROCK) {
                newanim = new AnimClass(Rule->SmallFire, Center_Coord(), 0, Random_Pick(1, 2));
                if (newanim != nullptr && xObject != nullptr) {
                    newanim->Attach_To(xObject);
                }
            }
        }
    }

    animext->Middle();
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
        this_ptr->AbsoluteHeight = Rule->FlightLevel;

    /**
     *  Original code.
     */
    } else if (!this_ptr->Class->IsGroundLayer) {
        this_ptr->AbsoluteHeight = Rule->FlightLevel;

    } else {
        this_ptr->Height = 0;
    }

    JMP(0x00413D63);
}


/**
 *  Saves the currently drawn animation before the call to Draw_Shape
 *  in AnimClass::Draw_It so that we can use it later.
 *
 *  @author: ZivDero
 */
static AnimClass* _CurrentlyDrawnAnim = nullptr;
DECLARE_PATCH(_AnimClass_Draw_It_Shadow_Patch)
{
    GET_REGISTER_STATIC(AnimClass*, anim, esi);
    _asm pushad

    _CurrentlyDrawnAnim = anim;

    _asm popad
    _asm mov eax, [eax + 0x1CC]
    JMP_REG(edx, 0x00414B48);
}


/**
 *  Proxy for Draw_Shape that lets up draw an animation's shadow
 *  without hunting for arguments.
 *
 *  @author: ZivDero
 */
void Draw_Shape_Proxy(
    Surface& surface,
    ConvertClass& convert,
    const ShapeSet* shapefile,
    int shapenum,
    const Point2D& point,
    const Rect& window,
    ShapeFlags_Type flags = SHAPE_NORMAL,
    const char* remap = nullptr,
    int height_offset = 0,
    ZGradientType zgrad = ZGRAD_GROUND,
    int intensity = 1000,
    const ShapeSet* z_shapefile = nullptr,
    int z_shapenum = 0,
    Point2D z_off = Point2D(0, 0))
{
    Draw_Shape(surface, convert, shapefile, shapenum, point, window, flags, remap, height_offset, zgrad, intensity, z_shapefile, z_shapenum, z_off);

    /**
     *  Make sure that we have a valid animation saved just in case.
     */
    if (_CurrentlyDrawnAnim != nullptr && _CurrentlyDrawnAnim->Class != nullptr) {
        const auto typeext = Extension::Fetch<AnimTypeClassExtension>(_CurrentlyDrawnAnim->Class);

        /**
         *  Draw the shadow.
         */
        if (typeext->IsShadow) {

            /**
             *  We can deduce the necessary arguments for the draw call based on the ones that were passed in.
             */
            int shadow_shapenum = shapenum + shapefile->Get_Count() / 2;
            Point2D shadow_point = point - Point2D(0, _CurrentlyDrawnAnim->Class->YDrawOffset);
            int shadow_height_offset = height_offset - _CurrentlyDrawnAnim->ZAdjust - _CurrentlyDrawnAnim->Class->YDrawOffset;
            ShapeFlags_Type shadow_flags = flags & ~(SHAPE_Z_REMAP | SHAPE_DARKEN | SHAPE_TRANS75 | SHAPE_CENTER | SHAPE_WIN_REL) | (SHAPE_DARKEN | SHAPE_CENTER | SHAPE_WIN_REL);

            Draw_Shape(surface, convert, shapefile, shadow_shapenum, shadow_point, window, shadow_flags, nullptr, shadow_height_offset);
        }

        /**
         *  Clean up the saved anim.
         */
        _CurrentlyDrawnAnim = nullptr;
    }
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

    Patch_Jump(0x00413D3E, &_AnimClass_Constructor_Layer_Set_Z_Height_Patch);
    Patch_Jump(0x00414B42, &_AnimClass_Draw_It_Shadow_Patch);
    Patch_Call(0x00414BA9, &Draw_Shape_Proxy);
    Patch_Jump(0x00415D30, &AnimClassExt::_In_Which_Layer);
    Patch_Jump(0x00414E80, &AnimClassExt::_AI);
    Patch_Jump(0x00415D60, &AnimClassExt::_Start);
    Patch_Jump(0x00415F40, &AnimClassExt::_Middle);
    Patch_Jump(0x00413C89, 0x00413CBF); // Skip setting Class->Stages and Class->LoopEnd in the constructor, let it happen in AI instead.
}
