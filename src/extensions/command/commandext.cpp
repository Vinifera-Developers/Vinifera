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
#include "commandext_functions.h"
#include "tibsun_globals.h"
#include "tibsun_util.h"
#include "vinifera_globals.h"
#include "vinifera_util.h"
#include "iomap.h"
#include "tactical.h"
#include "dsurface.h"
#include "wwmouse.h"
#include "rules.h"
#include "house.h"
#include "housetype.h"
#include "base.h"
#include "super.h"
#include "factory.h"
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
#include "weapontype.h"
#include "warheadtype.h"
#include "session.h"
#include "ionstorm.h"
#include "ionblast.h"
#include "tiberium.h"
#include "combat.h"
#include "scenarioini.h"
#include "scenario.h"
#include "vox.h"
#include "language.h"
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
     *  #issue-195
     * 
     *  Output screenshots to its own sub-directory.
     * 
     *  @author: CCHyper
     */
    char fullpath_buffer[PATH_MAX];
    std::snprintf(fullpath_buffer, sizeof(fullpath_buffer), "%s\\%s", Vinifera_ScreenshotDirectory, buffer);

    /**
     *  We found a free filename, now write the buffer to a PNG file.
     */
    bool success = Write_PNG_File(&RawFileClass(fullpath_buffer), *HiddenSurface, &GamePalette);

    if (success) {
        DEBUG_INFO("PNG screenshot \"%s\" written sucessfully.\n", buffer);
    } else {
        DEBUG_ERROR("Failed to write PNG screenshot \"%s\"!\n", buffer);
    }

    return success;
}


/**
 *  #issue-112
 * 
 *  Enter the manual placement mode when a building is complete
 *  and pending placement on the sidebar.
 * 
 *  @author: CCHyper (based on research by dkeeton)
 */
const char *ManualPlaceCommandClass::Get_Name() const
{
    return "ManualPlace";
}

const char *ManualPlaceCommandClass::Get_UI_Name() const
{
    return "Place Building";
}

const char *ManualPlaceCommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char *ManualPlaceCommandClass::Get_Description() const
{
    return "Enter the manual placement mode when a building is complete and pending on the sidebar.";
}

bool ManualPlaceCommandClass::Process()
{
    if (!PlayerPtr) {
        return false;
    }

    /**
     *  Fetch the houses factory associated with producing buildings.
     */
    FactoryClass *factory = PlayerPtr->Fetch_Factory(RTTI_BUILDING);
    if (!factory) {
        DEV_DEBUG_WARNING("ManualPlaceCommand - Unable to fetch primary factory!\n");
        return false;
    }

    /**
     *  If this object is still being built, then bail.
     */
    if (!factory->Has_Completed()) {
        DEV_DEBUG_WARNING("ManualPlaceCommand - Factory object has not yet completed production!\n");

        /**
         *  #issue-537
         * 
         *  Removed due to them being a nuisance to the player.
         */
        //Speak(VOX_NO_FACTORY);
        return false;
    }

    TechnoClass *pending = factory->Get_Object();

    /**
     *  If by some rare chance the product is not a building, then bail.
     */
    if (pending->What_Am_I() != RTTI_BUILDING) {
        DEV_DEBUG_ERROR("ManualPlaceCommand - Factory object is not a building!\n");

        /**
         *  #issue-537
         * 
         *  Removed due to them being a nuisance to the player.
         */
        //Speak(VOX_NO_FACTORY);
        return false;
    }

    BuildingClass *pending_bptr = reinterpret_cast<BuildingClass *>(pending);

    /**
     *  Fetch the factory building that can build this object.
     */
    BuildingClass *builder = pending_bptr->Who_Can_Build_Me();
    if (!builder) {
        DEV_DEBUG_ERROR("ManualPlaceCommand - No builder available!\n");

        /**
         *  #issue-537
         * 
         *  Removed due to them being a nuisance to the player.
         */
        //Speak(VOX_NO_FACTORY);
        return false;
    }

    /**
     *  Are we already trying to place this building? No need to re-enter placement mode...
     */
    if (Map.PendingObjectPtr == pending_bptr) {
        return true;
    }

    DEV_DEBUG_INFO("ManualPlaceCommand - Entering placement mode with \"%s\"\n", pending_bptr->Full_Name());

    /**
     *  Go into placement mode.
     */
    return PlayerPtr->Manual_Place(builder, pending_bptr);
}


/**
 *  Skip to the previous playable music track.
 * 
 *  @author: CCHyper
 */
const char *PrevThemeCommandClass::Get_Name() const
{
    return "PrevTheme";
}

const char *PrevThemeCommandClass::Get_UI_Name() const
{
    return "Music: Previous Track";
}

const char *PrevThemeCommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char *PrevThemeCommandClass::Get_Description() const
{
    return "Play the previous music track in the jukebox.";
}

bool PrevThemeCommandClass::Process()
{
    Prev_Theme_Command();

    return true;
}


/**
 *  Skip to the next playable music track.
 * 
 *  @author: CCHyper
 */
const char *NextThemeCommandClass::Get_Name() const
{
    return "NextTheme";
}

const char *NextThemeCommandClass::Get_UI_Name() const
{
    return "Music: Next Track";
}

const char *NextThemeCommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char *NextThemeCommandClass::Get_Description() const
{
    return "Play the next music track in the jukebox.";
}

bool NextThemeCommandClass::Process()
{
    Next_Theme_Command();

    return true;
}


