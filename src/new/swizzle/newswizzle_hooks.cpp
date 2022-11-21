/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          NEWSWIZZLE_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for implementing the new swizzle manager.
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
#include "tibsun_globals.h"
#include "vinifera_saveload.h"
#include "saveload.h"
#include "vinifera_util.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "fatal.h"

#include "hooker.h"
#include "hooker_macros.h"

#include "aircraft.h"
#include "aitrigtype.h"
#include "alphashape.h"
#include "anim.h"
#include "animtype.h"
#include "base.h"
#include "building.h"
#include "buildinglight.h"
#include "brain.h"
#include "bullet.h"
#include "bullettype.h"
#include "cell.h"
#include "factory.h"
#include "foot.h"
#include "house.h"
#include "rules.h"
#include "script.h"
#include "techno.h"
#include "technotype.h"
#include "team.h"
#include "voxelanim.h"
#include "voxelanimtype.h"

#include "wstring.h"


#ifdef VINIFERA_USE_NEW_SWIZZLE_MANAGER

/**
 *  The swizzle database contains return addresses for all annoucement and remap
 *  request calls, allowing us to detect this address and attach debug information
 *  which is used by the new Swizzle manager.
 */
static struct SwizzleInfoDatabaseEntry
{
    SwizzleInfoDatabaseEntry() :
        ReturnAddress(0x00000000),
        Line(0),
        File(),
        Function(),
        Variable()
    {
        std::memset(File, 0, sizeof(File));
        std::memset(Function, 0, sizeof(Function));
        std::memset(Variable, 0, sizeof(Variable));
    }

    bool operator==(const SwizzleInfoDatabaseEntry &src) const { return false; }
    bool operator!=(const SwizzleInfoDatabaseEntry &src) const { return true; }

    uint32_t ReturnAddress;
    char Variable[512];
    char Function[512];
    char File[512];
    uint32_t Line;
};

static DynamicVectorClass<SwizzleInfoDatabaseEntry> SwizzleInfoDatabase;


/**
 *  Adds a database entry to the global list.
 * 
 *  @author: tomsons26
 */
static void Add_Swizzle_Database_Entry(uint32_t retaddr, char *function,  char *variable, char *file, int line = -1)
{
    SwizzleInfoDatabaseEntry info;

    info.ReturnAddress = retaddr;
    std::strncpy(info.Variable, variable, sizeof(info.Variable));
    std::strncpy(info.Function, function, sizeof(info.Function));

    /**
     *  The original Tiberian Sun source tree path.
     */
    static const char *TIBSUN_SOURCE_PATH = "D:\\Projects\\Sun\\CodeFS\\";

    // Add the Tiberian Sun source path (as we know it) to the source name.
    Wstring filepath = TIBSUN_SOURCE_PATH;
    filepath += file;

    std::strncpy(info.File, filepath.Peek_Buffer(), sizeof(info.File));

    info.Line = line;

    SwizzleInfoDatabase.Add(info);
}


/**
 *  Populate the database with debug information for all known callsites.
 * 
 *  @author: tomsons26
 */
