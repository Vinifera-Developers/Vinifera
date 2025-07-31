/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TEXTPRINTEXT_HOOKS.CPP
 *
 *  @author        Rampastring
 *
 *  @brief         Contains the hooks for the extended text print functionality.
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

#include "tibsun_globals.h"
#include "tibsun_defines.h"

#include "wwfont.h"


#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-1085
 *
 *  Reimplements Format_Window_String to fix a bug where the width of the modified string
 *  can exceed the given max line length.
 *
 *  Reimplemented based on Red Alert source code.
 *
 *  @author: 03/27/1992  SB : Created.
 *           05/18/1995 JLB : Greatly revised for new font system.
 *           09/04/1996 BWG : Added '@' is treated as a carriage return for width calculations.
 *           02/20/2024 Rampastring : Modified for Tiberian Sun and fixed the aforementioned bug.
 */
int _Format_Window_String_Custom_Implementation(char* string, WWFontClass* font, int maxlinelen, int& width, int& height)
{
    int    linelen;
    int    lines = 0;
    width = 0;
    height = 0;

    // If no string was passed in, then there are no lines.
    if (!string)
        return 0;

    // If no font was passed in, return no lines.
    if (!font)
        return 0;

    // While there are more letters left divide the line up.
    while (*string) {
        linelen = 0;
        height += font->Get_Font_Height() + font->Get_Y_Spacing();
        lines++;

        /*
        **    Look for special line break character and force a line break when it is
        **    discovered.
        */
        if (*string == '@') {
            *string = '\r';
        }

        // While the current line is less then the max length...
        while (linelen < maxlinelen && *string != '\r' && *string != '\0' && *string != '@') {
            linelen += font->Char_Pixel_Width(*string);
            string++;
        }

        // if the line is too long...
        if (linelen >= maxlinelen) {

            /*
            **    Back up to an appropriate location to break.
            */
            while (linelen > maxlinelen || (*string != ' ' && *string != '\r' && *string != '\0' && *string != '@')) {
                linelen -= font->Char_Pixel_Width(*string);
                string--;
            }
        }

        /*
        **    Record the largest width of the worst case string.
        */
        if (linelen > width) {
            width = linelen;

            if (width > maxlinelen) {
                width = linelen;
            }
        }

        /*
        **    Force a break at the end of the line.
        */
        if (*string) {
            *string++ = '\r';
        }
    }

    return lines;
}


void TextPrintExtension_Hooks()
{
    Patch_Jump(0x00474960, &_Format_Window_String_Custom_Implementation);
}