/**
 *  Scroll tactical map to the north-east.
 * 
 *  @author: CCHyper
 */
const char *ScrollNECommandClass::Get_Name() const
{
    return "ScrollNorthEast";
}

const char *ScrollNECommandClass::Get_UI_Name() const
{
    return "Scroll North-East";
}

const char *ScrollNECommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char *ScrollNECommandClass::Get_Description() const
{
    return "Scroll tactical map to the north-east.";
}

bool ScrollNECommandClass::Process()
{
    int dist = 34;

    Map.Scroll_Map(FACING_NE, dist);

    return true;
}


/**
 *  Scroll tactical map to the south-east.
 * 
 *  @author: CCHyper
 */
const char *ScrollSECommandClass::Get_Name() const
{
    return "ScrollSouthEast";
}

const char *ScrollSECommandClass::Get_UI_Name() const
{
    return "Scroll South-East";
}

const char *ScrollSECommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char *ScrollSECommandClass::Get_Description() const
{
    return "Scroll tactical map to the south-east.";
}

bool ScrollSECommandClass::Process()
{
    int dist = 34;

    Map.Scroll_Map(FACING_SE, dist);

    return true;
}


/**
 *  Scroll tactical map to the south-west.
 * 
 *  @author: CCHyper
 */
const char *ScrollSWCommandClass::Get_Name() const
{
    return "ScrollSouthWest";
}

const char *ScrollSWCommandClass::Get_UI_Name() const
{
    return "Scroll South-West";
}

const char *ScrollSWCommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char *ScrollSWCommandClass::Get_Description() const
{
    return "Scroll tactical map to the south-west.";
}

bool ScrollSWCommandClass::Process()
{
    int dist = 34;

    Map.Scroll_Map(FACING_SW, dist);

    return true;
}


/**
 *  Scroll tactical map to the north-west.
 * 
 *  @author: CCHyper
 */
const char *ScrollNWCommandClass::Get_Name() const
{
    return "ScrollNorthWest";
}

const char *ScrollNWCommandClass::Get_UI_Name() const
{
    return "Scroll North-West";
}

const char *ScrollNWCommandClass::Get_Category() const
{
    return Text_String(TXT_INTERFACE);
}

const char *ScrollNWCommandClass::Get_Description() const
{
    return "Scroll tactical map to the north-west.";
}

bool ScrollNWCommandClass::Process()
{
    int dist = 34;

    Map.Scroll_Map(FACING_NW, dist);

    return true;
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
                    DEBUG_INFO("  %04d\tName: %s\tCRC: 0x%08X\n", i, ptr->Name(), crc.CRC_Value()); \
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

    LOG_CRC(WeaponTypeClass, WeaponTypes);
    LOG_CRC(WarheadTypeClass, WarheadTypes);

    /**
     *  Color Schemes.
     */
    {
        DEBUG_INFO("ColorSchemes :\n");
        if (!ColorSchemes.Count()) {
            DEBUG_INFO("  EMPTY\n");
        } else {
            WWCRCEngine crc;
            for (unsigned i = 0; i < ColorSchemes.Count(); ++i) {
                ColorScheme *ptr = ColorSchemes[i];
                if (ptr != nullptr) {
                    DEBUG_INFO("  %04d\tName: %s\tfield_310: %d\n", i, ptr->Name, ptr->field_310);
                } else {
                    DEBUG_INFO("  %04d\tFAILED!\n", i);
                }
            }
        }
        DEBUG_INFO("\n");
    }
    
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


/**
 *  Removes the selected object(s) from the game world.
 * 
 *  @author: CCHyper
 */
const char *DeleteObjectCommandClass::Get_Name() const
{
    return "DeleteObject";
}

const char *DeleteObjectCommandClass::Get_UI_Name() const
{
    return "Delete Selected";
}

const char *DeleteObjectCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *DeleteObjectCommandClass::Get_Description() const
{
    return "Removes the selected object(s) from the game world.";
}

bool DeleteObjectCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    for (int i = 0; i < CurrentObjects.Count(); ++i) {
        ObjectClass *object = CurrentObjects[i];
        if (!object) {
            continue;
        }

        /**
         *  Buildings need to be "sold".
         */
        if (object->What_Am_I() == RTTI_BUILDING) {
            object->Sell_Back(1);
        } else {
            object->Unselect();
            object->Limbo();
            delete object;
        }
    }

    Map.Recalc();

    return true;
}


/**
 *  Spawn all buildable units and structures at mouse cursor location.
 * 
 *  @author: CCHyper
 */
const char *SpawnAllCommandClass::Get_Name() const
{
    return "SpawnAll";
}

const char *SpawnAllCommandClass::Get_UI_Name() const
{
    return "Spawn All";
}

const char *SpawnAllCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *SpawnAllCommandClass::Get_Description() const
{
    return "Spawn all buildable units and structures at mouse location.";
}

/**
 *  Attempt to unlimbo the object at the cell specified.
 */
