///**
//*  yrpp-spawner
//*
//*  Copyright(C) 2022-present CnCNet
//*
//*  This program is free software: you can redistribute it and/or modify
//*  it under the terms of the GNU General Public License as published by
//*  the Free Software Foundation, either version 3 of the License, or
//*  (at your option) any later version.
//*
//*  This program is distributed in the hope that it will be useful,
//*  but WITHOUT ANY WARRANTY; without even the implied warranty of
//*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//*  GNU General Public License for more details.
//*
//*  You should have received a copy of the GNU General Public License
//*  along with this program.If not, see <http://www.gnu.org/licenses/>.
//*/
//
//#include "Spawner.h"
//#include "NetHack.h"
//
//#include <HouseClass.h>
//#include <SessionClass.h>
//#include <Utilities/Debug.h>
//#include <Utilities/Macro.h>
//
//DEFINE_HOOK(0x6BD7C5, WinMain_SpawnerInit, 0x6)
//{
//    if (Spawner::Enabled)
//    {
//        Spawner::Init();
//
//
//        Patch::Apply_CALL(0x48CDD3, Spawner::StartGame); // Main_Game
//        Patch::Apply_CALL(0x48CFAA, Spawner::StartGame); // Main_Game
//
//        { // HousesStuff
//            Patch::Apply_CALL(0x68745E, Spawner::AssignHouses); // Read_Scenario_INI
//            Patch::Apply_CALL(0x68ACFF, Spawner::AssignHouses); // ScenarioClass::Read_INI
//
//            Patch::Apply_LJMP(0x5D74A0, 0x5D7570); // MPGameModeClass_AllyTeams
//            Patch::Apply_LJMP(0x501721, 0x501736); // HouseClass_ComputerParanoid
//            Patch::Apply_LJMP(0x686A9E, 0x686AC6); // RemoveAIPlayers
//        }
//
//        { // NetHack
//            Patch::Apply_CALL(0x7B3D75, NetHack::SendTo);   // UDPInterfaceClass::Message_Handler
//            Patch::Apply_CALL(0x7B3EEC, NetHack::RecvFrom); // UDPInterfaceClass::Message_Handler
//        }
//
//        { // Skip Intro, EA_WWLOGO and LoadScreen
//            Patch::Apply_LJMP(0x52CB50, 0x52CB6E); // InitIntro_Skip
//            Patch::Apply_LJMP(0x52C5E0, 0x52C5F8); // InitGame_SkipLogoAndLoadScreen
//        }
//
//        { // Cooperative
//            Patch::Apply_LJMP(0x553321, 0x5533C5); // LoadProgressMgr_Draw_CooperativeDescription
//            Patch::Apply_LJMP(0x55D0DF, 0x55D0E8); // AuxLoop_Cooperative_EndgameCrashFix
//        }
//
//        // Set ConnTimeout
//        Patch::Apply_TYPED<int>(0x6843C7, { Spawner::GetConfig()->ConnTimeout }); //  Scenario_Load_Wait
//
//        // Show GameMode in DiplomacyDialog in Skirmish
//        Patch::Apply_LJMP(0x658117, 0x658126); // RadarClass_DiplomacyDialog
//
//        // Leaves bottom bar closed for losing players during last game frames
//        Patch::Apply_LJMP(0x6D1639, 0x6D1640); // TabClass_6D1610
//    }
//
//    return 0;
//}
//
//// Display UIGameMode if is set
//// Otherwise use mode name from MPModesMD.ini
//DEFINE_HOOK(0x65812E, RadarClass__DiplomacyDialog_UIGameMode, 0x6)
//{
//    enum { Show = 0x65813E, DontShow = 0x65814D };
//
//    if (Spawner::Enabled && Spawner::GetConfig()->UIGameMode[0])
//    {
//        R->EBX(R->EAX());
//        R->EAX(Spawner::GetConfig()->UIGameMode);
//        return Show;
//    }
//
//    if (!SessionClass::Instance->MPGameMode)
//        return DontShow;
//
//    return 0;
//}
//
//// Clear UIGameMode on game load
//DEFINE_HOOK(0x689669, ScenarioClass_Load_Suffix, 0x6)
//{
//    if (Spawner::Enabled)
//        Spawner::GetConfig()->UIGameMode[0] = 0;
//
//    return 0;
//}
//
//#pragma region MPlayerDefeated
//namespace MPlayerDefeated
//{
//    HouseClass* pThis = nullptr;
//}
//
//DEFINE_HOOK(0x4FC0B6, HouseClass__MPlayerDefeated_SaveArgument, 0x5)
//{
//    MPlayerDefeated::pThis = (Spawner::Enabled && !SessionClass::IsCampaign())
//        ? R->ECX<HouseClass*>()
//        : nullptr;
//
//    return 0;
//}
//
//// Skip match-end logic if MPlayerDefeated called for observer
//DEFINE_HOOK_AGAIN(0x4FC332, HouseClass__MPlayerDefeated_SkipObserver, 0x5)
//DEFINE_HOOK(0x4FC262, HouseClass__MPlayerDefeated_SkipObserver, 0x6)
//{
//    enum { ProcEpilogue = 0x4FC6BC };
//
//    if (!MPlayerDefeated::pThis)
//        return 0;
//
//    return MPlayerDefeated::pThis->IsObserver()
//        ? ProcEpilogue
//        : 0;
//}
//
//DEFINE_HOOK(0x4FC551, HouseClass__MPlayerDefeated_NoEnemies, 0x5)
//{
//    enum { ProcEpilogue = 0x4FC6BC };
//
//    if (!MPlayerDefeated::pThis)
//        return 0;
//
//    for (const auto pHouse : *HouseClass::Array)
//    {
//        if (pHouse->Defeated || pHouse == MPlayerDefeated::pThis || pHouse->Type->MultiplayPassive)
//            continue;
//
//        if ((pHouse->IsHumanPlayer || Spawner::GetConfig()->ContinueWithoutHumans) && pHouse->IsMutualAllie(MPlayerDefeated::pThis))
//        {
//            Debug::Log("[Spawner] MPlayer_Defeated() - Defeated player has a living ally");
//            if (Spawner::GetConfig()->DefeatedBecomesObserver)
//                MPlayerDefeated::pThis->MakeObserver();
//
//            return ProcEpilogue;
//        }
//    }
//
//    return 0;
//}
//
//DEFINE_HOOK(0x4FC57C, HouseClass__MPlayerDefeated_CheckAliveAndHumans, 0x7)
//{
//    enum { ProcEpilogue = 0x4FC6BC, FinishMatch = 0x4FC591 };
//
//    if (!MPlayerDefeated::pThis)
//        return 0;
//
//    GET_STACK(int, numHumans, STACK_OFFSET(0xC0, -0xA8));
//    GET_STACK(int, numAlive, STACK_OFFSET(0xC0, -0xAC));
//
//    bool continueWithoutHumans = Spawner::GetConfig()->ContinueWithoutHumans ||
//        (SessionClass::IsSkirmish() && HouseClass::CurrentPlayer->IsInitiallyObserver());
//
//    if (numAlive > 1 && (numHumans != 0 || continueWithoutHumans))
//    {
//        if (Spawner::GetConfig()->DefeatedBecomesObserver)
//            MPlayerDefeated::pThis->MakeObserver();
//
//        return ProcEpilogue;
//    }
//
//    return FinishMatch;
//}
//
//#pragma endregion MPlayerDefeated
