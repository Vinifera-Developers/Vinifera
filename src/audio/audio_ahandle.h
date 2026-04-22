/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  VQA movie audio handler using miniaudio streaming.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "dsaudio.h"
#include "vqa.h"

struct AhandleInitParams;

class AudioHandleClass
{
public:
    static unsigned long __cdecl Timer_Callback_Audio_Handler(VQAHandle* vqa);
    static long __cdecl Stream_Audio_Handler(VQAHandle* vqa, long action, void* buffer, long nbytes);
    static unsigned long __cdecl Simple_Timer_Callback_Audio_Handler(VQAHandle* vqa);
    static long __cdecl Lock_Audio_Handler();
    static long __cdecl Unlock_Audio_Handler();

private:
    static unsigned long Get_Total_Bytes_Played(VQAHandle* vqa, VQAConfig* config);

    static long __cdecl Open_Audio_Handler(VQAHandleP* vqap, AhandleInitParams* params, long b);
    static long __cdecl Close_Audio_Handler(VQAHandleP* vqap);
    static long __cdecl Start_Audio_Handler(VQAHandleP* vqap);
    static long __cdecl Load_Audio_Handler(VQAHandleP* vqap, void* buffer, long nbytes);
    static long __cdecl Pause_Audio_Handler(VQAHandleP* vqap);
    static long __cdecl Play_Audio_Handler(VQAHandleP* vqap);
    static long __cdecl Stop_Audio_Handler(VQAHandleP* vqap);
};