bool SpawnAllCommandClass::Try_Unlimbo(TechnoClass *techno, Cell &cell)
{
    if (techno) {

        int map_cell_x = Map.MapCellX;
        int map_cell_y = Map.MapCellY;
        int map_cell_right = map_cell_x + Map.MapCellWidth;
        int map_cell_bottom = map_cell_y + Map.MapCellHeight;

        /**
         *  Generally try to prevent the objects from spawning off the right of the screen.
         */
        map_cell_right = std::min(map_cell_right, cell.X + 26);

        Cell attempt = cell;

        while (attempt.Y < map_cell_bottom) {

            Coordinate coord = Cell_Coord(attempt, true);
            if (techno->Unlimbo(coord)) {

                attempt.X++;
                if (attempt.X > map_cell_right - 2) {
                    attempt.X = cell.X; //map_cell_x + 2;
                    attempt.Y++;
                }

                cell = attempt;
                return true;
            }

            attempt.X++;
            if (attempt.X > map_cell_right - 2) {
                attempt.X = cell.X; //map_cell_x + 2;
                attempt.Y++;
            }
        }

        cell = attempt;
    }

    return false;
}

bool SpawnAllCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    /**
     *  Dont spawn anything lower than this row.
     */
    int map_cell_bottom = Map.MapCellY + Map.MapCellHeight;

    /**
     *  Default spawn location (top left of map).
     */
    CellStruct origin(Map.MapCellX + 2, Map.MapCellY + 2);

    /**
     *  If mouse position is valid, convert to world coordinates and update
     *  the spawn origin position to that of the mouse position.
     */
    if (WWMouse->Get_Mouse_XY().Is_Valid()) {
        origin = Get_Cell_Under_Mouse();
    }

    CellStruct attempt = origin;

    /**
     *  Attempt to spawn all ownable objects for the player house.
     */

    for (BuildingType index = BUILDING_FIRST; index < BuildingTypes.Count(); ++index) {
        BuildingTypeClass const & building_type = BuildingTypeClass::As_Reference(index);
        if (building_type.Get_Ownable() /*&& building_type.Level != -1*/) {
            BuildingClass * building = (BuildingClass *)building_type.Create_One_Of(PlayerPtr);
            if (building) {
                attempt = origin;
                while (attempt.Y < map_cell_bottom) {
                    if (Try_Unlimbo(building, attempt)) {
                        DEBUG_INFO("BuildingType %s spawned at %d,%d.\n", building_type.Name(),  attempt.X, attempt.Y);
                        break;
                    }
                }
            }
        }
    }

    for (UnitType index = UNIT_FIRST; index < UnitTypes.Count(); ++index) {
        UnitTypeClass const & unit_type = UnitTypeClass::As_Reference(index);
        if (unit_type.Get_Ownable() /*&& unit_type.Level != -1*/) {
            UnitClass * unit = (UnitClass *)unit_type.Create_One_Of(PlayerPtr);
            if (unit) {

                attempt = origin;

                while (attempt.Y < map_cell_bottom) {
                    if (Try_Unlimbo(unit, attempt)) {
                        DEBUG_INFO("UnitType %s spawned at %d,%d.\n", unit_type.Name(), attempt.X, attempt.Y);
                        break;
                    }
                }
            }
        }
    }

    for (InfantryType index = INFANTRY_FIRST; index < InfantryTypes.Count(); ++index) {
        InfantryTypeClass const & infantry_type = InfantryTypeClass::As_Reference(index);
        if (infantry_type.Get_Ownable() /*&& infantry_type.Level != -1*/) {
            InfantryClass * inf = (InfantryClass *)infantry_type.Create_One_Of(PlayerPtr);
            if (inf) {
                attempt = origin;
                while (attempt.Y < map_cell_bottom) {
                    if (Try_Unlimbo(inf, attempt)) {
                        DEBUG_INFO("InfantryType %s spawned at %d,%d.\n", infantry_type.Name(),  attempt.X, attempt.Y);
                        break;
                    }
                }
            }
        }
    }

    for (AircraftType index = AIRCRAFT_FIRST; index < AircraftTypes.Count(); ++index) {
        AircraftTypeClass const & aircraft_type = AircraftTypeClass::As_Reference(index);

        /**
         *  DROPPOD breaks the game!
         */
        //if (index == AIRCRAFT_DROPPOD) continue;
        if (aircraft_type == "DPOD") continue;

        if (aircraft_type.Get_Ownable() /*&& aircraft_type.Level != -1*/) {
            AircraftClass * air = (AircraftClass *)aircraft_type.Create_One_Of(PlayerPtr);
            if (air) {
                attempt = origin;
                while (attempt.Y < map_cell_bottom) {
                    if (Try_Unlimbo(air, attempt)) {
                        DEBUG_INFO("AircraftType %s spawned at %d,%d.\n", aircraft_type.Name(),  attempt.X, attempt.Y);
                        break;
                    }
                }
            }
        }
    }

    return true;
}


/**
 *  Apply damage to all selected objects.
 */
const char *DamageCommandClass::Get_Name() const
{
    return "Damage";
}

const char *DamageCommandClass::Get_UI_Name() const
{
    return "Damage";
}

const char *DamageCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *DamageCommandClass::Get_Description() const
{
    return "Apply damage to all selected objects.";
}

bool DamageCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    /**
     *  Iterate over all selected objects and deal 50 hit points. Use C4Damage as the backup.
     */
    for (int i = 0; i < CurrentObjects.Count(); ++i) {
        int damage = std::max(50, Rule->MinDamage);
        const WarheadTypeClass *warhead = WarheadTypeClass::As_Pointer("SA");
        if (!warhead) {
            warhead = Rule->C4Warhead;
        }
        CurrentObjects[i]->Take_Damage(damage, 0, warhead, nullptr);
    }

    Map.Recalc();

    return true;
}