static void Build_Swizzle_Address_Database()
{
    // Here_I_Am
    Add_Swizzle_Database_Entry(0x00405D4C, STRINGIZE(AbstractClass::Abstract_Load()), "this", "Abstract.cpp");
    Add_Swizzle_Database_Entry(0x004CE38C, STRINGIZE(HouseTypeClass::Load()), "this", "HouseType.cpp");
    Add_Swizzle_Database_Entry(0x0050662C, STRINGIZE(LocomotionClass::Load()), "this", "Locomotion.cpp");
    Add_Swizzle_Database_Entry(0x0061F433, STRINGIZE(TagTypeClass::Read_Scenario_INI()), "tagtypeptr", "TagType.cpp");
    Add_Swizzle_Database_Entry(0x00621912, STRINGIZE(TaskForceClass::Read_Scenario_INI()), "taskforceptr", "TaskForce.cpp");
    Add_Swizzle_Database_Entry(0x00663010, STRINGIZE(VeinholeMonsterClass::Load()), "veinholeptr", "Veinhole.cpp");
    Add_Swizzle_Database_Entry(0x004CE3C8, STRINGIZE(HouseTypeClass::Save()), "this", "HouseType.cpp");

    // Swizzle
    Add_Swizzle_Database_Entry(0x0040ECBE, STRINGIZE(AircraftClass::Load()), STRINGIZE(AircraftClass::Class), "Aircraft.cpp");
    Add_Swizzle_Database_Entry(0x004106E2, STRINGIZE(AITriggerTypeClass::Load()), STRINGIZE(AITriggerTypeClass::ConditionObject), "AITrigType.cpp");
    Add_Swizzle_Database_Entry(0x004106F3, STRINGIZE(AITriggerTypeClass::Load()), STRINGIZE(AITriggerTypeClass::TeamOne), "AITrigType.cpp");
    Add_Swizzle_Database_Entry(0x00410704, STRINGIZE(AITriggerTypeClass::Load()), STRINGIZE(AITriggerTypeClass::TeamTwo), "AITrigType.cpp");
    Add_Swizzle_Database_Entry(0x00412A32, STRINGIZE(AlphaShapeClass::Load()), STRINGIZE(AlphaShapeClass::AttachedTo), "AlphaShape.cpp");
    Add_Swizzle_Database_Entry(0x004164F2, STRINGIZE(AnimClass::Load()), STRINGIZE(AnimClass::Class), "Anim.cpp");
    Add_Swizzle_Database_Entry(0x00416500, STRINGIZE(AnimClass::Load()), STRINGIZE(AnimClass::xObject), "Anim.cpp");
    Add_Swizzle_Database_Entry(0x0041969D, STRINGIZE(AnimTypeClass::Load()), STRINGIZE(AnimTypeClass::ChainTo), "AnimType.cpp");
    Add_Swizzle_Database_Entry(0x004196AE, STRINGIZE(AnimTypeClass::Load()), STRINGIZE(AnimTypeClass::ExpireAnim), "AnimType.cpp");
    Add_Swizzle_Database_Entry(0x004196BF, STRINGIZE(AnimTypeClass::Load()), STRINGIZE(AnimTypeClass::TrailerAnim), "AnimType.cpp");
    Add_Swizzle_Database_Entry(0x004196D0, STRINGIZE(AnimTypeClass::Load()), STRINGIZE(AnimTypeClass::BounceAnim), "AnimType.cpp");
    Add_Swizzle_Database_Entry(0x004196E1, STRINGIZE(AnimTypeClass::Load()), STRINGIZE(AnimTypeClass::Spawns), "AnimType.cpp");
    Add_Swizzle_Database_Entry(0x004196F2, STRINGIZE(AnimTypeClass::Load()), STRINGIZE(AnimTypeClass::Warhead), "AnimType.cpp");
    Add_Swizzle_Database_Entry(0x00419703, STRINGIZE(AnimTypeClass::Load()), STRINGIZE(AnimTypeClass::TiberiumSpawnType), "AnimType.cpp");
    Add_Swizzle_Database_Entry(0x0041FDA5, STRINGIZE(BaseClass::Load()), STRINGIZE(BaseClass::House), "Base.cpp");
    Add_Swizzle_Database_Entry(0x00422B05, STRINGIZE(BuildingLightClass::Load()), STRINGIZE(BuildingLightClass::Source), "BuildingLight.cpp");
    Add_Swizzle_Database_Entry(0x00422B16, STRINGIZE(BuildingLightClass::Load()), STRINGIZE(BuildingLightClass::Following), "BuildingLight.cpp");
    Add_Swizzle_Database_Entry(0x004254BF, STRINGIZE(NeuronClass::Load()), STRINGIZE(NeuronClass::field_14), "Brain.cpp");
    Add_Swizzle_Database_Entry(0x004254CD, STRINGIZE(NeuronClass::Load()), STRINGIZE(NeuronClass::field_18), "Brain.cpp");
    Add_Swizzle_Database_Entry(0x004254DB, STRINGIZE(NeuronClass::Load()), STRINGIZE(NeuronClass::Brain), "Brain.cpp");
    Add_Swizzle_Database_Entry(0x0043817C, STRINGIZE(BuildingClass::Load()), STRINGIZE(BuildingClass::Class), "Building.cpp");
    Add_Swizzle_Database_Entry(0x0043818D, STRINGIZE(BuildingClass::Load()), STRINGIZE(BuildingClass::Factory), "Building.cpp");
    Add_Swizzle_Database_Entry(0x0043819E, STRINGIZE(BuildingClass::Load()), STRINGIZE(BuildingClass::WhomToRepay), "Building.cpp");
    Add_Swizzle_Database_Entry(0x004381AF, STRINGIZE(BuildingClass::Load()), STRINGIZE(BuildingClass::AnimToTrack), "Building.cpp");
    Add_Swizzle_Database_Entry(0x004381C0, STRINGIZE(BuildingClass::Load()), STRINGIZE(BuildingClass::BuildingLight), "Building.cpp");
    Add_Swizzle_Database_Entry(0x004381D6, STRINGIZE(BuildingClass::Load()), STRINGIZE(BuildingClass::Anims), "Building.cpp");
    Add_Swizzle_Database_Entry(0x004381F2, STRINGIZE(BuildingClass::Load()), STRINGIZE(BuildingClass::Upgrades), "Building.cpp");
    Add_Swizzle_Database_Entry(0x00443497, STRINGIZE(BuildingTypeClass::Load()), STRINGIZE(BuildingTypeClass::ToOverlay), "BuildingType.cpp");
    Add_Swizzle_Database_Entry(0x004434A8, STRINGIZE(BuildingTypeClass::Load()), STRINGIZE(BuildingTypeClass::FreeUnit), "BuildingType.cpp");
    Add_Swizzle_Database_Entry(0x0044714A, STRINGIZE(BulletClass::Load()), STRINGIZE(BulletClass::Class), "Bullet.cpp");
    Add_Swizzle_Database_Entry(0x00447158, STRINGIZE(BulletClass::Load()), STRINGIZE(BulletClass::Payback), "Bullet.cpp");
    Add_Swizzle_Database_Entry(0x00447169, STRINGIZE(BulletClass::Load()), STRINGIZE(BulletClass::Warhead), "Bullet.cpp");
    Add_Swizzle_Database_Entry(0x0044717A, STRINGIZE(BulletClass::Load()), STRINGIZE(BulletClass::TarCom), "Bullet.cpp");
    Add_Swizzle_Database_Entry(0x0044858C, STRINGIZE(BulletTypeClass::Load()), STRINGIZE(BulletTypeClass::Trailer), "BulletType.cpp");
    Add_Swizzle_Database_Entry(0x0044859D, STRINGIZE(BulletTypeClass::Load()), STRINGIZE(BulletTypeClass::AirburstWeapon), "BulletType.cpp");
    Add_Swizzle_Database_Entry(0x00459903, STRINGIZE(CellClass::Load()), STRINGIZE(CellClass::FoggedObjects), "Cell.cpp");
    Add_Swizzle_Database_Entry(0x0045994E, STRINGIZE(CellClass::Load()), STRINGIZE(CellClass::field_1C), "Cell.cpp");
    Add_Swizzle_Database_Entry(0x0045995C, STRINGIZE(CellClass::Load()), STRINGIZE(CellClass::field_20), "Cell.cpp");
    Add_Swizzle_Database_Entry(0x0045996A, STRINGIZE(CellClass::Load()), STRINGIZE(CellClass::CellTag), "Cell.cpp");
    Add_Swizzle_Database_Entry(0x00459978, STRINGIZE(CellClass::Load()), STRINGIZE(CellClass::OccupierPtr), "Cell.cpp");
    Add_Swizzle_Database_Entry(0x00459986, STRINGIZE(CellClass::Load()), STRINGIZE(CellClass::AltOccupierPtr), "Cell.cpp");
    Add_Swizzle_Database_Entry(0x0048425C, STRINGIZE(DropshipLoadoutClass::Load()), STRINGIZE(DropshipLoadoutClass::field_14), "Dropship.cpp");
    Add_Swizzle_Database_Entry(0x004976BD, STRINGIZE(FactoryClass::Load()), STRINGIZE(FactoryClass::QueuedObjects), "Factory.cpp");
    Add_Swizzle_Database_Entry(0x004976D2, STRINGIZE(FactoryClass::Load()), STRINGIZE(FactoryClass::House), "Factory.cpp");
    Add_Swizzle_Database_Entry(0x004976E0, STRINGIZE(FactoryClass::Load()), STRINGIZE(FactoryClass::Object), "Factory.cpp");
    Add_Swizzle_Database_Entry(0x0049F487, STRINGIZE(FoggedObjectClass::Load()), STRINGIZE(FoggedObjectClass::Owner), "FoggedObj.cpp");
    Add_Swizzle_Database_Entry(0x0049F552, STRINGIZE(FoggedObjectClass::Load()), STRINGIZE(FoggedObjectClass::DrawRecords[]), "FoggedObj.cpp");
    Add_Swizzle_Database_Entry(0x004A6104, STRINGIZE(FootClass::Load()), STRINGIZE(FootClass::Team), "Foot.cpp");
    Add_Swizzle_Database_Entry(0x004A6115, STRINGIZE(FootClass::Load()), STRINGIZE(FootClass::Member), "Foot.cpp");
    Add_Swizzle_Database_Entry(0x004A6126, STRINGIZE(FootClass::Load()), STRINGIZE(FootClass::NavCom), "Foot.cpp");
    Add_Swizzle_Database_Entry(0x004A6137, STRINGIZE(FootClass::Load()), STRINGIZE(FootClass::SuspendedNavCom), "Foot.cpp");
    Add_Swizzle_Database_Entry(0x004A6148, STRINGIZE(FootClass::Load()), STRINGIZE(FootClass::field_2A0), "Foot.cpp");
    Add_Swizzle_Database_Entry(0x004A6168, STRINGIZE(FootClass::Load()), STRINGIZE(FootClass::NavList), "Foot.cpp");
    Add_Swizzle_Database_Entry(0x004A6193, STRINGIZE(FootClass::Load()), STRINGIZE(FootClass::PathfindingCells), "Foot.cpp");
    Add_Swizzle_Database_Entry(0x004C4B97, STRINGIZE(HouseClass::Load()), STRINGIZE(HouseClass::Class), "House.cpp");
    Add_Swizzle_Database_Entry(0x004C4BA8, STRINGIZE(HouseClass::Load()), STRINGIZE(HouseClass::InfantryFactory), "House.cpp");
    Add_Swizzle_Database_Entry(0x004C4BB9, STRINGIZE(HouseClass::Load()), STRINGIZE(HouseClass::UnitFactory), "House.cpp");
    Add_Swizzle_Database_Entry(0x004C4BCA, STRINGIZE(HouseClass::Load()), STRINGIZE(HouseClass::AircraftFactory), "House.cpp");
    Add_Swizzle_Database_Entry(0x004C4BDB, STRINGIZE(HouseClass::Load()), STRINGIZE(HouseClass::BuildingFactory), "House.cpp");
    Add_Swizzle_Database_Entry(0x004C4BEC, STRINGIZE(HouseClass::Load()), STRINGIZE(HouseClass::ToCapture), "House.cpp");
    Add_Swizzle_Database_Entry(0x004C4C02, STRINGIZE(HouseClass::Load()), STRINGIZE(HouseClass::WaypointPaths), "House.cpp");
    Add_Swizzle_Database_Entry(0x004C4E20, STRINGIZE(HouseClass::Load()), STRINGIZE(HouseClass::AngerNodes), "House.cpp");
    Add_Swizzle_Database_Entry(0x004C4EFE, STRINGIZE(HouseClass::Load()), STRINGIZE(HouseClass::ScoutNodes), "House.cpp");
    Add_Swizzle_Database_Entry(0x004C4FBD, STRINGIZE(HouseClass::Load()), STRINGIZE(HouseClass::SuperWeapon), "House.cpp");
    Add_Swizzle_Database_Entry(0x004C4FE2, STRINGIZE(HouseClass::Load()),  STRINGIZE(HouseClass::field_28), "House.cpp");
    Add_Swizzle_Database_Entry(0x004C5004, STRINGIZE(HouseClass::Load()),  STRINGIZE(HouseClass::field_40), "House.cpp");
    Add_Swizzle_Database_Entry(0x004D94B8, STRINGIZE(InfantryClass::Load()), STRINGIZE(InfantryClass::Class), "Infantry.cpp");
    Add_Swizzle_Database_Entry(0x004F22A2, STRINGIZE(IsometricTileClass::Load()), STRINGIZE(Class), "IsoTile.cpp");
    Add_Swizzle_Database_Entry(0x004F8651, STRINGIZE(IsometricTileTypeClass::Load()), STRINGIZE(IsometricTileTypeClass::NextTileTypeInSet), "IsoTileType.cpp");
    Add_Swizzle_Database_Entry(0x004FD005, STRINGIZE(LayerClass::Load()), STRINGIZE(LayerClass::(*this)[]), "Layer.cpp");
    Add_Swizzle_Database_Entry(0x00506655, STRINGIZE(LayerClass::Load()), STRINGIZE(LocomotionClass::LinkedTo), "Locomotion.cpp");
    Add_Swizzle_Database_Entry(0x005627FC, STRINGIZE(SidebarClass::StripClass::Load()), STRINGIZE(SidebarClass::StripClass::Buildables::Factory), "Sidebar.cpp");
    Add_Swizzle_Database_Entry(0x00562824, STRINGIZE(DisplayClass::Load()), STRINGIZE(DisplayClass::PendingObjectPtr), "Display.cpp");
    Add_Swizzle_Database_Entry(0x00562835, STRINGIZE(DisplayClass::Load()), STRINGIZE(DisplayClass::PendingObject), "Display.cpp");
    Add_Swizzle_Database_Entry(0x00562846, STRINGIZE(DisplayClass::Load()), STRINGIZE(DisplayClass::FollowingObjectPtr), "Display.cpp");
    Add_Swizzle_Database_Entry(0x00586712, STRINGIZE(ObjectClass::Load()), STRINGIZE(ObjectClass::Next), "Object.cpp");
    Add_Swizzle_Database_Entry(0x00586720, STRINGIZE(ObjectClass::Load()), STRINGIZE(ObjectClass::AttachedTag), "Object.cpp");
    Add_Swizzle_Database_Entry(0x0058C512, STRINGIZE(OverlayClass::Load()), STRINGIZE(ObjectClass::Class), "Overlay.cpp");
    Add_Swizzle_Database_Entry(0x0058D84D, STRINGIZE(OverlayTypeClass::Load()), STRINGIZE(OverlayTypeClass::CellAnim), "OverlayType.cpp");
    Add_Swizzle_Database_Entry(0x005A4E6F, STRINGIZE(ParticleClass::Load()), STRINGIZE(ParticleClass::Class), "Particle.cpp");
    Add_Swizzle_Database_Entry(0x005A4E80, STRINGIZE(ParticleClass::Load()), STRINGIZE(ParticleClass::ParticleSystem), "Particle.cpp");
    Add_Swizzle_Database_Entry(0x005A7645, STRINGIZE(ParticleSystemClass::Load()), STRINGIZE(ParticleSystemClass::Class), "ParticleSys.cpp");
    Add_Swizzle_Database_Entry(0x005A7656, STRINGIZE(ParticleSystemClass::Load()), STRINGIZE(ParticleSystemClass::Target), "ParticleSys.cpp");
    Add_Swizzle_Database_Entry(0x005A7667, STRINGIZE(ParticleSystemClass::Load()), STRINGIZE(ParticleSystemClass::Owner), "ParticleSys.cpp");
    Add_Swizzle_Database_Entry(0x005A7714, STRINGIZE(ParticleSystemClass::Load()), STRINGIZE(ParticleSystemClass::Particles), "ParticleSys.cpp");
    Add_Swizzle_Database_Entry(0x005AF9A1, STRINGIZE(ParticleTypeClass::Load()), STRINGIZE(ParticleTypeClass::Warhead), "ParticleType.cpp");
    Add_Swizzle_Database_Entry(0x005BDAC2, STRINGIZE(RadioClass::Load()), STRINGIZE(RadioClass::Radio), "Radio.cpp");
    Add_Swizzle_Database_Entry(0x005D01E5, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::NukeWarhead), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D01F6, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::FlameDamage), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0207, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::FlameDamage2), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0215, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::LargeVisceroid), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0223, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::SmallVisceroid), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0231, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::UnloadingHarvester), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0242, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::DropPodWeapon), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0253, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::EMPulseWarhead), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0264, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::C4Warhead), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0275, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::IonCannonWarhead), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0286, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::FirestormWarhead), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0297, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::VeinholeWarhead), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D02A8, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::DefaultFirestormExplosionSystem), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D02B9, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::DefaultLargeGreySmokeSystem), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D02CA, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::DefaultSmallGreySmokeSystem), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D02DB, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::DefaultSparkSystem), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D02EC, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::DefaultLargeRedSmokeSystem), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D02FD, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::DefaultSmallRedSmokeSystem), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D030E, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::DefaultDebrisSmokeSystem), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D031F, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::DefaultFireStreamSystem), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0330, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::DefaultTestParticleSystem), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0341, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::DefaultRepairParticleSystem), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0352, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::VeinholeTypeClass), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0363, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::field_7D4), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0374, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::Smoke), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0385, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::MoveFlash), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0396, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::BombParachute), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D03A7, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::Parachute), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D03B8, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::SmallFire), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D03C9, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::LargeFire), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D03DA, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::DropZoneAnim), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D03EB, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::BaseUnit), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D03FC, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::UnitCrateType), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D040D, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::Paratrooper), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D041E, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::Disguise), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D042F, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::Technician), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0440, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::Engineer), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0451, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::Pilot), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0462, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::Crew), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0473, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::RepairBay), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0484, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::GDIGateOne), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0495, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::GDIGateTwo), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D04A6, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::NodGateOne), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D04B7, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::NodGateTwo), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D04C8, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::WallTower), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D04D9, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::GDIPowerPlant), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D04EA, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::GDIPowerTurbine), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D04FB, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::NodRegularPower), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D050C, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::NodAdvancedPower), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D051D, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::GDIFirestormGenerator), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D052E, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::GDIHunterSeeker), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D053F, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::NodHunterSeeker), "Rules.cpp"); 
    Add_Swizzle_Database_Entry(0x005D0550, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::NukeProjectile), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0561, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::NukeDown), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0572, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::EMPulseProjectile), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0583, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::TireVoxelDebris), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0594, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::ScrapVoxelDebris), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D05A5, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::AtmosphereEntry), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D05B6, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::InfantryExplode), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D05C7, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::IonBlast), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D05D8, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::IonBeam), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D05E9, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::DigAnim), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D05FA, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::FirestormIdleAnim), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D060B, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::FirestormAirAnim), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D061C, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::FirestormGroundAnim), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D062D, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::FirestormActiveAnim), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D063E, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::EMPulseSparkles), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D064F, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::Wake), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0660, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::FlamingInfantry), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0671, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::VeinAttack), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D067F, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::BarrelExplode), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D068D, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::BarrelParticle), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D069B, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::DropPodPuff), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D06AC, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::IonStormWarhead), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D06BD, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::WoodCrateImg), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D06CE, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::CrateImg), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D06DF, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::WebbedInfantry), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D06F9, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::BarrelDebris), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0721, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::OnFire), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D074C, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::TreeFire), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0777, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::SplashList), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D07A2, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::Scorches), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D07CD, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::Scorches1), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D07F8, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::Scorches2), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0823, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::Scorches3), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D084E, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::Scorches4), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0879, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::Craters), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D08A4, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::HarvesterUnit), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D08CF, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::BuildConst), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D08FA, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::BuildPower), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0925, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::BuildRefinery), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0950, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::BuildBarracks), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D097B, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::BuildTech), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D09A6, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::BuildWeapons), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D09D1, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::BuildDefense), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D09FC, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::BuildPDefense), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0A27, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::BuildAA), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0A52, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::BuildHelipad), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0A7D, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::BuildRadar), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0AA8, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::ConcreteWalls), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0AD3, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::NSGates), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0AFE, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::EWGates), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0B29, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::PadAircraft), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0B54, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::ExplosiveVoxelDebris), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0B7F, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::DeadBodies), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0BAA, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::DropPod), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0BD5, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::MetallicDebris), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0C00, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::BridgeExplosions), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D0C25, STRINGIZE(RulesClass::Load()), STRINGIZE(RulesClass::HSBuilding), "Rules.cpp");
    Add_Swizzle_Database_Entry(0x005D7B1E, STRINGIZE(Load_Misc_Values()), STRINGIZE(MasterParticle), "SaveLoad.cpp");
    Add_Swizzle_Database_Entry(0x005D7B44, STRINGIZE(Load_Misc_Values()), STRINGIZE(PlayerPtr), "SaveLoad.cpp");
    Add_Swizzle_Database_Entry(0x005D7C14, STRINGIZE(Load_Misc_Values()), STRINGIZE(CurrentObjects), "SaveLoad.cpp");
    Add_Swizzle_Database_Entry(0x005D7D39, STRINGIZE(Load_Misc_Values()), STRINGIZE(LogicTriggers), "SaveLoad.cpp");
    Add_Swizzle_Database_Entry(0x005D7DFB, STRINGIZE(Load_Misc_Values()), STRINGIZE(MapTriggers), "SaveLoad.cpp");
    Add_Swizzle_Database_Entry(0x005DF6D2, STRINGIZE(Scenario_Load()), STRINGIZE(ScenarioClass::AllowableUnits), "Scenario.cpp");
    Add_Swizzle_Database_Entry(0x005E7A8F, STRINGIZE(ScriptClass::Load()), STRINGIZE(ScriptClass::Class), "Script.cpp");
    Add_Swizzle_Database_Entry(0x005FAE42, STRINGIZE(SmudgeClass::Load()), STRINGIZE(SmudgeClass::Class), "Smudge.cpp");
    Add_Swizzle_Database_Entry(0x0060C7CA, STRINGIZE(SuperClass::Load()), STRINGIZE(SuperClass::Class), "Super.cpp");
    Add_Swizzle_Database_Entry(0x0060C7D8, STRINGIZE(SuperClass::Load()), STRINGIZE(SuperClass::House), "Super.cpp");
    Add_Swizzle_Database_Entry(0x0060D196, STRINGIZE(SuperWeaponTypeClass::Load()), STRINGIZE(SuperWeaponTypeClass::Weapon), "SuperType.cpp");
    Add_Swizzle_Database_Entry(0x0060D1A7, STRINGIZE(SuperWeaponTypeClass::Load()), STRINGIZE(SuperWeaponTypeClass::AuxBuilding), "SuperType.cpp");
    Add_Swizzle_Database_Entry(0x00617F6A, STRINGIZE(Tactical::Load()), STRINGIZE(Tactical::VisibleCells), "Tactical.cpp");
    Add_Swizzle_Database_Entry(0x0061D8DF, STRINGIZE(TActionClass::Load()), STRINGIZE(TActionClass::Next), "TAction.cpp");
    Add_Swizzle_Database_Entry(0x0061D8ED, STRINGIZE(TActionClass::Load()), STRINGIZE(TActionClass::Team), "TAction.cpp");
    Add_Swizzle_Database_Entry(0x0061D8FB, STRINGIZE(TActionClass::Load()), STRINGIZE(TActionClass::Tag), "TAction.cpp");
    Add_Swizzle_Database_Entry(0x0061D909, STRINGIZE(TActionClass::Load()), STRINGIZE(TActionClass::Trigger), "TAction.cpp");
    Add_Swizzle_Database_Entry(0x0061EC1F, STRINGIZE(TagClass::Load()), STRINGIZE(TagClass::Class), "Tag.cpp");
    Add_Swizzle_Database_Entry(0x0061EC2D, STRINGIZE(TagClass::Load()), STRINGIZE(TagClass::AttachedTrigger), "Tag.cpp");
    Add_Swizzle_Database_Entry(0x0061F85F, STRINGIZE(TagTypeClass::Load()), STRINGIZE(TagTypeClass::TriggerType), "TagType.cpp");
    Add_Swizzle_Database_Entry(0x00621D55, STRINGIZE(TaskForceClass::Load()), STRINGIZE(TagTypeClass::Members::Class), "TaskForce.cpp");
    Add_Swizzle_Database_Entry(0x006252A0, STRINGIZE(TeamClass::Load()), STRINGIZE(TeamClass::Class), "Team.cpp");
    Add_Swizzle_Database_Entry(0x006252AE, STRINGIZE(TeamClass::Load()), STRINGIZE(TeamClass::Script), "Team.cpp");
    Add_Swizzle_Database_Entry(0x006252BC, STRINGIZE(TeamClass::Load()), STRINGIZE(TeamClass::House), "Team.cpp");
    Add_Swizzle_Database_Entry(0x006252CA, STRINGIZE(TeamClass::Load()), STRINGIZE(TeamClass::Tag), "Team.cpp");
    Add_Swizzle_Database_Entry(0x006252D8, STRINGIZE(TeamClass::Load()), STRINGIZE(TeamClass::Member), "Team.cpp");
    Add_Swizzle_Database_Entry(0x006252E6, STRINGIZE(TeamClass::Load()), STRINGIZE(TeamClass::ClosestMember), "Team.cpp");
    Add_Swizzle_Database_Entry(0x006252F4, STRINGIZE(TeamClass::Load()), STRINGIZE(TeamClass::Target), "Team.cpp");
    Add_Swizzle_Database_Entry(0x00625302, STRINGIZE(TeamClass::Load()), STRINGIZE(TeamClass::Zone), "Team.cpp");
    Add_Swizzle_Database_Entry(0x00625310, STRINGIZE(TeamClass::Load()), STRINGIZE(TeamClass::MissionTarget), "Team.cpp");
    Add_Swizzle_Database_Entry(0x0062531E, STRINGIZE(TeamClass::Load()), STRINGIZE(TeamClass::field_20), "Team.cpp");
    Add_Swizzle_Database_Entry(0x0062532C, STRINGIZE(TeamClass::Load()), STRINGIZE(TeamClass::field_34), "Team.cpp");
    Add_Swizzle_Database_Entry(0x00628F12, STRINGIZE(TeamTypeClass::Load()), STRINGIZE(TeamTypeClass::House), "TeamType.cpp");
    Add_Swizzle_Database_Entry(0x00628F23, STRINGIZE(TeamTypeClass::Load()), STRINGIZE(TeamTypeClass::Tag), "TeamType.cpp");
    Add_Swizzle_Database_Entry(0x00628F34, STRINGIZE(TeamTypeClass::Load()), STRINGIZE(TeamTypeClass::Script), "TeamType.cpp");
    Add_Swizzle_Database_Entry(0x00628F45, STRINGIZE(TeamTypeClass::Load()), STRINGIZE(TeamTypeClass::TaskForce), "TeamType.cpp");
    Add_Swizzle_Database_Entry(0x00638DED, STRINGIZE(TechnoClass::Load()), STRINGIZE(TechnoClass::House), "Techno.cpp");
    Add_Swizzle_Database_Entry(0x00638E03, STRINGIZE(TechnoClass::Load()), STRINGIZE(TechnoClass::ParticleSystems), "Techno.cpp");
    Add_Swizzle_Database_Entry(0x00638E1A, STRINGIZE(TechnoClass::Load()), STRINGIZE(TechnoClass::ArchiveTarget), "Techno.cpp");
    Add_Swizzle_Database_Entry(0x00638E2B, STRINGIZE(TechnoClass::Load()), STRINGIZE(TechnoClass::field_20C), "Techno.cpp");
    Add_Swizzle_Database_Entry(0x00638E3C, STRINGIZE(TechnoClass::Load()), STRINGIZE(TechnoClass::TarCom), "Techno.cpp");
    Add_Swizzle_Database_Entry(0x00638E4D, STRINGIZE(TechnoClass::Load()), STRINGIZE(TechnoClass::SuspendedTarCom), "Techno.cpp");
    Add_Swizzle_Database_Entry(0x00638E5E, STRINGIZE(TechnoClass::Load()), STRINGIZE(TechnoClass::Cargo), "Techno.cpp");
    Add_Swizzle_Database_Entry(0x00638E6F, STRINGIZE(TechnoClass::Load()), STRINGIZE(TechnoClass::Wave), "Techno.cpp");
    Add_Swizzle_Database_Entry(0x0063DA03, STRINGIZE(TechnoTypeClass::Load()), STRINGIZE(TechnoTypeClass::DeploysInto), "TechnoType.cpp");
    Add_Swizzle_Database_Entry(0x0063DA14, STRINGIZE(TechnoTypeClass::Load()), STRINGIZE(TechnoTypeClass::UndeploysInto), "TechnoType.cpp");
    Add_Swizzle_Database_Entry(0x0063DA25, STRINGIZE(TechnoTypeClass::Load()), STRINGIZE(TechnoTypeClass::NaturalParticleSystem), "TechnoType.cpp");
    Add_Swizzle_Database_Entry(0x0063DD8D, STRINGIZE(TechnoTypeClass::Load()), STRINGIZE(TechnoTypeClass::Dock), "TechnoType.cpp");
    Add_Swizzle_Database_Entry(0x0063DDB8, STRINGIZE(TechnoTypeClass::Load()), STRINGIZE(TechnoTypeClass::Explosion), "TechnoType.cpp");
    Add_Swizzle_Database_Entry(0x0063DDE3, STRINGIZE(TechnoTypeClass::Load()), STRINGIZE(TechnoTypeClass::DebrisTypes), "TechnoType.cpp");
    Add_Swizzle_Database_Entry(0x0063DE04, STRINGIZE(TechnoTypeClass::Load()), STRINGIZE(TechnoTypeClass::Weapons), "TechnoType.cpp");
    Add_Swizzle_Database_Entry(0x0063DE2A, STRINGIZE(TechnoTypeClass::Load()), STRINGIZE(TechnoTypeClass::DamageParticleSystems), "TechnoType.cpp");
    Add_Swizzle_Database_Entry(0x006407C2, STRINGIZE(TerrainClass::Load()), STRINGIZE(TEventClass::Class), "Terrain.cpp");
    Add_Swizzle_Database_Entry(0x00642DDF, STRINGIZE(TEventClass::Load()), STRINGIZE(TEventClass::Next), "TEvent.cpp");
    Add_Swizzle_Database_Entry(0x00642DED, STRINGIZE(TEventClass::Load()), STRINGIZE(TEventClass::Team), "TEvent.cpp");
    Add_Swizzle_Database_Entry(0x00645175, STRINGIZE(TiberiumClass::Load()), STRINGIZE(TiberiumClass::Image), "Tiberium.cpp");
    Add_Swizzle_Database_Entry(0x006451E5, STRINGIZE(TiberiumClass::Load()), STRINGIZE(TiberiumClass::Debris[]), "Tiberium.cpp");
    Add_Swizzle_Database_Entry(0x006498AF, STRINGIZE(TriggerClass::Load()), STRINGIZE(TriggerClass::Class), "Trigger.cpp");
    Add_Swizzle_Database_Entry(0x006498BD, STRINGIZE(TriggerClass::Load()), STRINGIZE(TriggerClass::Next), "Trigger.cpp");
    Add_Swizzle_Database_Entry(0x0064AAEF, STRINGIZE(TriggerTypeClass::Load()), STRINGIZE(TriggerTypeClass::Next), "TrigType.cpp");
    Add_Swizzle_Database_Entry(0x0064AAFD, STRINGIZE(TriggerTypeClass::Load()), STRINGIZE(TriggerTypeClass::Event), "TrigType.cpp");
    Add_Swizzle_Database_Entry(0x0064AB0B, STRINGIZE(TriggerTypeClass::Load()), STRINGIZE(TriggerTypeClass::Action), "TrigType.cpp");
    Add_Swizzle_Database_Entry(0x0064AB19, STRINGIZE(TriggerTypeClass::Load()), STRINGIZE(TriggerTypeClass::House), "TrigType.cpp");
    Add_Swizzle_Database_Entry(0x00659723, STRINGIZE(UnitClass::Load()), STRINGIZE(UnitClass::Class), "Unit.cpp");
    Add_Swizzle_Database_Entry(0x00659734, STRINGIZE(UnitClass::Load()), STRINGIZE(UnitClass::FollowingMe), "Unit.cpp");
    Add_Swizzle_Database_Entry(0x0065EFD5, STRINGIZE(VoxelAnimClass::Load()), STRINGIZE(VoxelAnimClass::Class), "VoxelAnim.cpp");
    Add_Swizzle_Database_Entry(0x0065EFE6, STRINGIZE(VoxelAnimClass::Load()), STRINGIZE(VoxelAnimClass::AttachedParticleSys), "VoxelAnim.cpp");
    Add_Swizzle_Database_Entry(0x0065EFF7, STRINGIZE(VoxelAnimClass::Load()), STRINGIZE(VoxelAnimClass::House), "VoxelAnim.cpp");
    Add_Swizzle_Database_Entry(0x0065FEB6, STRINGIZE(VoxelAnimTypeClass::Load()), STRINGIZE(VoxelAnimTypeClass::Warhead), "VoxelAnimType.cpp");
    Add_Swizzle_Database_Entry(0x0065FEC7, STRINGIZE(VoxelAnimTypeClass::Load()), STRINGIZE(VoxelAnimTypeClass::BounceAnim), "VoxelAnimType.cpp");
    Add_Swizzle_Database_Entry(0x0065FED8, STRINGIZE(VoxelAnimTypeClass::Load()), STRINGIZE(VoxelAnimTypeClass::ExpireAnim), "VoxelAnimType.cpp");
    Add_Swizzle_Database_Entry(0x0065FEE9, STRINGIZE(VoxelAnimTypeClass::Load()), STRINGIZE(VoxelAnimTypeClass::TrailerAnim), "VoxelAnimType.cpp");
    Add_Swizzle_Database_Entry(0x0065FEFA, STRINGIZE(VoxelAnimTypeClass::Load()), STRINGIZE(VoxelAnimTypeClass::Spawns), "VoxelAnimType.cpp");
    Add_Swizzle_Database_Entry(0x0065FF0B, STRINGIZE(VoxelAnimTypeClass::Load()), STRINGIZE(VoxelAnimTypeClass::AttachedSystem), "VoxelAnimType.cpp");
    Add_Swizzle_Database_Entry(0x0066F871, STRINGIZE(WarheadTypeClass::Load()), STRINGIZE(WarheadTypeClass::AnimList), "WarheadType.cpp");
    Add_Swizzle_Database_Entry(0x0066F88D, STRINGIZE(WarheadTypeClass::Load()), STRINGIZE(WarheadTypeClass::Particle), "WarheadType.cpp");
    Add_Swizzle_Database_Entry(0x00670BCB, STRINGIZE(WaveClass::Load()), STRINGIZE(WarheadTypeClass::field_F0), "Wave.cpp");
    Add_Swizzle_Database_Entry(0x00670BDC, STRINGIZE(WaveClass::Load()), STRINGIZE(WaveClass::Source), "Wave.cpp");
    Add_Swizzle_Database_Entry(0x00670C8B, STRINGIZE(WaveClass::Load()), STRINGIZE(WaveClass::Cells), "Wave.cpp");
    Add_Swizzle_Database_Entry(0x006816DC, STRINGIZE(WeaponTypeClass::Load()), STRINGIZE(WeaponTypeClass::WarheadPtr), "WeaponType.cpp");
    Add_Swizzle_Database_Entry(0x006816EA, STRINGIZE(WeaponTypeClass::Load()), STRINGIZE(WeaponTypeClass::Bullet), "WeaponType.cpp");
    Add_Swizzle_Database_Entry(0x006816FB, STRINGIZE(WeaponTypeClass::Load()), STRINGIZE(WeaponTypeClass::AttachedParticleSystem), "WeaponType.cpp");
    Add_Swizzle_Database_Entry(0x0068171C, STRINGIZE(WeaponTypeClass::Load()), STRINGIZE(WeaponTypeClass::Anim), "WeaponType.cpp");

    // Fetch_Swizzle_ID
    Add_Swizzle_Database_Entry(0x004CE3C8, STRINGIZE(HouseTypeClass::Save()), "this", "HouseType.cpp");
}


