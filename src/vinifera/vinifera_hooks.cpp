/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains various hooks that do not fit elsewhere.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "vinifera_hooks.h"

#include "armortype.h"
#include "asserthandler.h"
#include "beacon.h"
#include "blowfish.h"
#include "blowpipe.h"
#include "blowstraw.h"
#include "debughandler.h"
#include "detach_listener.h"
#include "dsurface.h"
#include "ebolt.h"
#include "extension.h"
#include "hooker.h"
#include "iomap.h"
#include "kamikazetracker.h"
#include "language.h"
#include "layer.h"
#include "loadoptions.h"
#include "prerequisitegroup.h"
#include "rockettype.h"
#include "spawner.h"
#include "spawnmanager.h"
#include "syringe.h"
#include "theme.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "tracker.h"
#include "scenarioext.h"
#include "sessionext.h"
#include "vinifera_functions.h"
#include "vinifera_globals.h"
#include "vinifera_saveload.h"
#include "vinifera_util.h"


/**
 *  Intercepts calls to the vanilla Detach_This_From_All so we can also notify
 *  Vinifera's type-indexed detach registry. Only listeners that opted in for
 *  the target's concrete type (and AbstractClass-wide listeners) are visited.
 */
static void _Detach_This_From_All_Intercept(AbstractClass * target, bool all)
{
    Vinifera::Detach::Notify_Abstract(target, all);
    Detach_This_From_All(target, all);
}


/**
 *  This function is for intercepting the calls to Free_Heaps to also process
 *  the extension interface.
 * 
 *  @author: CCHyper
 */
static void _Free_Heaps_Intercept()
{
    /**
     *  Cleanup global heaps/vectors.
     */
    ++ScenarioInit;

    /**
     *  Delete things that may depend on extensions/vanilla objects still existing.
     */
    while (RocketTypes.Count()) {
        delete RocketTypes[0];
    }
    Delete_Marked();

    //while (SpawnManagers.Count()) { // spawn managers are destroyed by their owners
    //    delete SpawnManagers[0];
    //}
    //Delete_Marked();

    EBoltClass::Clear_All();

    /**
     *  Free extensions as they may reference vanilla objects.
     */
    Extension::Free_Heaps();

    /**
     *  Now free vanilla objects.
     */
    Free_Heaps();

    /**
     *  Finally, clear armors. We do this at the very end because
     *  lots of things depend on verses being around.
     */
    while (ArmorTypes.Count()) {
        delete ArmorTypes[0];
    }
    Delete_Marked();

    /**
     *  Finally, clear armors. We do this at the very end because
     *  lots of things depend on verses being around.
     */
    while (PrerequisiteGroups.Count()) {
        delete PrerequisiteGroups[0];
    }
    Delete_Marked();

    BeaconManager.Reset();

    --ScenarioInit;
}


#if 0
/**
 *  This function is for intercepting the call to Clear_Scenarion in Load_All
 *  to flag that we are performing a load operation, which stops the game from
 *  creating extensions while the Windows API calls the class factories to create
 *  the instances.
 *
 *  @author: tomsons26
 */
static void _On_Load_Clear_Scenario_Intercept()
{
    Clear_Scenario();

    /**
     *  Now the scenario data has been cleaned up, we can now tell the extension
     *  hooks that we will be creating the extension classes via the class factories.
     */
    Vinifera_PerformingLoad = true;
}
#endif


/**
 *  Draws the version text on the main menu.
 * 
 *  @author: CCHyper
 */
void _ShowVersionText(Surface* surface)
{
    Vinifera_Draw_Version_Text(surface);
}


/**
 *  Draws the version text over the loading screen background.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005ADFBE, _ProgressClass_Load_Screen_Version_Text_Patch, 6)
{
    Vinifera_Draw_Version_Text(HiddenSurface);

    return 0;
}


/**
 *  Draws the version text over the loading screen background.
 * 
 *  @note: This has to be after the New menu initialisation, otherwise the menu
 *         title page will draw over the text.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004E084D, _Init_Game_Loading_Screen_Version_Text_Patch, 0)
{
    /**
     *  Flag as pre-init, as we need to draw this differently.
     */
    Vinifera_Draw_Version_Text(VisibleSurface, true);

    /**
     *  Stolen bytes/code.
     */
