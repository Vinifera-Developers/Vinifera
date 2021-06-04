/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          COMMANDEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended hotkey command class.
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
#include "commandext.h"
#include "tibsun_globals.h"
#include "tibsun_util.h"
#include "vinifera_globals.h"
#include "vinifera_util.h"
#include "iomap.h"
#include "dsurface.h"
#include "wwmouse.h"
#include "rules.h"
#include "house.h"
#include "super.h"
#include "anim.h"
#include "animtype.h"
#include "unit.h"
#include "unittype.h"
#include "infantry.h"
#include "infantrytype.h"
#include "building.h"
#include "buildingtype.h"
#include "aircraft.h"
#include "aircrafttype.h"
#include "warheadtype.h"
#include "session.h"
#include "ionstorm.h"
#include "ionblast.h"
#include "combat.h"
#include "scenarioini.h"
#include "wwcrc.h"
#include "filepcx.h"
#include "filepng.h"
#include "fatal.h"
#include "minidump.h"
#include "winutil.h"
#include "miscutil.h"
#include "debughandler.h"
#include "asserthandler.h"


/**
 *  Handy defines for handling any adjustments.
 */
#define CATEGORY_DEVELOPER "Developer"


/**
 *  #issue-167
 * 
 *  Writes a PNG screenshot of the current screen buffer.
 * 
 *  @author: CCHyper
 */
const char *PNGScreenCaptureCommandClass::Get_Name() const
{
    return "ScreenCapture";
}

const char *PNGScreenCaptureCommandClass::Get_UI_Name() const
{
    return "Screen Capture";
}

const char *PNGScreenCaptureCommandClass::Get_Category() const
{
    return "Interface";
}

const char *PNGScreenCaptureCommandClass::Get_Description() const
{
    return "Takes a snapshot of the game screen (Saved as 'SCRN_<date-time>.PNG.)";
}

bool PNGScreenCaptureCommandClass::Process()
{
    if (!IsWindow(MainWindow)) {
        return false;
    }

    RECT crect;
    if (!GetClientRect(MainWindow, &crect)) {
        return false;
    }

    POINT tl_point;
    tl_point.x = crect.left;
    tl_point.y = crect.top;
    if (!ClientToScreen(MainWindow, &tl_point)) {
        return false;
    }

    POINT br_point;
    br_point.x = crect.right;
    br_point.y = crect.bottom;
    if (!ClientToScreen(MainWindow, &br_point)) {
        return false;
    }

    int w = std::min((int)crect.right+1, HiddenSurface->Get_Width());
    int h = std::min((int)crect.bottom+1, HiddenSurface->Get_Height());

    Rect src(tl_point.x, tl_point.y, w, h);
    Rect dest(0, 0, HiddenSurface->Get_Width(), HiddenSurface->Get_Height());

    /**
     *  We don't want the mouse to appear in screenshots!
     */
    WWMouse->Hide_Mouse();

    /**
     *  Blit primary surface to the hidden.
     */
    bool blit = HiddenSurface->Copy_From(dest, *PrimarySurface, src);
    ASSERT(blit);

    /**
     *  Now show the mouse again.
     */
    WWMouse->Show_Mouse();

    char buffer[256];

#if 0
    /**
     *  Find a free filename slot.
     */
    for (unsigned i = 0; i <= 9999; ++i) {
        std::snprintf(buffer, sizeof(buffer), "SCRN%04d.PNG", i);
        if (!RawFileClass(buffer).Is_Available()) {
            break;
        }
    }
#endif

    /**
     *  Generate a unique filename with the current timestamp.
     */
    int day = 0;
    int month = 0;
    int year = 0;
    int hour = 0;
    int min = 0;
    int sec = 0;
    Get_Full_Time(day, month, year, hour, min, sec);
    std::snprintf(buffer, sizeof(buffer), "SCRN_%02u-%02u-%04u_%02u-%02u-%02u.PNG", day, month, year, hour, min, sec);

    /**
     *  We found a free filename, now write the buffer to a PNG file.
     */
    bool success = Write_PNG_File(&RawFileClass(buffer), *HiddenSurface, &GamePalette);

    if (success) {
        DEBUG_INFO("PNG screenshot \"%s\" written sucessfully.\n", buffer);
    } else {
        DEBUG_ERROR("Failed to write PNG screenshot \"%s\"!\n", buffer);
    }

    return success;
}