/**
 *  Toggle the elite status of the selected objects.
 * 
 *  @author: CCHyper
 */
const char *ToggleEliteCommandClass::Get_Name() const
{
    return "ToggleElite";
}

const char *ToggleEliteCommandClass::Get_UI_Name() const
{
    return "Toggle Elite";
}

const char *ToggleEliteCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ToggleEliteCommandClass::Get_Description() const
{
    return "Toggle the elite status of the selected objects.";
}

bool ToggleEliteCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    for (int i = 0; i < CurrentObjects.Count(); ++i) {

        TechnoClass *techno = reinterpret_cast<TechnoClass *>(CurrentObjects[i]);
        if (!techno) {
            continue;
        }
        
        /**
         *  Upgrade to rookie.
         */
        if (techno->Veterancy.Is_Dumbass()) {
            techno->Veterancy.Set_Rookie(false);
            continue;
        }

        /**
         *  Upgrade to veteran.
         */
        if (techno->Veterancy.Is_Rookie()) {
            techno->Veterancy.Set_Veteran(true);
            continue;
        }
        
        /**
         *  Upgrade to elite.
         */
        if (techno->Veterancy.Is_Veteran()) {
            techno->Veterancy.Set_Elite(true);
            continue;
        }
        
        /**
         *  Degrade elite back to dumbass.
         */
        if (techno->Veterancy.Is_Elite()) {
            techno->Veterancy.Set_Rookie(true);
            continue;
        }
    }

    Map.Recalc();

    return true;
}


/**
 *  Unlock all available build options for the player house.
 * 
 *  @author: CCHyper
 */
const char *BuildCheatCommandClass::Get_Name() const
{
    return "BuildCheat";
}

const char *BuildCheatCommandClass::Get_UI_Name() const
{
    return "Build Cheat";
}

const char *BuildCheatCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *BuildCheatCommandClass::Get_Description() const
{
    return "Unlock all available build options for the player house.";
}

bool BuildCheatCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    /**
     *  Toggle the build cheat flag.
     */
    Vinifera_Developer_BuildCheat = !Vinifera_Developer_BuildCheat;

    /**
     *  Flag the player house to recalculate buildables.
     */
    PlayerPtr->IsRecalcNeeded = true;

    if (!ScenarioInit) {

        /**
         *  Update all factories.
         */
        for (int index = 0; index < Buildings.Count(); index++) {
            BuildingClass *building = Buildings[index];
            if (building) {
                if (building->Owning_House() == PlayerPtr) {
                    building->Update_Buildables();
                }
            }
        }
    }

    Map.Recalc();

    return true;
}


/**
 *  Toggles the visibility of the map shroud.
 * 
 *  @author: CCHyper
 */
const char *ToggleShroudCommandClass::Get_Name() const
{
    return "ToggleShroud";
}

const char *ToggleShroudCommandClass::Get_UI_Name() const
{
    return "Toggle Shroud";
}

const char *ToggleShroudCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ToggleShroudCommandClass::Get_Description() const
{
    return "Toggles the visibility of the map shroud.";
}

bool ToggleShroudCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    /**
     *  Toggle the unshroud flag.
     */
    Vinifera_Developer_Unshroud = !Vinifera_Developer_Unshroud;

    /**
     *  #NOTE:
     *  This is temporary code until the Unshroud flag is correctly
     *  hooked into DisplayClass and RadarClass!
     */
    if (Vinifera_Developer_Unshroud) {
        Map.Reveal_The_Map();
    } else {
        Map.Shroud_The_Map();
    }

    /**
     *  Force a redraw of the screen.
     */
    Map.Flag_To_Redraw(true);

    return true;
}


/**
 *  Heal the selected objects by 50 hit points.
 * 
 *  @author: CCHyper
 */
const char *HealCommandClass::Get_Name() const
{
    return "Heal";
}

const char *HealCommandClass::Get_UI_Name() const
{
    return "Heal";
}

const char *HealCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *HealCommandClass::Get_Description() const
{
    return "Heal the selected objects.";
}

bool HealCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    /**
     *  Iterate over all selected objects and heal by 50 hit points.
     */
    for (int i = 0; i < CurrentObjects.Count(); ++i) {
        int damage = -50;
        CurrentObjects[i]->Take_Damage(damage, 0, Rule->C4Warhead, nullptr);
    }

    Map.Recalc();

    return true;
}


/**
 *  Toggles if weapons do damage or not.
 * 
 *  @author: CCHyper
 */
const char *ToggleInertCommandClass::Get_Name() const
{
    return "ToggleInert";
}

const char *ToggleInertCommandClass::Get_UI_Name() const
{
    return "Toggle Inert";
}

const char *ToggleInertCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ToggleInertCommandClass::Get_Description() const
{
    return "Toggles if weapons are inert or not.";
}

bool ToggleInertCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    /**
     *  This flags controls whether weapons are inert. An inert weapon doesn't do
     *  any damage. Effectively, if this is true, then units will never die.
     */
    Scen->SpecialFlags.IsInert = !Scen->SpecialFlags.IsInert;

    return true;
}


/**
 *  Dumps all the current AI house base node info to the log output.
 * 
 *  @author: CCHyper
 */
const char *DumpAIBaseNodesCommandClass::Get_Name() const
{
    return "DumpAIBaseNodes";
}

const char *DumpAIBaseNodesCommandClass::Get_UI_Name() const
{
    return "Dump AI Base Nodes";
}

