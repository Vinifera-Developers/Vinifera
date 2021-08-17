/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          MIXFILEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended MixFileClass.
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
#include "mixfileext_hooks.h"
#include "mixfile.h"
#include "cdfile.h"
#include "vector.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


static DynamicVectorClass<void *> AllocPointers;


/**
 *  Allocates a buffer and loads the file into it.
 * 
 *  @author: 10/17/1994 JLB - Red Alert source code.
 *           CCHyper - Adjustments for Vinifera.
 */
static void * Load_Alloc_Data(FileClass &file)
{
	void *ptr = nullptr;
	long size = file.Size();

	ptr = new char [size];
	if (ptr) {
		file.Read(ptr, size);
	}

	return ptr;
}


/**
 *  Frees up any buffers we allocated.
 * 
 *  @author: CCHyper
 */
static void __cdecl Free_Alloc_Data()
{
    DEBUG_INFO("Cleanuping up locally loaded file memory...");

    for (int i = 0; i < AllocPointers.Count(); ++i) {
        delete [] AllocPointers[i];
    }

    AllocPointers.Clear();
}


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or deconstructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class MixFileClassFake : public MixFileClass
{
    public:
        static const void * _Retrieve(const char *filename);
};


/**
 *  Retrieves a pointer to the specified data file.
 * 
 *  @author: 08/23/1994 JLB - Red Alert source code.
 *           CCHyper - Adjustments for Vinifera.
 */
const void * MixFileClassFake::_Retrieve(const char * filename)
{
    void * ptr = nullptr;

    /**
     *  #issue-515
     * 
     *  Various game assets are loaded with Retrieve only, and as a result
     *  do not use the typical file searching interface. This addition makes
     *  the game check if the file exists in the root directory before searching
     *  the loaded mix files.
     * 
     *  @author: CCHyper
     */
    CDFileClass file;
    file.Set_Name(filename);
    if (file.Is_Available()) {
        ptr = Load_Alloc_Data(file);
        if (ptr) {
            if (!AllocPointers.Count()) {
                std::atexit(Free_Alloc_Data);
            }
            AllocPointers.Add(ptr);
            return ptr;
        }
    }

    /**
     *  Original code from Retrieve().
     */
    ptr = nullptr;
    Offset(filename, &ptr);
    return ptr;
}


/**
 *  Main function for patching the hooks.
 */
void MixFileClassExtension_Hooks()
{
    Patch_Jump(0x00559DE0, &MixFileClassFake::_Retrieve);
}