/**
 *  Produces a memory dump on request.
 * 
 *  @author: CCHyper
 */
const char *MemoryDumpCommandClass::Get_Name() const
{
    return "MemoryDump";
}

const char *MemoryDumpCommandClass::Get_UI_Name() const
{
    return "Memory Dump";
}

const char *MemoryDumpCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *MemoryDumpCommandClass::Get_Description() const
{
    return "Produces a mini-dump of the memory for analysis.";
}

bool MemoryDumpCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    DEBUG_INFO("About to produce memory dump...\n");

    GenerateFullCrashDump = false; // We don't need a full memory dump.
    NonFatalMinidump = true;
    MinidumpUseCurrentTime = true;

    Create_Mini_Dump(nullptr, Get_Module_File_Name());

    return true;
}


/**
 *  Dumps all the current game objects as CRCs to the log output.
 * 
 *  @author: CCHyper
 */
const char *DumpHeapCRCCommandClass::Get_Name() const
{
    return "DumpHeapCRC";
}

const char *DumpHeapCRCCommandClass::Get_UI_Name() const
{
    return "Dump Heap CRCs";
}

const char *DumpHeapCRCCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *DumpHeapCRCCommandClass::Get_Description() const
{
    return "Dumps all the current game objects as CRCs to the log output.";
}

/**
 *  Handy macro for defining the logging the heaps CRCs.
 * 
 *  @author: CCHyper
 */
#define LOG_CRC(class_name, heap_name) \
    { \
        DEBUG_INFO(#class_name ":\n"); \
        if (!heap_name.Count()) { \
            DEBUG_INFO("  EMPTY\n"); \
        } else { \
            WWCRCEngine crc; \
            for (unsigned i = 0; i < heap_name.Count(); ++i) { \
                class_name *ptr = heap_name[i]; \
                if (ptr != nullptr) { \
                    ptr->Compute_CRC(crc); \
                    DEBUG_INFO("  %04d\tCRC: 0x%08X\n", i, crc.CRC_Value()); \
                } else { \
                    DEBUG_INFO("  %04d\tFAILED!\n", i); \
                } \
            } \
        } \
        DEBUG_INFO("\n"); \
    }

bool DumpHeapCRCCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    DEBUG_INFO("\nAbout to dump heap CRC's...\n\n");

    LOG_CRC(UnitClass, Units);
    LOG_CRC(InfantryClass, Infantry);
    LOG_CRC(BuildingClass, Buildings);
    LOG_CRC(AircraftClass, Aircrafts);

    LOG_CRC(UnitTypeClass, UnitTypes);
    LOG_CRC(InfantryTypeClass, InfantryTypes);
    LOG_CRC(BuildingTypeClass, BuildingTypes);
    LOG_CRC(AircraftTypeClass, AircraftTypes);
    
    DEBUG_INFO("\nFinished!\n\n");

    return true;
}


/**
 *  Toggles the instant build cheat for the player.
 * 
 *  @author: CCHyper
 */
const char *InstantBuildCommandClass::Get_Name() const
{
    return "InstantBuild";
}

const char *InstantBuildCommandClass::Get_UI_Name() const
{
    return "Instant Build (Player)";
}

const char *InstantBuildCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *InstantBuildCommandClass::Get_Description() const
{
    return "Toggles the instant build cheat for the player.";
}

bool InstantBuildCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    Vinifera_Developer_InstantBuild = !Vinifera_Developer_InstantBuild;

    return true;
}