const char *DumpAIBaseNodesCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *DumpAIBaseNodesCommandClass::Get_Description() const
{
    return "Dumps all the current AI house base node info to the log output.";
}

bool DumpAIBaseNodesCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    DEBUG_INFO("About to dump AI base nodes...\n\n");

    for (int house_index = 0; house_index < Houses.Count(); ++house_index) {
        HouseClass *house = Houses[house_index];

        /**
         *  Make sure we only process non-player houses.
         */
        if (!house->Is_Player_Control() && !house->Is_Human_Control()) {

            DEBUG_INFO("\n");

            DEBUG_INFO("%02d \"%s\":\n", house_index, house->Class->Name());

            //DEBUG_INFO("  field_50: %d\n", house->Base.field_50);
            //DEBUG_INFO("  field_64: %d\n", house->Base.field_64);
            //DEBUG_INFO("  field_68: %d\n", house->Base.field_68);
            //DEBUG_INFO("  field_6C: %d\n", house->Base.field_6C);
            //DEBUG_INFO("  field_70: %d\n", house->Base.field_70);
            DEBUG_INFO("  PercentBuilt: %03d\n", house->Base.PercentBuilt);

            DEBUG_INFO("  Nodes.Count: %d\n", house->Base.Nodes.Count());

            /**
             *  Iterate all nodes for this house.
             */
            for (int node_index = 0; node_index < house->Base.Nodes.Count(); ++node_index) {
                BaseNodeClass &node = house->Base.Nodes[node_index];

                if (node.Type == BUILDING_NONE) {
                    continue;
                }

                const char *name = BuildingTypeClass::Name_From(node.Type);
                DEBUG_INFO("  Node %03d: \"%s\" at %d,%d\n", node_index, name, node.Where.X, node.Where.Y);
            }
        }
    }

    DEBUG_INFO("\nFinished!\n\n");

    return true;
}


/**
 *  Toggles the berzerk state of the selected infantry.
 * 
 *  @author: CCHyper
 */
const char *ToggleBerzerkCommandClass::Get_Name() const
{
    return "ToggleBerzerk";
}

const char *ToggleBerzerkCommandClass::Get_UI_Name() const
{
    return "Toggle Berzerk";
}

const char *ToggleBerzerkCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ToggleBerzerkCommandClass::Get_Description() const
{
    return "Toggles the berzerk state of the selected infantry.";
}

bool ToggleBerzerkCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    /**
     *  Iterate over all selected infantry and toggle their berzerk state.
     */
    for (int i = 0; i < CurrentObjects.Count(); ++i) {
        ObjectClass *object = CurrentObjects[i];
        if (object && object->Is_Infantry()) {
            InfantryClass *infantry = reinterpret_cast<InfantryClass *>(object);
            if (infantry) {
                infantry->IsBerzerk = !infantry->IsBerzerk;
            }
        }        
    }

    return true;
}


/**
 *  Increase the shroud darkness by one step (cell).
 * 
 *  @author: CCHyper
 */
const char *EncroachShadowCommandClass::Get_Name() const
{
    return "EncroachShadow";
}

const char *EncroachShadowCommandClass::Get_UI_Name() const
{
    return "Encroach Shadow";
}

const char *EncroachShadowCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *EncroachShadowCommandClass::Get_Description() const
{
    return "Increase the shroud darkness by one step (cell).";
}

bool EncroachShadowCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    Map.Encroach_Shadow();

    Map.Flag_To_Redraw(2);

    return true;
}


/**
 *  Increase the fog of war by one step (cell).
 * 
 *  @author: CCHyper
 */
const char *EncroachFogCommandClass::Get_Name() const
{
    return "EncroachFog";
}

const char *EncroachFogCommandClass::Get_UI_Name() const
{
    return "Encroach Fog";
}

const char *EncroachFogCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *EncroachFogCommandClass::Get_Description() const
{
    return "Increase the fog of war by one step (cell).";
}

bool EncroachFogCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    Map.Encroach_Fog();

    Map.Flag_To_Redraw(2);

    return true;
}


/**
 *  Toggles alliance with the selected objects house.
 * 
 *  @author: CCHyper
 */
const char *ToggleAllianceCommandClass::Get_Name() const
{
    return "ToggleAlly";
}

const char *ToggleAllianceCommandClass::Get_UI_Name() const
{
    return "Toggle Alliance";
}

const char *ToggleAllianceCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ToggleAllianceCommandClass::Get_Description() const
{
    return "Toggles alliance with the selected objects house.";
}

bool ToggleAllianceCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    /**
     *  Fetch the currently selected object and toggle the players alliance with its owner.
     */
    if (CurrentObjects.Count() == 1) {
        ObjectClass *object = CurrentObjects.Fetch_Head();
        if (object && object->Is_Techno()) {
            TechnoClass *techno = reinterpret_cast<TechnoClass *>(object);
            if (techno) {
                if (PlayerPtr != techno->House) {
                    if (PlayerPtr->Is_Ally(techno->House) || techno->House->Is_Ally(PlayerPtr)) {
                        PlayerPtr->Make_Enemy(techno->House);
                        techno->House->Make_Enemy(PlayerPtr);
                    } else {
                        PlayerPtr->Make_Ally(techno->House);
                        techno->House->Make_Ally(PlayerPtr);
                    }
                }
            }
        }        
    }

    return true;
}