original_code:
    Call_Back();

    return 0x004E0852;
}


/**
 *  Draws the version text over the menu background.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004E3B7A, _Load_Title_Page_Version_Text_Patch, 1)
{
    Vinifera_Draw_Version_Text(HiddenSurface, true);

    return 0;
}


/**
 *  Patch in the main Vinifera startup function.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005FF81C, _WinMain_Vinifera_Startup, 0)
{
    if (Vinifera_Startup()) {
        return 0x005FFC41;
    }

    /**
     *  Something went wrong!
     */
    DEBUG_ERROR("Failed to initialise Vinifera systems!\n");

    R->ESI(EXIT_FAILURE);
    return 0x00601A6B;
}


/**
 *  Patch in the Vinifera com object register function.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00600F6E, _WinMain_Register_Com_Objects, 0)
{
    Vinifera_Register_Com_Objects();

    return 0x00600FA3;
}


/**
 *  Patch in the main Vinifera shutdown function.
 * 
 *  @author: CCHyper
 */
//DEFINE_HOOK(0x00602474, _Game_Shutdown_Vinifera_Shutdown, 3) // TS-Patches places a call here
DEFINE_HOOK(0x0060246E, _Game_Shutdown_Vinifera_Shutdown, 6)
{
    if (!Vinifera_Shutdown()) {

        /**
         *  Something went wrong!
         */

        DEBUG_ERROR("Failed to shutdown Vinifera systems!\n");
    }

    return 0;
}


/**
 *  Patch in the Vinifera init game function.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00462927, _Main_Game_Vinifera_Init_Game, 0)
{
    GET(int, argc, ECX);
    GET(char **, argv, EDX);

    int retval = Vinifera_Pre_Init_Game(argc, argv);
    if (retval) {
        if (retval < 0) {
            goto show_error;
        }
        goto failure;
    }
    DEV_DEBUG_INFO("Vinifera_Pre_Init_Game returned OK.\n");

    retval = Init_Game(argc, argv);
    if (retval) {
        if (retval < 0) {
            goto show_error;
        }
        goto failure;
    }
    DEV_DEBUG_INFO("Init_Game returned OK.\n");

    retval = Vinifera_Post_Init_Game(argc, argv);
    if (retval) {
        if (retval < 0) {
            goto show_error;
        }
        goto failure;
    }
    DEV_DEBUG_INFO("Vinifera_Post_Init_Game returned OK.\n");

success:
    return 0x00462990;

failure:
    return 0x00462932;

show_error:
    return 0x00462938;
}


/**
 *  #issue-96
 * 
 *  Remove the requirement for BLOWFISH.DLL (Blowfish encryption) and now
 *  handle the encryption/decryption internally.
 * 
 *  @author: CCHyper
 */

class FakeBlowfishClass
{
    public:
        BlowfishEngine *Hook_Ctor() { return new (reinterpret_cast<BlowfishEngine *>(this)) BlowfishEngine; }
        void Hook_Dtor() { reinterpret_cast<BlowfishEngine *>(this)->BlowfishEngine::~BlowfishEngine(); }
};

static void _Remove_External_Blowfish_Dependency_Patch()
{
    /**
     *  The following two patches remove dependency on BLOWFISH.DLL being registered at startup.
     */
    Patch_Jump(0x00600F6E, 0x00600FA3); // This forces the game init process to skip BLOWFISH.DLL loading errors.
    Patch_Jump(0x005FFE46, 0x005FFF2B); // This skips code registering BLOWFISH.DLL.

    /**
     *  Hook in the implementations of BlowStraw, BlowPipe and BlowfishEngine.
     */
    Hook_Virtual(0x00424230, BlowStraw::Get);
    Hook_Virtual(0x00424320, BlowStraw::Key);
    Hook_Virtual(0x00424080, BlowPipe::Flush);
    Hook_Virtual(0x004240C0, BlowPipe::Put);
    Hook_Virtual(0x004241F0, BlowPipe::Key);

    Hook_Function(0x00423F70, &FakeBlowfishClass::Hook_Ctor);
    Hook_Function(0x00423FE0, &FakeBlowfishClass::Hook_Dtor);
    Hook_Function(0x00423FF0, &BlowfishEngine::Submit_Key);
    Hook_Function(0x00424020, &BlowfishEngine::Encrypt);
    Hook_Function(0x00424050, &BlowfishEngine::Decrypt);
}