/**
 *  Toggles the instant build cheat for the AI.
 *  
 *  @note: This will effect ALL the AI houses currently in the game session!
 * 
 *  @author: CCHyper
 */
const char *AIInstantBuildCommandClass::Get_Name() const
{
    return "AIInstantBuild";
}

const char *AIInstantBuildCommandClass::Get_UI_Name() const
{
    return "Instant Build (AI)";
}

const char *AIInstantBuildCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *AIInstantBuildCommandClass::Get_Description() const
{
    return "Toggles the instant build cheat for the AI.";
}

bool AIInstantBuildCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    Vinifera_Developer_AIInstantBuild = !Vinifera_Developer_AIInstantBuild;

    return true;
}


/**
 *  Forces the player to win the current game session.
 * 
 *  @author: CCHyper
 */
const char *ForceWinCommandClass::Get_Name() const
{
    return "ForceWin";
}

const char *ForceWinCommandClass::Get_UI_Name() const
{
    return "To Win";
}

const char *ForceWinCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ForceWinCommandClass::Get_Description() const
{
    return "Forces the player to win the current game session.";
}

bool ForceWinCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    /**
     *  Player wins.
     */
    return PlayerPtr->Flag_To_Win();
}


/**
 *  Forces the player to lose the current game session.
 * 
 *  @author: CCHyper
 */
const char *ForceLoseCommandClass::Get_Name() const
{
    return "ForceLose";
}

const char *ForceLoseCommandClass::Get_UI_Name() const
{
    return "To Lose";
}

const char *ForceLoseCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ForceLoseCommandClass::Get_Description() const
{
    return "Forces the player to lose the current game session.";
}

bool ForceLoseCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    /**
     *  Player loses
     */
    return PlayerPtr->Flag_To_Lose();
}


/**
 *  Forces the player to blowup and lose the current game session.
 * 
 *  @author: CCHyper
 */
const char *ForceDieCommandClass::Get_Name() const
{
    return "ForceDie";
}

const char *ForceDieCommandClass::Get_UI_Name() const
{
    return "To Die";
}

const char *ForceDieCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ForceDieCommandClass::Get_Description() const
{
    return "Forces the player to blowup, loosing the current game session.";
}

bool ForceDieCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    /**
     *  Player dies.
     */
    return PlayerPtr->Flag_To_Die();
}


/**
 *  Take ownership of any selected objects.
 * 
 *  @author: CCHyper
 */
const char *CaptureObjectCommandClass::Get_Name() const
{
    return "CaptureObject";
}

const char *CaptureObjectCommandClass::Get_UI_Name() const
{
    return "Capture Object";
}

const char *CaptureObjectCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *CaptureObjectCommandClass::Get_Description() const
{
    return "Take ownership of any selected objects.";
}

bool CaptureObjectCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    /**
     *  Iterate over all currently selected objects and take ownership of them.
     */
    for (int i = 0; i < CurrentObjects.Count(); ++i) {
        ObjectClass * object = CurrentObjects[i];
        if (!object || !object->Is_Techno()) {
            continue;
        }

        /**
         *  We own this object already, skip it.
         */
        if (object->Owning_House() == PlayerPtr) {
            continue;
        }

        TechnoClass *techno = dynamic_cast<TechnoClass *>(object);
        techno->Captured(PlayerPtr);
    }

    Map.Recalc();

    return true;
}


/**
 *  Grants all available special weapons to the player.
 * 
 *  @author: CCHyper
 */
const char *SpecialWeaponsCommandClass::Get_Name() const
{
    return "SpecialWeapons";
}

const char *SpecialWeaponsCommandClass::Get_UI_Name() const
{
    return "Special Weapons";
}

const char *SpecialWeaponsCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *SpecialWeaponsCommandClass::Get_Description() const
{
    return "Grants all available special weapons to the player.";
}

