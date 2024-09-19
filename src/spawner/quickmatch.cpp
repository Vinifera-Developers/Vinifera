///**
//*  yrpp-spawner
//*
//*  Copyright(C) 2023-present CnCNet
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
//#include <Utilities/Macro.h>
//
//namespace QuickMatch
//{
//    const wchar_t* PlayerString = L"Player";
//}
//
//DEFINE_HOOK(0x643AA5, ProgressScreenClass_643720_HideName, 0x8)
//{
//    if ((Spawner::Enabled && Spawner::GetConfig()->QuickMatch) == false)
//        return 0;
//
//    REF_STACK(wchar_t*, pPlayerName, STACK_OFFSET(0x5C, 8));
//    pPlayerName = const_cast<wchar_t*>(QuickMatch::PlayerString);
//
//    return 0;
//}
//
//DEFINE_HOOK(0x65837A, RadarClass_658330_HideName, 0x6)
//{
//    if ((Spawner::Enabled && Spawner::GetConfig()->QuickMatch) == false)
//        return 0;
//
//    R->ECX(QuickMatch::PlayerString);
//    return 0x65837A + 0x6;
//}
//
//DEFINE_HOOK(0x64B156, ModeLessDialog_64AE50_HideName, 0x9)
//{
//    if ((Spawner::Enabled && Spawner::GetConfig()->QuickMatch) == false)
//        return 0;
//
//    R->EDX(QuickMatch::PlayerString);
//    return 0x64B156 + 0x9;
//}
//
//DEFINE_HOOK(0x648EA8, WaitForPlayers_HideName, 0x6)
//{
//    if ((Spawner::Enabled && Spawner::GetConfig()->QuickMatch) == false)
//        return 0;
//
//    R->EAX(QuickMatch::PlayerString);
//    return 0x648EB3;
//}
