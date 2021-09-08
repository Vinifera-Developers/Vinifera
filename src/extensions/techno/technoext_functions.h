/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TECHNOEXT_FUNCTIONS.H
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the supporting functions for the extended TechnoClass.
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

#include "always.h"
#include "tibsun_defines.h"


class TechnoClass;
class EBoltClass;


EBoltClass *TechnoClassExtension_Electric_Zap(TechnoClass *this_ptr, TARGET target, int which, const WeaponTypeClass *weapontype, Coordinate &source_coord);
EBoltClass *TechnoClassExtension_Electric_Bolt(TechnoClass *this_ptr, TARGET target);