bool SpecialWeaponsCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    /**
     *  Iterate over all the special weapon slots for the player house
     *  and make them all available, fully charged!
     */
    for (SpecialWeaponType i = SPECIAL_FIRST; i < SuperWeaponTypes.Count(); ++i) {

        PlayerPtr->SuperWeapon[i]->Enable(true, true, true);
        PlayerPtr->SuperWeapon[i]->Forced_Charge(true);
        Map.Add(RTTI_SPECIAL, i);

        /**
         *  Redraw the right column.
         */
        Map.Column[1].Flag_To_Redraw();
    }

    return true;
}


/**
 *  Hands out free money to the player.
 * 
 *  @author: CCHyper
 */
const char *FreeMoneyCommandClass::Get_Name() const
{
    return "FreeMoney";
}

const char *FreeMoneyCommandClass::Get_UI_Name() const
{
    return "Free Money";
}

const char *FreeMoneyCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *FreeMoneyCommandClass::Get_Description() const
{
    return "Gives free money to the player.";
}

bool FreeMoneyCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    /**
     *  Give 10,000 credits to the player.
     */
    PlayerPtr->Refund_Money(10000);

    return true;
}


/**
 *  Fires a lightning bolt at the current mouse cursor location.
 * 
 *  @author: CCHyper
 */
const char *LightningBoltCommandClass::Get_Name() const
{
    return "LightningBolt";
}

const char *LightningBoltCommandClass::Get_UI_Name() const
{
    return "Lightning Bolt";
}

const char *LightningBoltCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *LightningBoltCommandClass::Get_Description() const
{
    return "Fires a lightning bolt at the current mouse location.";
}

bool LightningBoltCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    Cell mouse_cell = Get_Cell_Under_Mouse();

    IonStorm_Lightning_Strike_At(mouse_cell);

    return true;
}


/**
 *  Fires an ion blast bolt at the current mouse cursor location.
 * 
 *  @author: CCHyper
 */
const char *IonBlastCommandClass::Get_Name() const
{
    return "IonBlast";
}

const char *IonBlastCommandClass::Get_UI_Name() const
{
    return "Ion Blast";
}

const char *IonBlastCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *IonBlastCommandClass::Get_Description() const
{
    return "Fires an ion blast bolt at the current mouse location.";
}

bool IonBlastCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    Coordinate mouse_coord = Get_Coord_Under_Mouse();
    mouse_coord.Z = Map.Get_Cell_Height(mouse_coord);

    new IonBlastClass(mouse_coord);

    return true;
}


/**
 *  Spawns an explosion at the mouse cursor location.
 * 
 *  @author: CCHyper
 */
const char *ExplosionCommandClass::Get_Name() const
{
    return "Explosion";
}

const char *ExplosionCommandClass::Get_UI_Name() const
{
    return "Explosion";
}

const char *ExplosionCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ExplosionCommandClass::Get_Description() const
{
    return "Spawns a explosion at the mouse location.";
}

bool ExplosionCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    Coordinate mouse_coord = Get_Coord_Under_Mouse();
    mouse_coord.Z = Map.Get_Cell_Height(mouse_coord);

    const CellClass *cellptr = &Map[mouse_coord];
    if (!cellptr) {
        return false;
    }

    /**
     *  The damage to deal at the coord.
     */
    int damage = Rule->MaxDamage;

    /**
     *  Pick a random warhead from the list, using C4Warhead as a backup.
     */
    const WarheadTypeClass *warheadtypeptr = WarheadTypeClass::As_Pointer(Percent_Chance(50) ? "AP" : "HE");
    if (!warheadtypeptr) {
        warheadtypeptr = Rule->C4Warhead;
    }

    /**
     *  What anim should we use for this criteria.
     */
    const AnimTypeClass *cellanim = Combat_Anim(damage, warheadtypeptr, cellptr->Land_Type(), &mouse_coord);
    if (!cellanim) {
        return false;
    }

    new AnimClass(cellanim, mouse_coord);

    Explosion_Damage(&mouse_coord, damage, nullptr, warheadtypeptr);

    return true;
}


/**
 *  Spawns a large explosion at the mouse cursor location.
 * 
 *  @author: CCHyper
 */
