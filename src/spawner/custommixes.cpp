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
//#include <HardEndStuff/Ra2Mode.h>
//
//#include <Utilities/Macro.h>
//#include <MixFileClass.h>
//
//MixFileClass* Ra2ModeMIX = nullptr;
//MixFileClass* CnCnetMIX = nullptr;
//
//DEFINE_HOOK(0x6BE9BD, ProgEnd_CustomMixes, 0x6)
//{
//	if (Ra2ModeMIX)
//	{
//		GameDelete(Ra2ModeMIX);
//		Ra2ModeMIX = nullptr;
//	}
//
//	if (CnCnetMIX)
//	{
//		GameDelete(CnCnetMIX);
//		CnCnetMIX = nullptr;
//	}
//
//	return 0;
//}
//
//DEFINE_HOOK(0x6BD7DC, InitBootstrapMixFiles_CustomMixes, 0x5)
//{
//	ProgEnd_CustomMixes(R);
//
//	if (Ra2Mode::IsEnabled())
//		Ra2ModeMIX = GameCreate<MixFileClass>("ra2mode.mix");
//
//	CnCnetMIX = GameCreate<MixFileClass>("cncnet.mix");
//
//	return 0;
//}