/**
 *  Searches for a database entry from the input return address.
 * 
 *  @author: tomsons26
 */
static SwizzleInfoDatabaseEntry *Swizzle_Find_Database_Entry(uintptr_t retaddr)
{
    for (int i = 0; i < SwizzleInfoDatabase.Count(); ++i) {
        if (SwizzleInfoDatabase[i].ReturnAddress == retaddr) {
            return &SwizzleInfoDatabase[i];
        }
    }

    //DEV_DEBUG_WARNING("0x%p was not found in the Swizzle database!\n", retaddr);

    return nullptr;
}


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or deconstructor.
 * 
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
static class SwizzleManagerClassExt final : public ViniferaSwizzleManagerClass
{
    public:
        COM_DECLSPEC_NOTHROW LONG STDAPICALLTYPE _Reset();
        COM_DECLSPEC_NOTHROW LONG STDAPICALLTYPE _Swizzle(void **pointer);
        COM_DECLSPEC_NOTHROW LONG STDAPICALLTYPE _Fetch_Swizzle_ID(void *pointer, LONG *id);
        COM_DECLSPEC_NOTHROW LONG STDAPICALLTYPE _Here_I_Am(LONG id, void *pointer);
};


/**
 *  Wrapper for Here_I_Am to call the new Swizzle manager with attached debug information.
 * 
 *  @author: CCHyper
 */