const char *SuperExplosionCommandClass::Get_Name() const
{
    return "SuperExplosion";
}

const char *SuperExplosionCommandClass::Get_UI_Name() const
{
    return "Super Explosion";
}

const char *SuperExplosionCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *SuperExplosionCommandClass::Get_Description() const
{
    return "Spawns a large explosion at the mouse location.";
}

bool SuperExplosionCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    Coordinate mouse_coord = Get_Coord_Under_Mouse();
    mouse_coord.Z = Map.Get_Cell_Height(mouse_coord);

    const CellClass *cellptr = &Map[mouse_coord];
    if (!cellptr) {
        return false;
    }

    /**
     *  The damage to deal at the coord.
     */
    int damage = Rule->MaxDamage;

    /**
     *  Pick a random warhead from the list, using C4Warhead as a backup.
     */
    const WarheadTypeClass *warheadtypeptr = WarheadTypeClass::As_Pointer("Super");
    if (!warheadtypeptr) {
        warheadtypeptr = Rule->C4Warhead;
    }

    /**
     *  What anim should we use for this criteria.
     */
    const AnimTypeClass *cellanim = Combat_Anim(damage, warheadtypeptr, cellptr->Land_Type(), &mouse_coord);
    if (!cellanim) {
        return false;
    }

    new AnimClass(cellanim, mouse_coord);

    Explosion_Damage(&mouse_coord, damage, nullptr, warheadtypeptr);

    return true;
}


/**
 *  Exits the game completely.
 * 
 *  @author: CCHyper
 */
const char *BailOutCommandClass::Get_Name() const
{
    return "BailOut";
}

const char *BailOutCommandClass::Get_UI_Name() const
{
    return "Bail Out";
}

const char *BailOutCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *BailOutCommandClass::Get_Description() const
{
    return "Exits the game to the desktop.";
}

bool BailOutCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    DEBUG_WARNING("Bail out!");
    Fatal("Bail out!");

    return true;
}


/**
 *  Toggles the ion storm on/off.
 * 
 *  @author: CCHyper
 */
const char *IonStormCommandClass::Get_Name() const
{
    return "IonStorm";
}

const char *IonStormCommandClass::Get_UI_Name() const
{
    return "Ion Storm";
}

const char *IonStormCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *IonStormCommandClass::Get_Description() const
{
    return "Toggles an ion storm on/off.";
}

bool IonStormCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    if (IonStorm_Is_Active()) {
        IonStorm_Stop();
    } else {
        IonStorm_Start(TICKS_PER_SECOND * Rule->IonStormDuration/*, TICKS_PER_SECOND * Rule->IonStormWarning*/); // No warning (instant).
    }

    return true;
}


/**
 *  Saves a snapshot of the current scenario state.
 * 
 *  @author: CCHyper
 */
const char *MapSnapshotCommandClass::Get_Name() const
{
    return "MapSnapshot";
}

const char *MapSnapshotCommandClass::Get_UI_Name() const
{
    return "Scenario Snapshot";
}

const char *MapSnapshotCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *MapSnapshotCommandClass::Get_Description() const
{
    return "Saves a snapshot of the current scenario state (Saved as 'SCEN_<date-time>.MAP.).";
}

bool MapSnapshotCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    char buffer[128];

    /**
     *  Generate a unique filename with the current timestamp.
     */
    int day = 0;
    int month = 0;
    int year = 0;
    int hour = 0;
    int min = 0;
    int sec = 0;
    Get_Full_Time(day, month, year, hour, min, sec);
    std::snprintf(buffer, sizeof(buffer), "SCEN_%02u-%02u-%04u_%02u-%02u-%02u.MAP", day, month, year, hour, min, sec);

    DEBUG_INFO("Saving map snapshot...");

    Write_Scenario_INI(buffer);
    
    DEBUG_INFO(" COMPLETE!\n");

    DEBUG_INFO("Filename: %s\n", buffer);

    return true;
}
