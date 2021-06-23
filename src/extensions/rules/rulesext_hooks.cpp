/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          RULESEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended RulesClass.
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
#include "buildingext_hooks.h"
#include "rules.h"
#include "tiberium.h"
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
class RulesClassFake final : public RulesClass
{
    public:
        void _Process(CCINIClass &ini);
};


/**
 *  Fetch the bulk of the rule data from the control file.
 * 
 *  @author: CCHyper
 */
void RulesClassFake::_Process(CCINIClass &ini)
{
    Colors(ini);
    Houses(ini);
    Sides(ini);
    Overlays(ini);
    SuperWeapons(ini);
    Warheads(ini);
    Smudges(ini);
    Terrains(ini);
    Buildings(ini);
    Vehicles(ini);
    Aircraft(ini);
    Infantry(ini);
    Animations(ini);
    VoxelAnims(ini);
    Particles(ini);
    ParticleSystems(ini);
    JumpjetControls(ini);
    MPlayer(ini);
    AI(ini);
    Powerups(ini);
    Land_Types(ini);
    IQ(ini);
    General(ini);
    Objects(ini);
    Difficulty(ini);
    CrateRules(ini);
    CombatDamage(ini);
    AudioVisual(ini);
    SpecialWeapons(ini);
    TiberiumClass::Process(ini);
}


/**
 *  Main function for patching the hooks.
 */
void RulesClassExtension_Hooks()
{
    Patch_Jump(0x005C6710, &RulesClassFake::_Process);
}