/**
 *  Clear any game session and global variables before next game.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004E1F24, _Select_Game_Clear_Globals_Patch, 0)
{
    /**
     *  Clear any developer mode globals.
     */
    Vinifera_Developer_AIInstantBuild = false;
    Vinifera_Developer_InstantBuild = false;
    Vinifera_Developer_InstantSuperRecharge = false;
    Vinifera_Developer_AIInstantSuperRecharge = false;
    Vinifera_Developer_BuildCheat = false;
    Vinifera_Developer_Unshroud = false;
    Vinifera_Developer_FrameStep = false;
    Vinifera_Developer_FrameStepCount = 0;
    Vinifera_Developer_AIControl = false;

    /**
     *  Reset any globals.
     */
    Vinifera_ShowSuperWeaponTimers = true;
    if (SessionExtension) {
        SessionExtension->Init_Clear();
    }

    /**
     *  Stolen bytes/code.
     */
    Map.Set_Default_Mouse(MOUSE_NORMAL);

    return 0x004E1F30;
}


/**
 *  #issue-269
 * 
 *  Adds a "Load Game" button to the dialog shown on mission lose.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005DCDFD, _Do_Lose_Create_Lose_WWMessageBox, 0)
{
    while (true) {

        /**
         *  Show the message box.
         */
        int ret = Vinifera_Do_WWMessageBox(Text_String(TXT_TO_REPLAY), Text_String(TXT_YES), Text_String(TXT_NO), "Load Game");

        switch (ret) {
        default:
        case 0: // User pressed "Yes"
            return 0x005DCE1A;

        case 1: // User pressed "No"
            return 0x005DCE56;

        case 2: // User pressed "Load Game"
        {
            /**
             *  If no save games are available, notify the user and return back
             *  and reissue the main dialog.
             */
            if (!LoadOptionsClass().Read_Save_Files()) {
                Vinifera_Do_WWMessageBox("No saved games available.", Text_String(TXT_OK));
                continue;
            }

            /**
             *  Show the load game dialog.
             */
            ret = LoadOptionsClass().Load_Dialog();
            if (ret) {
                Theme.Stop();
                return 0x005DCE48;
            }

            /**
             *  Reissue the dialog if the user pressed cancel on the load dialog.
             */
            continue;
        }
        }
    }
}


/**
 *  Produces a random serial number for this client.
 * 
 *  #NOTE:
 *  The result number string will be invalid and will not pass WWOnline/XWIS
 *  checks, this is for local network use only.
 * 
 *  @author: CCHyper
 */
static void Decrypt_Serial(char *buffer)
{
    static bool _done = false;
    static char _buf[] = { "0000000000000000000000" };
    static const char _alphanum[] = { "0123456789" };

    /**
     *  Generate a one-time random number string.
     */
    if (!_done) {
        std::srand(timeGetTime());
        for (int i = 0; i < std::size(_buf); ++i) {
            _buf[i] = _alphanum[std::rand() % (std::size(_alphanum)-1)];
        }
        _done = true;
    }

    std::strncpy(buffer, _buf, sizeof(_buf));
}


/**
 *  Export function that returns the supported save file version.
 *
 *  @author: CCHyper
 */
__declspec(dllexport) uint32_t __cdecl Vinifera_Save_File_Version()
{
    return Extension::Get_Save_Version_Number();
}


/**
 *  A replacement buffer for RLE_Blit.
 */
char RLEBlitBuffer[4096];


/**
 *  Temporary LayerClass extension to catch null pointers being added to layers.
 */
static class LayerClassExt : public LayerClass
{
public:
    bool _Submit(const ObjectClass* object, bool sort);
};


