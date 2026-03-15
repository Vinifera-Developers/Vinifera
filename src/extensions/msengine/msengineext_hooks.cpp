/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          MSENGINEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended MS classes.
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
#include "msengineext_hooks.h"
#include "msengine.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "ccfile.h"
#include "wstring.h"
#include "audio_manager.h"
#include "audio_util.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  Loads the sound effect file.
 * 
 *  @author: CCHyper
 */
static const void *MSEngine_Load_File(const char *fname)
{
    const void *fileptr = nullptr;

    RawFileClass file(fname);

    if (file.Is_Available()) {
        fileptr = Load_Alloc_Data(file);
    } else {
        fileptr = MFCD::Retrieve(fname);
    }

    return fileptr;
}


/**
 *  Returns the filename of the best available sound effect.
 * 
 *  @author: CCHyper
 */
static const char *MSEngine_Get_Available_Filename(const char *name)
{
    static char _fnamebuf[32];

    /**
     *  Remove the extension from the input filename.
     */
    std::strcpy(_fnamebuf, name);
    _fnamebuf[std::strlen(_fnamebuf)-4] = '\0';

    /**
     *  Attempt to play the sound effect in supported formats.
     *  Priority: OGG -> MP3 -> WAV-> AUD
     */
    std::string fname;
    bool found = false;

#if 0
    if (!Audio_Driver_Is_Direct_Sound()) {

        fname = Build_Audio_File_Name(_fnamebuf, "xOGG"); // Ogg disabled for now.
        if (Is_Sfx_File_Available(fname.Peek_Buffer())) {
            found = true;
        }

        if (!found) {
            fname = Build_Audio_File_Name(_fnamebuf, "xMP3"); // Mp3 disabled for now.
            if (Is_Sfx_File_Available(fname.Peek_Buffer())) {
                found = true;
            }
        }

        if (!found) {
            fname = Build_Audio_File_Name(_fnamebuf, "xWAV"); // Wav disabled for now.
            if (Is_Sfx_File_Available(fname.Peek_Buffer())) {
                found = true;
            }
        }

    }

    if (!found) {
        fname = Build_Audio_File_Name(_fnamebuf, "AUD");
        if (Is_Sfx_File_Available(fname.Peek_Buffer())) {
            found = true;
        }
    }

    if (found) {
        return strdup(fname.Peek_Buffer());
    }
#endif

    return nullptr;
}


DECLARE_PATCH(_MSSfxEntry_Constructor_Load_File_Patch)
{
    GET_REGISTER_STATIC(const char *, fname, edi);
    static const void *fileptr;
    static const char *filename;

    filename = MSEngine_Get_Available_Filename(fname);

    fileptr = MSEngine_Load_File(filename);

    _asm { mov eax, filename }
    _asm { mov [esi], eax } // MSSfxEntry.Name = eax

    _asm { mov eax, fileptr }

    _asm { mov edi, fname }
    _asm { xor ebx, ebx }

    JMP_REG(ecx, 0x0056F32F);
}


/**
 *  Main function for patching the hooks.
 */
void MSEngineExtension_Hooks()
{
    //Patch_Jump(0x0056F31D, &_MSSfxEntry_Constructor_Load_File_Patch); // completly broken, somehow.
}
