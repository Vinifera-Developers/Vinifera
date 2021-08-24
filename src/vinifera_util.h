/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          VINIFERA_UTIL.H
 *
 *  @authors       CCHyper
 *
 *  @brief         Various utility functions.
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
#pragma once

#include "tibsun_defines.h"
#include "wwkeyboard.h"


class XSurface;


const char *Vinifera_Version_String();
const char *TSpp_Version_String();

void Vinifera_Draw_Version_Text(XSurface *surface, bool pre_init = false);

bool Vinifera_Generate_Mini_Dump();

int Vinifera_Do_WWMessageBox(const char *msg, const char *btn1, const char *btn2 = nullptr, const char *btn3 = nullptr);
void Vinifera_DeveloperMode_Warning_WWMessageBox(const char *msg, ...);

KeyNumType Get_Command_Key_From_Name(const char *name, KeyNumType default = KN_NONE);