/**
 *  Adds 2000 power units to the player.
 * 
 *  @author: CCHyper
 */
const char *AddPowerCommandClass::Get_Name() const
{
    return "AddPower";
}

const char *AddPowerCommandClass::Get_UI_Name() const
{
    return "Add Power";
}

const char *AddPowerCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *AddPowerCommandClass::Get_Description() const
{
    return "Adds 2000 power units to the player.";
}

bool AddPowerCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    /**
     *  Adjust the power value of the player house. 
     */
    PlayerPtr->Adjust_Power(2000);

    Map.Recalc();

    return true;
}


/**
 *  Places a random crate at the mouse location.
 * 
 *  @author: CCHyper
 */
const char *PlaceCrateCommandClass::Get_Name() const
{
    return "PlaceCrate";
}

const char *PlaceCrateCommandClass::Get_UI_Name() const
{
    return "Place Crate";
}

const char *PlaceCrateCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *PlaceCrateCommandClass::Get_Description() const
{
    return "Places a random crate at the mouse location.";
}

bool PlaceCrateCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    Cell mouse_cell = Get_Cell_Under_Mouse();

    CellClass *cell = &Map[mouse_cell];
    if (!cell) {
        return false;
    }

    /**
     *  Some safety checks;
     *   - Don't place in unshrouded cells.
     *   - Bridges are overlay, don't place there.
     *   - Make sure the cell does not already contain overlay.
     */
    if (!cell->IsVisible || cell->Is_Bridge_Here() || cell->Overlay != OVERLAY_NONE) {
        return false;
    }

    if (!Map.Place_Crate(mouse_cell)) {
        return false;
    }

    DEBUG_INFO("Crate placed at %d, %d\n", mouse_cell.X, mouse_cell.Y);

    return true;
}


/**
 *  Displays cell coordinates of the mouse cursor.
 * 
 *  @author: CCHyper
 */
const char *CursorPositionCommandClass::Get_Name() const
{
    return "CursorPosition";
}

const char *CursorPositionCommandClass::Get_UI_Name() const
{
    return "Cursor Position";
}

const char *CursorPositionCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *CursorPositionCommandClass::Get_Description() const
{
    return "Displays cell coordinates of the mouse cursor.";
}

bool CursorPositionCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    /**
     *  Toggle the show cursor position flag.
     */
    Vinifera_Developer_ShowCursorPosition = !Vinifera_Developer_ShowCursorPosition;

    return true;
}


/**
 *  Toggle frame step mode to step through the game frame-by-frame (for inspection).
 * 
 *  @author: CCHyper
 */
const char *ToggleFrameStepCommandClass::Get_Name() const
{
    return "ToggleFrameStep";
}

const char *ToggleFrameStepCommandClass::Get_UI_Name() const
{
    return "Toggle Frame Step";
}

const char *ToggleFrameStepCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ToggleFrameStepCommandClass::Get_Description() const
{
    return "Toggle frame step mode to step through the game frame-by-frame (for inspection).";
}

bool ToggleFrameStepCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    /**
     *  Toggle the frame step mode flag.
     */
    Vinifera_Developer_FrameStep = !Vinifera_Developer_FrameStep;
    Vinifera_Developer_FrameStepCount = 0;

    return true;
}


/**
 *  Frame Step Only: Step forward 1 frame.
 * 
 *  @author: CCHyper
 */
const char *Step1FrameCommandClass::Get_Name() const
{
    return "Step1Frame";
}

const char *Step1FrameCommandClass::Get_UI_Name() const
{
    return "Step Forward 1 Frame";
}

const char *Step1FrameCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *Step1FrameCommandClass::Get_Description() const
{
    return "Frame Step Only: Step forward 1 frame.";
}

bool Step1FrameCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    if (!Vinifera_Developer_FrameStep) {
        return false;
    }

    Vinifera_Developer_FrameStepCount = 1;

    return true;
}


/**
 *  Frame Step Only: Step forward 5 frames.
 * 
 *  @author: CCHyper
 */
const char *Step5FramesCommandClass::Get_Name() const
{
    return "Step5Frame";
}

const char *Step5FramesCommandClass::Get_UI_Name() const
{
    return "Step Forward 5 Frames";
}

const char *Step5FramesCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *Step5FramesCommandClass::Get_Description() const
{
    return "Frame Step Only: Step forward 5 frames.";
}

bool Step5FramesCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    if (!Vinifera_Developer_FrameStep) {
        return false;
    }

    Vinifera_Developer_FrameStepCount = 5;

    return true;
}


/**
 *  Frame Step Only: Step forward 10 frames.
 * 
 *  @author: CCHyper
 */
const char *Step10FramesCommandClass::Get_Name() const
{
    return "Step10Frames";
}

const char *Step10FramesCommandClass::Get_UI_Name() const
{
    return "Step Forward 10 Frames";
}

const char *Step10FramesCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *Step10FramesCommandClass::Get_Description() const
{
    return "Frame Step Only: Step forward 10 frames.";
}

bool Step10FramesCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    if (!Vinifera_Developer_FrameStep) {
        return false;
    }

    Vinifera_Developer_FrameStepCount = 10;

    return true;
}


/**
 *  Toggles AI control of the player house.
 * 
 *  @author: CCHyper
 */
const char *ToggleAIControlCommandClass::Get_Name() const
{
    return "ToggleAIControl";
}

const char *ToggleAIControlCommandClass::Get_UI_Name() const
{
    return "Toggle AI Control";
}