bool LayerClassExt::_Submit(const ObjectClass* object, bool sort)
{
    ASSERT(object != nullptr);

    if (object == nullptr)
    {
        // force a crash
        int* p = nullptr;
        *p = 50;
    }

    /*
    **  Add the object to the layer. Either at the end (if "sort" is false) or at the
    **  appropriately sorted position.
    */
    if (sort) {
        return Sorted_Add(object) != false;
    }
    return Add((ObjectClass*)object) != false;
}



void Vinifera_Hooks()
{
    /**
     *  Remove the requirement for BLOWFISH.DLL (Blowfish encryption).
     */
    _Remove_External_Blowfish_Dependency_Patch();

    /**
     *  Draw the build version info on the bottom on the screen.
     */
    Patch_Jump(0x004E53C0, &_ShowVersionText);

    /**
     *  Add in Vinifera startup/shutdown hooks.
     */
    Patch_Call(0x00601078, &Vinifera_Parse_Command_Line);

#ifndef NDEBUG
    /**
     *  These patches remove the Digest requirement for LANGRULE.INI, allowing
     *  this file to be used by developers to quickly test features.
     */
    Patch_Byte(0x005C656E+1, 0); // CCINIClass::Load argument from "true" to "false".
    Patch_Byte(0x004E1436, 0x53); // CCINIClass::Load argument from "true" to "false".
#endif

#ifndef NDEBUG
    /**
     *  This patch allows 1 player LAN games for testing various network features.
     */
    Patch_Jump(0x00577029, 0x00577071);

    /**
     *  Allow up to 7 AI players in LAN games.
     */
    Patch_Byte(0x0057C97E+3, 0x07);
#endif

    /**
     *  Replace a small buffer in RLE_Blit which when overrun caused artifacts.
     */
    Patch_Dword(0x00423D4D + 4, reinterpret_cast<uint32_t>(&RLEBlitBuffer[0]));

    /**
     *  Patch in the new save and load system functions.
     */
    Patch_Jump(0x005D4FE0, &Vinifera_Save_Game);
    Patch_Jump(0x005D6910, &Vinifera_Load_Game);

    /**
     *  Hooks related to saving/loading games.
     */
    SaveGame_Hooks();

    /**
     *  Set the save game version.
     */
    ViniferaGameVersion = Extension::Get_Save_Version_Number();
    DEBUG_INFO("Save game version number: 0x{:X}\n", ViniferaGameVersion);

    /**
     *  This patch randomises the serial number for this client.
     */
    Patch_Jump(0x00576410, &Decrypt_Serial);

    /**
     *  These two patches changes the last character of the Autorun and Game
     *  application mutex GUID's So Vinifera can be run alongside another instance
     *  of Tiberian Sun (and even Red Alert 2 or Yuri's Revenge).
     * 
     *  "b350c6d2-2f36-11d3-a72c-0090272fa661" -> "b350c6d2-2f36-11d3-a72c-0090272fa66n"
     *  "29e3bb2a-2f36-11d3-a72c-0090272fa661" -> "29e3bb2a-2f36-11d3-a72c-0090272fa66n"
     */
    std::srand(timeGetTime());
    unsigned char num = (std::rand() % 10)+48;
    Patch_Byte(0x0070EEAB, num);
    Patch_Byte(0x0070EF0F, num);

    /**
     *  Various patches to intercept the games object tracking and heap processing.
     */
    Patch_Call(0x0053DF7A, &_Free_Heaps_Intercept); // MapSeedClass::Init_Random
    Patch_Call(0x005DC590, &_Free_Heaps_Intercept); // Clear_Scenario
    Patch_Call(0x00601BA2, &_Free_Heaps_Intercept); // Game_Shutdown

    Patch_Call(0x0040DBB3, &_Detach_This_From_All_Intercept); // AircraftClass::~AircraftClass
    Patch_Call(0x0040F123, &_Detach_This_From_All_Intercept); // AircraftClass_Fall_To_Death
    Patch_Call(0x0040FCD3, &_Detach_This_From_All_Intercept); // AircraftTypeClass::~AircraftTypeClass
    Patch_Call(0x00410223, &_Detach_This_From_All_Intercept); // AircraftTypeClass::~AircraftTypeClass
    Patch_Call(0x004142C6, &_Detach_This_From_All_Intercept); // AnimClass::~AnimClass
    Patch_Call(0x00426662, &_Detach_This_From_All_Intercept); // BuildingClass::~BuildingClass
    Patch_Call(0x0043F94D, &_Detach_This_From_All_Intercept); // BuildingTypeClass::~BuildingTypeClass
    Patch_Call(0x0044407D, &_Detach_This_From_All_Intercept); // BuildingTypeClass::~BuildingTypeClass
    Patch_Call(0x004445F3, &_Detach_This_From_All_Intercept); // BulletClass::~BulletClass
    Patch_Call(0x004474D3, &_Detach_This_From_All_Intercept); // BulletClass::~BulletClass
    Patch_Call(0x00447DC3, &_Detach_This_From_All_Intercept); // BulletTypeClass::~BulletTypeClass
    Patch_Call(0x00448723, &_Detach_This_From_All_Intercept); // BulletTypeClass::~BulletTypeClass
    Patch_Call(0x00448AE3, &_Detach_This_From_All_Intercept); // CampaignClass::~CampaignClass
    Patch_Call(0x00448EF3, &_Detach_This_From_All_Intercept); // CampaignClass::~CampaignClass
    Patch_Call(0x00456A26, &_Detach_This_From_All_Intercept); // CellClass::Wall_Update
    Patch_Call(0x00456A58, &_Detach_This_From_All_Intercept); // CellClass::Wall_Update
    Patch_Call(0x00456A7F, &_Detach_This_From_All_Intercept); // CellClass::Wall_Update
    Patch_Call(0x00456AAB, &_Detach_This_From_All_Intercept); // CellClass::Wall_Update
    Patch_Call(0x00456AD2, &_Detach_This_From_All_Intercept); // CellClass::Wall_Update
    Patch_Call(0x004571F9, &_Detach_This_From_All_Intercept); // CellClass::Reduce_Wall
    Patch_Call(0x004927D3, &_Detach_This_From_All_Intercept); // EMPulseClass::~EMPulseClass
    Patch_Call(0x004931E3, &_Detach_This_From_All_Intercept); // EMPulseClass::~EMPulseClass
    Patch_Call(0x00496DB3, &_Detach_This_From_All_Intercept); // FactoryClass::~FactoryClass
    Patch_Call(0x00497AA3, &_Detach_This_From_All_Intercept); // FactoryClass::~FactoryClass
    Patch_Call(0x004BB6DB, &_Detach_This_From_All_Intercept); // HouseClass::~HouseClass
    Patch_Call(0x004CDE93, &_Detach_This_From_All_Intercept); // HouseTypeClass::~HouseTypeClass
    Patch_Call(0x004CE603, &_Detach_This_From_All_Intercept); // HouseTypeClass::~HouseTypeClass
    Patch_Call(0x004D22DC, &_Detach_This_From_All_Intercept); // InfantryClass::~InfantryClass
    Patch_Call(0x004DA3B4, &_Detach_This_From_All_Intercept); // InfantryTypeClass::~InfantryTypeClass
    Patch_Call(0x004DB133, &_Detach_This_From_All_Intercept); // InfantryTypeClass::~InfantryTypeClass
    Patch_Call(0x004F2173, &_Detach_This_From_All_Intercept); // IsometricTileClass::~IsometricTileClass
    Patch_Call(0x004F23E3, &_Detach_This_From_All_Intercept); // IsometricTileClass::~IsometricTileClass
    Patch_Call(0x004F3344, &_Detach_This_From_All_Intercept); // IsometricTileTypeClass::~IsometricTileTypeClass
    Patch_Call(0x005015E3, &_Detach_This_From_All_Intercept); // LightSourceClass::~LightSourceClass
    Patch_Call(0x00501DA3, &_Detach_This_From_All_Intercept); // LightSourceClass::~LightSourceClass
    Patch_Call(0x00585F9E, &_Detach_This_From_All_Intercept); // ObjectClass::Detach_All
    Patch_Call(0x00586DB5, &_Detach_This_From_All_Intercept); // ObjectClass::entry_E4
    Patch_Call(0x0058B563, &_Detach_This_From_All_Intercept); // OverlayClass::~OverlayClass
    Patch_Call(0x0058CB13, &_Detach_This_From_All_Intercept); // OverlayClass::~OverlayClass
    Patch_Call(0x0058D196, &_Detach_This_From_All_Intercept); // OverlayTypeClass::~OverlayTypeClass
    Patch_Call(0x0058DC86, &_Detach_This_From_All_Intercept); // OverlayTypeClass::~OverlayTypeClass
    Patch_Call(0x005A32FA, &_Detach_This_From_All_Intercept); // ParticleClass::~ParticleClass
    Patch_Call(0x005A503A, &_Detach_This_From_All_Intercept); // ParticleClass::~ParticleClass
    Patch_Call(0x005A56D4, &_Detach_This_From_All_Intercept); // ParticleSystemClass::~ParticleSystemClass
    Patch_Call(0x005AE573, &_Detach_This_From_All_Intercept); // ParticleSystemTypeClass::~ParticleSystemTypeClass
    Patch_Call(0x005AEC63, &_Detach_This_From_All_Intercept); // ParticleSystemTypeClass::~ParticleSystemTypeClass
    Patch_Call(0x005AF153, &_Detach_This_From_All_Intercept); // ParticleTypeClass::~ParticleTypeClass
    Patch_Call(0x005AFC33, &_Detach_This_From_All_Intercept); // ParticleTypeClass::~ParticleTypeClass
    Patch_Call(0x005E78C3, &_Detach_This_From_All_Intercept); // ScriptClass::~ScriptClass
    Patch_Call(0x005E7B83, &_Detach_This_From_All_Intercept); // ScriptTypeClass::~ScriptTypeClass
    Patch_Call(0x005E81E3, &_Detach_This_From_All_Intercept); // ScriptClass::~ScriptClass
    Patch_Call(0x005E8293, &_Detach_This_From_All_Intercept); // ScriptTypeClass::~ScriptTypeClass
    Patch_Call(0x005F1AE3, &_Detach_This_From_All_Intercept); // SideClass::~SideClass
    Patch_Call(0x005F1D93, &_Detach_This_From_All_Intercept); // SideClass::~SideClass
    Patch_Call(0x005FAAD3, &_Detach_This_From_All_Intercept); // SmudgeClass::~SmudgeClass
    Patch_Call(0x005FAF03, &_Detach_This_From_All_Intercept); // SmudgeClass::~SmudgeClass
    Patch_Call(0x005FB313, &_Detach_This_From_All_Intercept); // SmudgeTypeClass::~SmudgeTypeClass
    Patch_Call(0x005FC023, &_Detach_This_From_All_Intercept); // SmudgeTypeClass::~SmudgeTypeClass
    Patch_Call(0x00618D03, &_Detach_This_From_All_Intercept); // TActionClass::~TActionClass
    Patch_Call(0x0061DAD3, &_Detach_This_From_All_Intercept); // TActionClass::~TActionClass
    Patch_Call(0x0061E4B6, &_Detach_This_From_All_Intercept); // TagClass::~TagClass
    Patch_Call(0x0061E73B, &_Detach_This_From_All_Intercept); // TagClass::~TagClass
    Patch_Call(0x0061E9AA, &_Detach_This_From_All_Intercept); // TagClass::Spring
    Patch_Call(0x0061F164, &_Detach_This_From_All_Intercept); // TagTypeClass::~TagTypeClass
    Patch_Call(0x00621503, &_Detach_This_From_All_Intercept); // TaskForceClass::~TaskForceClass
    Patch_Call(0x00621E43, &_Detach_This_From_All_Intercept); // TaskForceClass::~TaskForceClass
    Patch_Call(0x006224E3, &_Detach_This_From_All_Intercept); // TeamClass::~TeamClass
    Patch_Call(0x00627EF3, &_Detach_This_From_All_Intercept); // TeamTypeClass::~TeamTypeClass
    Patch_Call(0x00629293, &_Detach_This_From_All_Intercept); // TeamTypeClass::~TeamTypeClass
    Patch_Call(0x0063F188, &_Detach_This_From_All_Intercept); // TerrainClass::~TerrainClass
    Patch_Call(0x00640C38, &_Detach_This_From_All_Intercept); // TerrainClass::~TerrainClass
    Patch_Call(0x00641653, &_Detach_This_From_All_Intercept); // TerrainTypeClass::~TerrainTypeClass
    Patch_Call(0x00641D83, &_Detach_This_From_All_Intercept); // TerrainTypeClass::~TerrainTypeClass
    Patch_Call(0x00642223, &_Detach_This_From_All_Intercept); // TEventClass::~TEventClass
    Patch_Call(0x00642F23, &_Detach_This_From_All_Intercept); // TEventClass::~TEventClass
    Patch_Call(0x00644A45, &_Detach_This_From_All_Intercept); // TiberiumClass::~TiberiumClass
    Patch_Call(0x006491A3, &_Detach_This_From_All_Intercept); // TriggerClass::~TriggerClass
    Patch_Call(0x00649943, &_Detach_This_From_All_Intercept); // TriggerClass::~TriggerClass
    Patch_Call(0x00649E03, &_Detach_This_From_All_Intercept); // TriggerTypeClass::~TriggerTypeClass
    Patch_Call(0x0064AFD3, &_Detach_This_From_All_Intercept); // TubeClass::~TubeClass
    Patch_Call(0x0064B603, &_Detach_This_From_All_Intercept); // TubeClass::~TubeClass
    Patch_Call(0x0064D8A9, &_Detach_This_From_All_Intercept); // UnitClass::~UnitClass
    Patch_Call(0x0065BAD3, &_Detach_This_From_All_Intercept); // UnitTypeClass::~UnitTypeClass
    Patch_Call(0x0065C793, &_Detach_This_From_All_Intercept); // UnitTypeClass::~UnitTypeClass
    Patch_Call(0x0065DF23, &_Detach_This_From_All_Intercept); // VoxelAnimClass::~VoxelAnimClass
    Patch_Call(0x0065F5A3, &_Detach_This_From_All_Intercept); // VoxelAnimTypeClass::~VoxelAnimTypeClass
    Patch_Call(0x00660093, &_Detach_This_From_All_Intercept); // VoxelAnimTypeClass::~VoxelAnimTypeClass
    Patch_Call(0x00661227, &_Detach_This_From_All_Intercept); // VeinholeMonsterClass::~VeinholeMonsterClass
    Patch_Call(0x00661C00, &_Detach_This_From_All_Intercept); // VeinholeMonsterClass::Take_Damage
    Patch_Call(0x0066EF73, &_Detach_This_From_All_Intercept); // WarheadTypeClass::~WarheadTypeClass
    Patch_Call(0x0066FA93, &_Detach_This_From_All_Intercept); // WarheadTypeClass::~WarheadTypeClass
    Patch_Call(0x006702D4, &_Detach_This_From_All_Intercept); // WaveClass::~WaveClass
    Patch_Call(0x00672E73, &_Detach_This_From_All_Intercept); // WaveClass::~WaveClass
    Patch_Call(0x00673563, &_Detach_This_From_All_Intercept); // WaypointPathClass::~WaypointPathClass
    Patch_Call(0x00673AA3, &_Detach_This_From_All_Intercept); // WaypointPathClass::~WaypointPathClass
    Patch_Call(0x00680C54, &_Detach_This_From_All_Intercept); // WeaponTypeClass::~WeaponTypeClass
    Patch_Call(0x006818F4, &_Detach_This_From_All_Intercept); // WeaponTypeClass::~WeaponTypeClass

    /**
     *  Replace the vanilla Print_CRCs with Extension::Print_CRCs, which writes
     *  the unified Vinifera desync log instead of SYNC#.TXT. Patching the
     *  function itself ensures every caller (the game itself, as well as any
     *  external patch sets) produces a single log.
     */
    Patch_Jump(0x005B58F0, static_cast<void (*)(EventClass *)>(&Extension::Print_CRCs)); // Disambiguate the overload set for template deduction.

    //Patch_Call(0x005D6BEC, &_On_Load_Clear_Scenario_Intercept); // Load_All

    Patch_Jump(0x004FCD70, &LayerClassExt::_Submit);

    Patch_Jump(0x00600A54, 0x00600A91); // Skip registering vanilla JumpjetLocomotionClass in WinMain
}
