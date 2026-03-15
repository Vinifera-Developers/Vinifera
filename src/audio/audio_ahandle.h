/*******************************************************************************
/*                  O P E N  S O U R C E -- V I N I F E R A                   **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AUDIO_AHANDLE.H
 *
 *  @author        CCHyper, with suggestions and additional comments added by AI
 *
 *  @brief         VQA movie audio handler using miniaudio streaming.
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
 *  @note          This file contains heavily modified code from the source code
 *                 released by Electronic Arts for the C&C Remastered Collection
 *                 under the GPL3 license. Source:
 *                 https://github.com/ElectronicArts/CnC_Remastered_Collection
 *
 ******************************************************************************/
#pragma once

#include	"dsaudio.h"
#include	"vqa.h"

struct AhandleInitParams;

class AudioHandleClass
{
	public:
	
		static unsigned long __cdecl Timer_Callback_Audio_Handler(VQAHandle *vqa);
		static long __cdecl Stream_Audio_Handler(VQAHandle *vqa, long action, void *buffer, long nbytes);
		static unsigned long __cdecl Simple_Timer_Callback_Audio_Handler(VQAHandle *vqa);
		static long __cdecl Lock_Audio_Handler(void);
		static long __cdecl Unlock_Audio_Handler(void);

	private:
		static unsigned long Get_Total_Bytes_Played(VQAHandle *vqa, VQAConfig *config);

		static long __cdecl Open_Audio_Handler(VQAHandleP *vqap, AhandleInitParams *params, long b);
		static long __cdecl Close_Audio_Handler(VQAHandleP *vqap);
		static long __cdecl Start_Audio_Handler(VQAHandleP *vqap);
		static long __cdecl Load_Audio_Handler(VQAHandleP *vqap, void *buffer, long nbytes);
		static long __cdecl Pause_Audio_Handler(VQAHandleP *vqap);
		static long __cdecl Play_Audio_Handler(VQAHandleP *vqap);
		static long __cdecl Stop_Audio_Handler(VQAHandleP *vqap);

		static void __stdcall AudioHandleClass::AudioCallback(UINT, UINT, DWORD, DWORD, DWORD);
};
