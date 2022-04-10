/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          OBJECTTYPEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended ObjectTypeClass.
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
#include "objecttypeext_hooks.h"
#include "objecttypeext_init.h"
#include "objecttypeext.h"
#include "objecttype.h"
#include "theatertype.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "scenario.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or deconstructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static class ObjectTypeClassFake final : public ObjectTypeClass
{
    public:
        void _Assign_Theater_Name(char *buffer, TheaterType theater);
        const ShapeFileStruct * _Get_Image_Data() const;
};


/**
 *  Reimplementation of ObjectTypeClass::Assign_Theater_Name to support new theater types.
 * 
 *  @author: CCHyper
 */
void ObjectTypeClassFake::_Assign_Theater_Name(char *buffer, TheaterType theater)
{
    if (theater != THEATER_NONE && theater < TheaterTypes.Count()) {

        char first = buffer[0];
        char second = buffer[1];

        /**
         *  Make sure characters are lowercase.
         */
        if (first >= 'A' && first <= 'Z') {
            first += ' ';
        }
        if (second >= 'A' && second <= 'Z') {
            second += ' ';
        }

        /**
         *  Make sure this is a new theater style filename before assigning the theater id.
         */
        if ((first == 'g' || first == 'n' || first == 'c') && (second == 'a' || second == 't')) {
            buffer[1] = TheaterTypeClass::ImageLetter_From(theater);
        }
    }
}


/**
 *  This patch replaces an inlined instance of ObjectTypeClass::Assign_Theater_Name
 *  with a direct call.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_ObjectTypeClass_Load_Theater_Art_Assign_Theater_Name_Theater_Patch)
{
    GET_REGISTER_STATIC(ObjectTypeClass *, this_ptr, edi);
    LEA_STACK_STATIC(char *, fullname, esp, 0x0C);
    LEA_STACK_STATIC(char *, destbuffer, esp, 0x08);

    this_ptr->Assign_Theater_Name(fullname, Scen->Theater);

    JMP(0x005889E2);
}


/**
 *  Reimplementation of ObjectTypeClass::Get_Image_Data with added assertion.
 * 
 *  @author: CCHyper
 */
const ShapeFileStruct * ObjectTypeClassFake::_Get_Image_Data() const
{
    if (Image == nullptr) {
        DEBUG_WARNING("Object %s has NULL image data!\n", Name());
    }

    return Image;
}


/**
 *  Main function for patching the hooks.
 */
void ObjectTypeClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    ObjectTypeClassExtension_Init();

    Patch_Jump(0x004101A0, &ObjectTypeClassFake::_Get_Image_Data);
    Patch_Jump(0x00588D00, &ObjectTypeClassFake::_Assign_Theater_Name);
    Patch_Jump(0x0058891D, &_ObjectTypeClass_Load_Theater_Art_Assign_Theater_Name_Theater_Patch);
}
