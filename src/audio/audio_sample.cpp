/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Audio sample class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "audio_sample.h"

#include "audio_debug.h"
#include "audio_manager.h"
#include "ccfile.h"


/**
 *  Checks whether the sample's source file exists and is accessible.
 *
 *  @author: CCHyper
 */
bool AudioSampleClass::Is_Available() const
{
    if (!CCFileClass(FileName.c_str()).Is_Available()) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_SAMPLE, "AudioSample::Is_Available - Unable to find \"%s\"!\n", FileName.c_str());
        return false;
    }

    return true;
}