LONG STDAPICALLTYPE SwizzleManagerClassExt::_Here_I_Am(LONG id, void *pointer)
{
    //DEV_DEBUG_INFO("SwizzleManager::Here_I_Am - retaddr 0x%08X id 0x%08X pointer 0x%08X\n", (uintptr_t)_ReturnAddress(), id, pointer);

    /**
     *  Get the caller return address, we use this to identify a location in which the annoucement was made.
     */
    uintptr_t retaddr = (uintptr_t)_ReturnAddress();

    /**
     *  Fetch the caller debug information based off the return address.
     */
    SwizzleInfoDatabaseEntry *info = Swizzle_Find_Database_Entry(retaddr);
    if (!info) {
        DEV_DEBUG_WARNING("Here_I_Am() - Failed to find debug information for 0x%p!\n", retaddr);

#ifdef VINIFERA_ENABLE_SWIZZLE_DEBUG_PRINTING
        // Assert so we can investigate.
        ASSERT(false);
#endif
        
        /**
         *  Return failure!
         */
        return E_UNEXPECTED;

#ifdef VINIFERA_ENABLE_SWIZZLE_DEBUG_PRINTING
    } else {
        DEV_DEBUG_INFO("Here_I_Am() - Debug info found:\n  File: %s\n  Line: %d\n  Function: %s\n  Var: %s\n", info->File, info->Line, info->Function, info->Variable);
#endif
    }

    return Here_I_Am_Dbg(id, pointer, info->File, info->Line, info->Function, info->Variable);
}