const char *ToggleAIControlCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ToggleAIControlCommandClass::Get_Description() const
{
    return "Toggles AI control of the player house.";
}

bool ToggleAIControlCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    HouseClass *player_house = PlayerPtr;

    if (player_house->IsPlayerControl) {

        /**
         *  AI takes control of the player house. We flag both the automated
         *  production and the alerted state, both of these will enter the
         *  automated building production and auto team systems.
         */

        player_house->IsHuman = false;
        player_house->IsPlayerControl = false;

        player_house->IsStarted = true;
        player_house->IsAlerted = true;

        /**
         *  Crank up the AI IQ to the max available.
         */
        player_house->IQ = Rule->MaxIQ;

        player_house->Difficulty = DIFF_HARD;

        DEV_DEBUG_INFO("Developer Mode: AI has taken control of player.\n");

    } else {

        /**
         *  Player retakes control from the AI. Disable any automation flags
         *  to allow the player have complete control of the house again.
         */
    
        player_house->IsHuman = true;
        player_house->IsPlayerControl = true;

        player_house->IsStarted = false;
        player_house->IsAlerted = false;

        /**
         *  Reset the IQ level.
         */
        player_house->IQ = 0;

        player_house->Difficulty = DIFF_NORMAL;

        DEV_DEBUG_INFO("Developer Mode: Player has resumed control.\n");
    }

    /**
     *  Toggle the global state flag.
     */
    Vinifera_Developer_AIControl = !Vinifera_Developer_AIControl;

    return true;
}


/**
 *  Cycle the camera between the starting waypoints on the map.
 * 
 *  @author: CCHyper
 */
const char *StartingWaypointsCommandClass::Get_Name() const
{
    return "StartingWaypoints";
}

const char *StartingWaypointsCommandClass::Get_UI_Name() const
{
    return "Cycle Starting Waypoints";
}

const char *StartingWaypointsCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *StartingWaypointsCommandClass::Get_Description() const
{
    return "Cycle the camera between the starting waypoints on the map.";
}

bool StartingWaypointsCommandClass::Process()
{
    if (Session.Type != GAME_SKIRMISH) {
        return false;
    }

    /**
     *  Fetch the next starting waypoint. We clamp to the first 8 waypoints
     *  as Tiberian Sun only supports these for starting locatons.
     */
    static int _current_index = 0;
    Coordinate wp_coord = Scen->Get_Waypoint_Coord(_current_index++ % 8);
    if (!wp_coord) {
        return false;
    }

    wp_coord.Z = Map.Get_Cell_Height(wp_coord);

    /**
     *  Center the tactical camera at this waypoint.
     */
    TacticalMap->Set_Tactical_Position(wp_coord);

    /**
     *  Clear any interface actions if they are active.
     */
    if (Map.PendingObject) {
        Map.Set_Cursor_Pos(Cell(0,0));
    }
    Map.Follow_This(nullptr);

    Map.Flag_To_Redraw(true);

    return true;
}


/**
 *  Places a random infantry at the mouse cell.
 * 
 *  @author: CCHyper
 */
const char *PlaceInfantryCommandClass::Get_Name() const
{
    return "PlaceInfantry";
}

const char *PlaceInfantryCommandClass::Get_UI_Name() const
{
    return "Place Infantry";
}

const char *PlaceInfantryCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *PlaceInfantryCommandClass::Get_Description() const
{
    return "Places a random infantry at the mouse cell.";
}

bool PlaceInfantryCommandClass::Process()
{
    if (Session.Type != GAME_SKIRMISH) {
        return false;
    }

    Coordinate mouse_coord = Get_Coord_Under_Mouse();
    mouse_coord.Z = Map.Get_Cell_Height(mouse_coord);

    const CellClass *cellptr = &Map[mouse_coord];
    if (!cellptr) {
        return false;
    }
    
    DynamicVectorClass<InfantryTypeClass *> available_infantry;

    int owner_id = 1 << PlayerPtr->Class->ID;

    /**
     *  Build a list of infantry from the available starting units.
     */
    for (int i = 0; i < InfantryTypes.Count(); ++i) {
        InfantryTypeClass *infantrytype = InfantryTypes[i];
        if (infantrytype && infantrytype->IsAllowedToStartInMultiplayer) {
            if (infantrytype->TechLevel <= PlayerPtr->Control.TechLevel && (owner_id & infantrytype->Ownable) != 0) {
                available_infantry.Add(infantrytype);
            }
        }
    }

    InfantryTypeClass *infantrytype = available_infantry[Random_Pick(0, available_infantry.Count()-1)];

    /**
     *  Create an instance of the infantry.
     */
    InfantryClass *inf = reinterpret_cast<InfantryClass *>(infantrytype->Create_One_Of(PlayerPtr));
    if (inf->Unlimbo(mouse_coord)) {
        DEBUG_INFO("Placed infantry \"%s\" at %d,%d,%d\n", inf->Name(), inf->Coord.X, inf->Coord.Y, inf->Coord.Z);
        return true;
    } else {
        delete inf;
    }


    return false;
}


/**
 *  Places a random unit at the mouse cell.
 * 
 *  @author: CCHyper
 */
const char *PlaceUnitCommandClass::Get_Name() const
{
    return "PlaceUnit";
}

const char *PlaceUnitCommandClass::Get_UI_Name() const
{
    return "Place Unit";
}

