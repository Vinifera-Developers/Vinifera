/*******************************************************************************
/* O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 * @project       Vinifera
 *
 * @file          AUDIO_DECODERS.H
 *
 * @author        CCHyper
 *
 * @brief         Custom decoders for Miniaudio
 *
 * @license       Vinifera is free software: you can redistribute it and/or
 *                modify it under the terms of the GNU General Public License
 *                as published by the Free Software Foundation, either version
 *                3 of the License, or (at your option) any later version.
 *
 *                Vinifera is distributed in the hope that it will be
 *                useful, but WITHOUT ANY WARRANTY; without even the implied
 *                warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *                PURPOSE. See the GNU General Public License for more details.
 *
 *                You should have received a copy of the GNU General Public
 *                License along with this program.
 *                If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#pragma once

//#define MINIAUDIO_IMPLEMENTATION      // Not needed here as we just want header info!
#include <miniaudio/miniaudio.h>

// Uncomment this to remove the Westwood AUD decoder implementation.
//#define MA_NO_AUD

/**
 *  Backend table that declares all the custom decoders.
 */
extern const ma_decoding_backend_vtable * ma_custom_backend_vtable[1];