/**
 *  Wrapper for Reset to call the new Swizzle manager with attached debug information.
 * 
 *  @author: CCHyper
 */
LONG STDAPICALLTYPE SwizzleManagerClassExt::_Reset()
{
    //DEV_DEBUG_INFO("SwizzleManager::Reset - retaddr 0x%08X id 0x%08X pointer 0x%08X\n", (uintptr_t)_ReturnAddress(), id, pointer);

#ifdef VINIFERA_ENABLE_SWIZZLE_DEBUG_PRINTING
    /**
     *  Get the caller return address, we use this to identify a location in which the request was made.
     */
    uintptr_t retaddr = (uintptr_t)_ReturnAddress();

    switch (retaddr) {
        case 0x005D69C7:
            DEV_DEBUG_INFO("Reset() - From Load_Game\n");
            break;
        case 0x005DD668:
            DEV_DEBUG_INFO("Reset() - From Read_Scenario_INI\n");
            break;
    };
#endif

    return Reset();
}


/**
 *  Wrapper for Swizzle to call the new Swizzle manager with attached debug information.
 * 
 *  @author: CCHyper
 */
LONG STDAPICALLTYPE SwizzleManagerClassExt::_Swizzle(void **pointer)
{
    //DEV_DEBUG_INFO("SwizzleManager::Swizzle - retaddr 0x%08X id 0x%08X pointer 0x%08X\n", (uintptr_t)_ReturnAddress(), id, pointer);

    /**
     *  Get the caller return address, we use this to identify a location in which the request was made.
     */
    uintptr_t retaddr = (uintptr_t)_ReturnAddress();

    /**
     *  Fetch the caller debug information based off the return address.
     */
    SwizzleInfoDatabaseEntry *info = Swizzle_Find_Database_Entry(retaddr);
    if (!info) {
        DEV_DEBUG_WARNING("Swizzle() - Failed to find debug information for 0x%p!\n", retaddr);

#ifdef VINIFERA_ENABLE_SWIZZLE_DEBUG_PRINTING
        // Assert so we can investigate.
        ASSERT(false);
#endif
        
        /**
         *  Return failure!
         */
        return E_UNEXPECTED;

#ifdef VINIFERA_ENABLE_SWIZZLE_DEBUG_PRINTING
    } else {
        DEV_DEBUG_INFO("Swizzle() - Debug info found:\n  File: %s\n  Line: %d\n  Function: %s\n  Var: %s\n", info->File, info->Line, info->Function, info->Variable);
#endif
    }

    return Swizzle_Dbg(pointer, info->File, info->Line, info->Function, info->Variable);
}


