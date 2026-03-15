/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AUDIO_IO.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Simple stdio wrappers for Miniaudio’s read/seek/tell procs
 *                 that are required for the custom audio decoder.
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

//#define MINIAUDIO_IMPLEMENTATION      // Not needed here as we just want header info!
#include <miniaudio/miniaudio.h>

/**
 *  Backend table that declares all the custom VFS callabcks.
 */
extern ma_vfs_callbacks ma_custom_vfs_callbacks;