const char *PlaceUnitCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *PlaceUnitCommandClass::Get_Description() const
{
    return "Places a random unit at the mouse cell.";
}

bool PlaceUnitCommandClass::Process()
{
    if (Session.Type != GAME_SKIRMISH) {
        return false;
    }

    Coordinate mouse_coord = Get_Coord_Under_Mouse();
    mouse_coord.Z = Map.Get_Cell_Height(mouse_coord);

    const CellClass *cellptr = &Map[mouse_coord];
    if (!cellptr) {
        return false;
    }
    
    DynamicVectorClass<UnitTypeClass *> available_units;

    int owner_id = 1 << PlayerPtr->Class->ID;

    /**
     *  Build a list of units from the available starting units.
     */
    for (int i = 0; i < UnitTypes.Count(); ++i) {
        UnitTypeClass *unittype = UnitTypes[i];
        if (unittype && unittype->IsAllowedToStartInMultiplayer) {
            if (Rule->BaseUnit->Fetch_ID() != unittype->Fetch_ID()) {
                if (unittype->TechLevel <= PlayerPtr->Control.TechLevel && (owner_id & unittype->Ownable) != 0) {
                    available_units.Add(unittype);
                }
            }
        }
    }

    UnitTypeClass *unittype = available_units[Random_Pick(0, available_units.Count()-1)];

    /**
     *  Create an instance of the unit.
     */
    UnitClass *unit = reinterpret_cast<UnitClass *>(unittype->Create_One_Of(PlayerPtr));
    if (unit->Unlimbo(mouse_coord)) {
        DEBUG_INFO("Placed unit \"%s\" at %d,%d,%d\n", unit->Name(), unit->Coord.X, unit->Coord.Y, unit->Coord.Z);
        return true;
    } else {
        delete unit;
    }

    return false;
}


/**
 *  Places tiberium at the mouse cell.
 * 
 *  @author: CCHyper
 */
const char *PlaceTiberiumCommandClass::Get_Name() const
{
    return "PlaceTiberium";
}

const char *PlaceTiberiumCommandClass::Get_UI_Name() const
{
    return "Place Tiberium";
}

const char *PlaceTiberiumCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *PlaceTiberiumCommandClass::Get_Description() const
{
    return "Places tiberium at the mouse cell.";
}

bool PlaceTiberiumCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    Coordinate mouse_coord = Get_Coord_Under_Mouse();
    mouse_coord.Z = Map.Get_Cell_Height(mouse_coord);

    CellClass *cellptr = &Map[mouse_coord];
    if (!cellptr) {
        return false;
    }

    if (cellptr->Place_Tiberium(TIBERIUM_FIRST, 1)) {
        DEBUG_INFO("Placed tiberium \"%s\" at %d,%d,%d\n", Tiberiums[TIBERIUM_FIRST]->IniName, mouse_coord.X, mouse_coord.Y, mouse_coord.Z);
        return true;
    }

    return false;
}


/**
 *  Reduce tiberium at the mouse cell.
 * 
 *  @author: CCHyper
 */
const char *ReduceTiberiumCommandClass::Get_Name() const
{
    return "ReduceTiberium";
}

const char *ReduceTiberiumCommandClass::Get_UI_Name() const
{
    return "Reduce Tiberium";
}

const char *ReduceTiberiumCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *ReduceTiberiumCommandClass::Get_Description() const
{
    return "Reduces tiberium at the mouse cell.";
}

bool ReduceTiberiumCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    Coordinate mouse_coord = Get_Coord_Under_Mouse();
    mouse_coord.Z = Map.Get_Cell_Height(mouse_coord);

    CellClass *cellptr = &Map[mouse_coord];
    if (!cellptr) {
        return false;
    }

    if (cellptr->Reduce_Tiberium(1)) {
        DEBUG_INFO("Reduced tiberium \"%s\" at %d,%d,%d\n", Tiberiums[TIBERIUM_FIRST]->IniName, mouse_coord.X, mouse_coord.Y, mouse_coord.Z);
        return true;
    }

    return false;
}


/**
 *  Places fully grown tiberium at the mouse cell.
 * 
 *  @author: CCHyper
 */
const char *PlaceFullTiberiumCommandClass::Get_Name() const
{
    return "PlaceFullTiberium";
}

const char *PlaceFullTiberiumCommandClass::Get_UI_Name() const
{
    return "Place Fully Grown Tiberium";
}

const char *PlaceFullTiberiumCommandClass::Get_Category() const
{
    return CATEGORY_DEVELOPER;
}

const char *PlaceFullTiberiumCommandClass::Get_Description() const
{
    return "Places fully grown tiberium at the mouse cell.";
}

bool PlaceFullTiberiumCommandClass::Process()
{
    if (!Session.Singleplayer_Game()) {
        return false;
    }

    Coordinate mouse_coord = Get_Coord_Under_Mouse();
    mouse_coord.Z = Map.Get_Cell_Height(mouse_coord);

    CellClass *cellptr = &Map[mouse_coord];
    if (!cellptr) {
        return false;
    }

    if (cellptr->Place_Tiberium(TIBERIUM_FIRST, 11)) {
        DEBUG_INFO("Placed fully grown tiberium \"%s\" at %d,%d,%d\n", Tiberiums[TIBERIUM_FIRST]->IniName, mouse_coord.X, mouse_coord.Y, mouse_coord.Z);
        return true;
    }

    return false;
}