/**
 *  Wrapper for Fetch_Swizzle_ID to call the new Swizzle manager with attached debug information.
 * 
 *  @author: CCHyper
 */
LONG STDAPICALLTYPE SwizzleManagerClassExt::_Fetch_Swizzle_ID(void *pointer, LONG *id)
{
    //DEV_DEBUG_INFO("SwizzleManager::Fetch_Swizzle_ID - retaddr 0x%08X id 0x%08X pointer 0x%08X\n", (uintptr_t)_ReturnAddress(), id, pointer);

    /**
     *  Get the caller return address, we use this to identify a location in which the request was made.
     */
    uintptr_t retaddr = (uintptr_t)_ReturnAddress();

    /**
     *  Fetch the caller debug information based off the return address.
     */
    SwizzleInfoDatabaseEntry *info = Swizzle_Find_Database_Entry(retaddr);
    if (!info) {
        DEV_DEBUG_WARNING("Fetch_Swizzle_ID() - Failed to find debug information for 0x%p!\n", retaddr);

#ifdef VINIFERA_ENABLE_SWIZZLE_DEBUG_PRINTING
        // Assert so we can investigate.
        ASSERT(false);
#endif
        
        /**
         *  Return failure!
         */
        return E_UNEXPECTED;

#ifdef VINIFERA_ENABLE_SWIZZLE_DEBUG_PRINTING
    } else {
        DEV_DEBUG_INFO("Fetch_Swizzle_ID() - Debug info found:\n  File: %s\n  Line: %d\n  Function: %s\n  Var: %s\n", info->File, info->Line, info->Function, info->Variable);
#endif
    }

    return Fetch_Swizzle_ID_Dbg(pointer, id, info->File, info->Line, info->Function, info->Variable);
}


/**
 *  Replacement functions for the dynamic initialisers to replace SwizzleManager with SwizzleManagerClassExt.
 */
//static void __cdecl SwizzleManagerClassExt_atexit() { reinterpret_cast<SwizzleManagerClassExt &>(SwizzleManager).SwizzleManagerClassExt::~SwizzleManagerClassExt(); }
static void __cdecl SwizzleManagerClassExt_dyn_init() { new (&SwizzleManager) SwizzleManagerClassExt; } //std::atexit(SwizzleManagerClassExt_atexit); }

#endif


void NewSwizzle_Hooks()
{
#ifdef VINIFERA_USE_NEW_SWIZZLE_MANAGER

    /**
     *  Build the database of debug info.
     */
    Build_Swizzle_Address_Database();

    /**
     *  Replaces dynamic inits for original SwizzleManager global.
     */
    Hook_Function(0x0060D8A0, &SwizzleManagerClassExt_dyn_init);
    //Patch_Byte(0x0060D920, 0xC3); // retn, voids the original atexit.

    /**
     *  Replace the implementation of SwizzleManagerClass with our own implementation.
     */
    Patch_Jump(0x0060DA60, &SwizzleManagerClassExt::_Reset);
    Patch_Jump(0x0060DA70, &SwizzleManagerClassExt::_Swizzle);
    Patch_Jump(0x0060DCC0, &SwizzleManagerClassExt::_Fetch_Swizzle_ID);
    Patch_Jump(0x0060DAF0, &SwizzleManagerClassExt::_Here_I_Am);

#endif
}

