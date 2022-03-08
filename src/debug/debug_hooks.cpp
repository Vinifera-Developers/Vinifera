/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          DEBUG_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the debug hooks and patches.
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
#include "setup_hooks.h"
#include "asserthandler.h"
#include "debughandler.h"
#include "purecallhandler.h"
#include "exceptionhandler.h"
#include "vinifera_globals.h"
#include "tspp_assert.h"
#include "winutil.h"
#include "hooker.h"
#include "hooker_macros.h"
#include <string>
#include <stdarg.h>


/**
 *  Prints an string to the debug handler.
 * 
 *  @author: CCHyper
 */
static void __cdecl Debug_Print(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    std::string tmp = fmt;

    char buffer[4096];
    vsprintf(buffer, tmp.c_str(), args);

    DEBUG_GAME(buffer);

    va_end(args);
}


/**
 *  Prints an string without a carriage return to the debug handler.
 * 
 *  @author: CCHyper
 */
static void __cdecl Debug_Print_Line(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    std::string tmp = fmt;

    char buffer[4096];
    vsprintf(buffer, tmp.c_str(), args);

    DEBUG_GAME_LINE(buffer);

    va_end(args);
}


/**
 *  Prints an warning string to the debug handler.
 * 
 *  @author: CCHyper
 */
static void __cdecl Debug_Print_Warning(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    std::string tmp = fmt;

    char buffer[4096];
    vsprintf(buffer, tmp.c_str(), args);

    DEBUG_WARNING(buffer);

    va_end(args);
}


/**
 *  Prints an error string to the debug handler.
 * 
 *  @author: CCHyper
 */
static void __cdecl Debug_Print_Error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    std::string tmp = fmt;

    char buffer[4096];
    vsprintf(buffer, tmp.c_str(), args);

    DEBUG_ERROR(buffer);

    va_end(args);
}


/**
 *  Dummy function to silencing a debug print.
 * 
 *  @author: CCHyper
 */
static void __cdecl Debug_Print_Dummy(const char *fmt, ...)
{
}


/**
 *  Patch some error and warning calls to use specific debug outputs.
 * 
 *  @author: CCHyper
 */
static void Debug_Print_Patch()
{
    Patch_Call(0x00419A4B, &Debug_Print_Warning);
    Patch_Call(0x00419B67, &Debug_Print_Warning);
    Patch_Call(0x0041D95C, &Debug_Print_Warning);
    Patch_Call(0x0041D9A8, &Debug_Print_Warning);
    Patch_Call(0x0041DA2F, &Debug_Print_Warning);
    Patch_Call(0x0044EB65, &Debug_Print_Error);
    Patch_Call(0x0044EBB9, &Debug_Print_Error);
    Patch_Call(0x0044EC0D, &Debug_Print_Error);
    Patch_Call(0x0044ECAE, &Debug_Print_Error);
    Patch_Call(0x0044ED18, &Debug_Print_Error);
    Patch_Call(0x0044ED36, &Debug_Print_Error);
    Patch_Call(0x0044EE67, &Debug_Print_Error);
    Patch_Call(0x0044EEAC, &Debug_Print_Error);
    Patch_Call(0x0044EF5C, &Debug_Print_Error);
    Patch_Call(0x0044F051, &Debug_Print_Error);
    Patch_Call(0x0044F09A, &Debug_Print_Error);
    Patch_Call(0x0044F0E7, &Debug_Print_Error);
    Patch_Call(0x0044F119, &Debug_Print_Error);
    Patch_Call(0x0044F22E, &Debug_Print_Error);
    Patch_Call(0x0044F273, &Debug_Print_Error);
    Patch_Call(0x0044F2C0, &Debug_Print_Error);
    Patch_Call(0x0044F2F2, &Debug_Print_Error);
    Patch_Call(0x0044F428, &Debug_Print_Error);
    Patch_Call(0x0044F463, &Debug_Print_Error);
    Patch_Call(0x0044F4B0, &Debug_Print_Error);
    Patch_Call(0x0044F4E2, &Debug_Print_Error);
    Patch_Call(0x0044F59F, &Debug_Print_Error);
    Patch_Call(0x0044F61E, &Debug_Print_Error);
    Patch_Call(0x0044F68E, &Debug_Print_Error);
    Patch_Call(0x0044F6F7, &Debug_Print_Error);
    Patch_Call(0x0044F752, &Debug_Print_Error);
    Patch_Call(0x0044F797, &Debug_Print_Error);
    Patch_Call(0x0044F843, &Debug_Print_Error);
    Patch_Call(0x0044F88E, &Debug_Print_Error);
    Patch_Call(0x0044F8D4, &Debug_Print_Error);
    Patch_Call(0x0044F914, &Debug_Print_Error);
    Patch_Call(0x0044F94A, &Debug_Print_Error);
    Patch_Call(0x0044FA00, &Debug_Print_Error);
    Patch_Call(0x0044FA4A, &Debug_Print_Error);
    Patch_Call(0x0044FA97, &Debug_Print_Error);
    Patch_Call(0x0044FAC9, &Debug_Print_Error);
    Patch_Call(0x0044FB90, &Debug_Print_Error);
    Patch_Call(0x0044FBDA, &Debug_Print_Error);
    Patch_Call(0x0044FC27, &Debug_Print_Error);
    Patch_Call(0x0044FC59, &Debug_Print_Error);
    Patch_Call(0x0044FCDF, &Debug_Print_Error);
    Patch_Call(0x0044FD0A, &Debug_Print_Error);
    Patch_Call(0x0044FD7C, &Debug_Print_Error);
    Patch_Call(0x0044FD96, &Debug_Print_Error);
    Patch_Call(0x0044FE3D, &Debug_Print_Error);
    Patch_Call(0x0044FE7F, &Debug_Print_Error);
    Patch_Call(0x0044FF5D, &Debug_Print_Error);
    Patch_Call(0x0044FF9F, &Debug_Print_Error);
    Patch_Call(0x00450136, &Debug_Print_Error);
    Patch_Call(0x00450158, &Debug_Print_Error);
    Patch_Call(0x00450202, &Debug_Print_Error);
    Patch_Call(0x00450294, &Debug_Print_Error);
    Patch_Call(0x004503A0, &Debug_Print_Error);
    Patch_Call(0x004503E5, &Debug_Print_Error);
    Patch_Call(0x004503F4, &Debug_Print_Error);
    Patch_Call(0x00450403, &Debug_Print_Error);
    Patch_Call(0x00450412, &Debug_Print_Error);
    Patch_Call(0x00450422, &Debug_Print_Error);
    Patch_Call(0x004505AB, &Debug_Print_Error);
    Patch_Call(0x00456241, &Debug_Print_Error);
    Patch_Call(0x00460FE8, &Debug_Print_Error);
    Patch_Call(0x00461004, &Debug_Print_Warning);
    Patch_Call(0x004611FE, &Debug_Print_Error);
    Patch_Call(0x00461F3C, &Debug_Print_Error);
    Patch_Call(0x00463298, &Debug_Print_Error);
    Patch_Call(0x004632E3, &Debug_Print_Error);
    Patch_Call(0x004632F2, &Debug_Print_Error);
    Patch_Call(0x00472B0E, &Debug_Print_Error);
    Patch_Call(0x00472B4B, &Debug_Print_Error);
    Patch_Call(0x00472E3E, &Debug_Print_Error);
    Patch_Call(0x00472E79, &Debug_Print_Error);
    Patch_Call(0x00472F78, &Debug_Print_Error);
    Patch_Call(0x00472FB9, &Debug_Print_Error);
    Patch_Call(0x00487B62, &Debug_Print_Error);
    Patch_Call(0x00487C98, &Debug_Print_Error);
    Patch_Call(0x00487CB4, &Debug_Print_Error);
    Patch_Call(0x00487D3E, &Debug_Print_Error);
    Patch_Call(0x00487EA2, &Debug_Print_Error);
    Patch_Call(0x00487F6F, &Debug_Print_Error);
    Patch_Call(0x00487F92, &Debug_Print_Warning);
    Patch_Call(0x00488027, &Debug_Print_Error);
    Patch_Call(0x0048803F, &Debug_Print_Warning);
    Patch_Call(0x004880D8, &Debug_Print_Error);
    Patch_Call(0x004881B0, &Debug_Print_Error);
    Patch_Call(0x00488258, &Debug_Print_Error);
    Patch_Call(0x004883AC, &Debug_Print_Warning);
    Patch_Call(0x0048842D, &Debug_Print_Warning);
    Patch_Call(0x004884BB, &Debug_Print_Warning);
    Patch_Call(0x00488596, &Debug_Print_Warning);
    Patch_Call(0x004886C2, &Debug_Print_Warning);
    Patch_Call(0x00488716, &Debug_Print_Warning);
    Patch_Call(0x004887FC, &Debug_Print_Warning);
    Patch_Call(0x00488870, &Debug_Print_Warning);
    Patch_Call(0x00488900, &Debug_Print_Warning);
    Patch_Call(0x004889F8, &Debug_Print_Warning);
    Patch_Call(0x00488B49, &Debug_Print_Warning);
    Patch_Call(0x00488CB7, &Debug_Print_Error);
    Patch_Call(0x0048966E, &Debug_Print_Warning);
    Patch_Call(0x004896F4, &Debug_Print_Warning);
    Patch_Call(0x00489A93, &Debug_Print_Warning);
    Patch_Call(0x00489B61, &Debug_Print_Warning);
    Patch_Call(0x00489DD7, &Debug_Print_Warning);
    Patch_Call(0x00489E89, &Debug_Print_Warning);
    Patch_Call(0x00489F94, &Debug_Print_Warning);
    Patch_Call(0x0048A070, &Debug_Print_Warning);
    Patch_Call(0x0048A1A1, &Debug_Print_Warning);
    Patch_Call(0x0048A290, &Debug_Print_Warning);
    Patch_Call(0x0048A380, &Debug_Print_Warning);
    Patch_Call(0x0048A463, &Debug_Print_Warning);
    Patch_Call(0x0048A583, &Debug_Print_Warning);
    Patch_Call(0x0048A62C, &Debug_Print_Warning);
    Patch_Call(0x0048AF1B, &Debug_Print_Error);
    Patch_Call(0x0048B22A, &Debug_Print_Error);
    Patch_Call(0x0049555A, &Debug_Print_Error);
    Patch_Call(0x0049569A, &Debug_Print_Error);
    Patch_Call(0x0049572D, &Debug_Print_Error);
    Patch_Call(0x00495A24, &Debug_Print_Error);
    Patch_Call(0x00496359, &Debug_Print_Error);
    Patch_Call(0x00496454, &Debug_Print_Error);
    Patch_Call(0x00498839, &Debug_Print_Error);
    Patch_Call(0x0049886C, &Debug_Print_Error);
    Patch_Call(0x004AB962, &Debug_Print_Warning);
    Patch_Call(0x004BE280, &Debug_Print_Warning);
    Patch_Call(0x004BE2D9, &Debug_Print_Warning);
    Patch_Call(0x004BE30E, &Debug_Print_Warning);
    Patch_Call(0x004BE3F7, &Debug_Print_Warning);
    Patch_Call(0x004BECBD, &Debug_Print_Warning);
    Patch_Call(0x004DFBAC, &Debug_Print_Error);
    Patch_Call(0x004DFBF0, &Debug_Print_Error);
    Patch_Call(0x004DFD28, &Debug_Print_Error);
    Patch_Call(0x004E049B, &Debug_Print_Error);
    Patch_Call(0x004E0509, &Debug_Print_Error);
    Patch_Call(0x004E06DD, &Debug_Print_Error);
    Patch_Call(0x004E06FA, &Debug_Print_Warning);
    Patch_Call(0x004E08AB, &Debug_Print_Error);
    Patch_Call(0x004E093A, &Debug_Print_Error);
    Patch_Call(0x004E09AA, &Debug_Print_Error);
    Patch_Call(0x004E09FC, &Debug_Print_Error);
    Patch_Call(0x004E0A34, &Debug_Print_Error);
    Patch_Call(0x004E0AD1, &Debug_Print_Error);
    Patch_Call(0x004E0B1A, &Debug_Print_Error);
    Patch_Call(0x004E1149, &Debug_Print_Error);
    Patch_Call(0x004E123E, &Debug_Print_Error);
    Patch_Call(0x004E15A3, &Debug_Print_Error);
    Patch_Call(0x004E48C9, &Debug_Print_Error);
    Patch_Call(0x004E493C, &Debug_Print_Error);
    Patch_Call(0x004E723B, &Debug_Print_Error);
    Patch_Call(0x004E816E, &Debug_Print_Error);
    Patch_Call(0x004E8191, &Debug_Print_Error);
    Patch_Call(0x004E8426, &Debug_Print_Error);
    Patch_Call(0x004E86C5, &Debug_Print_Error);
    Patch_Call(0x004EFE0A, &Debug_Print_Error);
    Patch_Call(0x004F093F, &Debug_Print_Error);
    Patch_Call(0x004F0D2B, &Debug_Print_Warning);
    Patch_Call(0x004F0D7D, &Debug_Print_Error);
    Patch_Call(0x0055372D, &Debug_Print_Error);
    Patch_Call(0x00553CD0, &Debug_Print_Error);
    Patch_Call(0x00553CFC, &Debug_Print_Error);
    Patch_Call(0x0055C2D8, &Debug_Print_Error);
    Patch_Call(0x0055FC7E, &Debug_Print_Error);
    Patch_Call(0x00560915, &Debug_Print_Error);
    Patch_Call(0x00560B91, &Debug_Print_Error);
    Patch_Call(0x00560F2D, &Debug_Print_Error);
    Patch_Call(0x00560F6A, &Debug_Print_Error);
    Patch_Call(0x00561634, &Debug_Print_Error);
    Patch_Call(0x005684BA, &Debug_Print_Error);
    Patch_Call(0x0056E273, &Debug_Print_Error);
    Patch_Call(0x0056E383, &Debug_Print_Error);
    Patch_Call(0x0056E3D9, &Debug_Print_Error);
    Patch_Call(0x0056E5F4, &Debug_Print_Error);
    Patch_Call(0x005724E7, &Debug_Print_Error);
    Patch_Call(0x0057B56E, &Debug_Print_Error);
    Patch_Call(0x0057D78E, &Debug_Print_Warning);
    Patch_Call(0x0057EDDF, &Debug_Print_Warning);
    Patch_Call(0x00581BBC, &Debug_Print_Error);
    Patch_Call(0x00581C8F, &Debug_Print_Error);
    Patch_Call(0x00581D4D, &Debug_Print_Error);
    Patch_Call(0x00581E2B, &Debug_Print_Error);
    Patch_Call(0x00581E42, &Debug_Print_Error);
    Patch_Call(0x00581E67, &Debug_Print_Error);
    Patch_Call(0x00581E8C, &Debug_Print_Error);
    Patch_Call(0x00581EAE, &Debug_Print_Error);
    Patch_Call(0x00581ED0, &Debug_Print_Error);
    Patch_Call(0x00581EF2, &Debug_Print_Error);
    Patch_Call(0x005820BC, &Debug_Print_Error);
    Patch_Call(0x0058218F, &Debug_Print_Error);
    Patch_Call(0x0058224D, &Debug_Print_Error);
    Patch_Call(0x00582306, &Debug_Print_Warning);
    Patch_Call(0x0058231D, &Debug_Print_Warning);
    Patch_Call(0x0058233F, &Debug_Print_Warning);
    Patch_Call(0x00582361, &Debug_Print_Warning);
    Patch_Call(0x00582383, &Debug_Print_Warning);
    Patch_Call(0x00582893, &Debug_Print_Warning);
    Patch_Call(0x00582E6F, &Debug_Print_Warning);
    Patch_Call(0x00582F04, &Debug_Print_Error);
    Patch_Call(0x005831ED, &Debug_Print_Error);
    Patch_Call(0x00583489, &Debug_Print_Error);
    Patch_Call(0x00583549, &Debug_Print_Error);
    Patch_Call(0x0058360D, &Debug_Print_Error);
    Patch_Call(0x005836BC, &Debug_Print_Error);
    Patch_Call(0x0058384F, &Debug_Print_Error);
    Patch_Call(0x00583898, &Debug_Print_Error);
    Patch_Call(0x00583B24, &Debug_Print_Warning);
    Patch_Call(0x00583EF1, &Debug_Print_Warning);
    Patch_Call(0x0058C949, &Debug_Print_Warning);
    Patch_Call(0x005B1E56, &Debug_Print_Error);
    Patch_Call(0x005B226C, &Debug_Print_Warning);
    Patch_Call(0x005B4300, &Debug_Print_Error);
    Patch_Call(0x005B5974, &Debug_Print_Error);
    Patch_Call(0x005B8478, &Debug_Print_Warning);
    Patch_Call(0x005C0B07, &Debug_Print_Error);
    Patch_Call(0x005C0C66, &Debug_Print_Error);
    Patch_Call(0x005D515E, &Debug_Print_Error);
    Patch_Call(0x005D51CE, &Debug_Print_Error);
    Patch_Call(0x005D5288, &Debug_Print_Error);
    Patch_Call(0x005D5341, &Debug_Print_Error);
    Patch_Call(0x005D53F4, &Debug_Print_Error);
    Patch_Call(0x005D5613, &Debug_Print_Error);
    Patch_Call(0x005D565D, &Debug_Print_Error);
    Patch_Call(0x005D6160, &Debug_Print_Error);
    Patch_Call(0x005D639F, &Debug_Print_Error);
    Patch_Call(0x005D63D3, &Debug_Print_Error);
    Patch_Call(0x005D6407, &Debug_Print_Error);
    Patch_Call(0x005D643B, &Debug_Print_Error);
    Patch_Call(0x005D646F, &Debug_Print_Error);
    Patch_Call(0x005D64A3, &Debug_Print_Error);
    Patch_Call(0x005D64D7, &Debug_Print_Error);
    Patch_Call(0x005D650B, &Debug_Print_Error);
    Patch_Call(0x005D653F, &Debug_Print_Error);
    Patch_Call(0x005D6573, &Debug_Print_Error);
    Patch_Call(0x005D65A7, &Debug_Print_Error);
    Patch_Call(0x005D65DB, &Debug_Print_Error);
    Patch_Call(0x005D660F, &Debug_Print_Error);
    Patch_Call(0x005D6643, &Debug_Print_Error);
    Patch_Call(0x005D6677, &Debug_Print_Error);
    Patch_Call(0x005D66AB, &Debug_Print_Error);
    Patch_Call(0x005D66DF, &Debug_Print_Error);
    Patch_Call(0x005D6713, &Debug_Print_Error);
    Patch_Call(0x005D6747, &Debug_Print_Error);
    Patch_Call(0x005D677B, &Debug_Print_Error);
    Patch_Call(0x005D67AF, &Debug_Print_Error);
    Patch_Call(0x005D67E3, &Debug_Print_Error);
    Patch_Call(0x005D6817, &Debug_Print_Error);
    Patch_Call(0x005D684B, &Debug_Print_Error);
    Patch_Call(0x005D687A, &Debug_Print_Error);
    Patch_Call(0x005D68A9, &Debug_Print_Error);
    Patch_Call(0x005D68E5, &Debug_Print_Error);
    Patch_Call(0x005D78D8, &Debug_Print_Error);
    Patch_Call(0x005D91B2, &Debug_Print_Error);
    Patch_Call(0x005DB8ED, &Debug_Print_Warning);
    Patch_Call(0x005DB934, &Debug_Print_Warning);
    Patch_Call(0x005DBED2, &Debug_Print_Error);
    Patch_Call(0x005DD1C0, &Debug_Print_Error);
    Patch_Call(0x005EA9E7, &Debug_Print_Warning);
    Patch_Call(0x005EF1F7, &Debug_Print_Error);
    Patch_Call(0x005FFCA8, &Debug_Print_Error);
    Patch_Call(0x005FFCF8, &Debug_Print_Error);
    Patch_Call(0x005FFD1C, &Debug_Print_Error);
    Patch_Call(0x006655B3, &Debug_Print_Error);
    Patch_Call(0x0066561F, &Debug_Print_Error);
    Patch_Call(0x00681DBA, &Debug_Print_Error);
    Patch_Call(0x00681EB5, &Debug_Print_Error);
    Patch_Call(0x00681EF4, &Debug_Print_Error);
    Patch_Call(0x00681FF0, &Debug_Print_Error);
    Patch_Call(0x00682007, &Debug_Print_Error);
    Patch_Call(0x0068204B, &Debug_Print_Error);
    Patch_Call(0x00682106, &Debug_Print_Error);
    Patch_Call(0x0068212B, &Debug_Print_Error);
    Patch_Call(0x00682228, &Debug_Print_Error);
    Patch_Call(0x006822F9, &Debug_Print_Error);
    Patch_Call(0x00682404, &Debug_Print_Error);
    Patch_Call(0x006825A7, &Debug_Print_Error);
    Patch_Call(0x006829D9, &Debug_Print_Error);
    Patch_Call(0x00682B1D, &Debug_Print_Error);
    Patch_Call(0x00682CEC, &Debug_Print_Error);
    Patch_Call(0x00682DDA, &Debug_Print_Error);
    Patch_Call(0x00682DF3, &Debug_Print_Warning);
    Patch_Call(0x00682E05, &Debug_Print_Error);
    Patch_Call(0x00682E1E, &Debug_Print_Error);
    Patch_Call(0x00682E37, &Debug_Print_Error);
    Patch_Call(0x00682E50, &Debug_Print_Error);
    Patch_Call(0x00682E69, &Debug_Print_Error);
    Patch_Call(0x006885CA, &Debug_Print_Error);
    Patch_Call(0x0068CAD6, &Debug_Print_Error);
    Patch_Call(0x00692EB4, &Debug_Print_Error);
    Patch_Call(0x00697FCF, &Debug_Print_Warning);
    Patch_Call(0x0069A832, &Debug_Print_Warning);
    Patch_Call(0x0069BED7, &Debug_Print_Error);
    Patch_Call(0x0069C5FC, &Debug_Print_Error);
    Patch_Call(0x0069C852, &Debug_Print_Error);
    Patch_Call(0x006A03FF, &Debug_Print_Error);
    Patch_Call(0x006A040E, &Debug_Print_Warning);
    Patch_Call(0x006A0641, &Debug_Print_Error);
    Patch_Call(0x006A0687, &Debug_Print_Error);
    Patch_Call(0x006A075E, &Debug_Print_Error);
    Patch_Call(0x006A07CC, &Debug_Print_Error);
    Patch_Call(0x006A081D, &Debug_Print_Error);
    Patch_Call(0x006A086E, &Debug_Print_Error);
    Patch_Call(0x006A0B30, &Debug_Print_Warning);
    Patch_Call(0x006A1058, &Debug_Print_Error);
    Patch_Call(0x006A11FB, &Debug_Print_Error);
    Patch_Call(0x006A126A, &Debug_Print_Error);
    Patch_Call(0x006A1382, &Debug_Print_Error);
    Patch_Call(0x006A1770, &Debug_Print_Error);
    Patch_Call(0x006A17B5, &Debug_Print_Error);
    Patch_Call(0x006A17F0, &Debug_Print_Error);
    Patch_Call(0x006A223B, &Debug_Print_Error);
    Patch_Call(0x006A2692, &Debug_Print_Warning);
    Patch_Call(0x006A4E61, &Debug_Print_Warning);
    Patch_Call(0x006A4F09, &Debug_Print_Warning);
    Patch_Call(0x006A4F6E, &Debug_Print_Warning);
    Patch_Call(0x006A52E9, &Debug_Print_Warning);
    Patch_Call(0x006A5321, &Debug_Print_Warning);
    Patch_Call(0x006A53EF, &Debug_Print_Warning);
    Patch_Call(0x006A5741, &Debug_Print_Warning);
    Patch_Call(0x006A5839, &Debug_Print_Warning);
    Patch_Call(0x006A5A77, &Debug_Print_Warning);
    Patch_Call(0x006A5C26, &Debug_Print_Warning);
    Patch_Call(0x006A5E0B, &Debug_Print_Warning);
    Patch_Call(0x006A5E64, &Debug_Print_Warning);
    Patch_Call(0x006A5F5C, &Debug_Print_Warning);
    Patch_Call(0x006A6025, &Debug_Print_Warning);
    Patch_Call(0x006A6061, &Debug_Print_Warning);
    Patch_Call(0x006A6184, &Debug_Print_Warning);
    Patch_Call(0x006A61BF, &Debug_Print_Warning);
    Patch_Call(0x006A629A, &Debug_Print_Warning);
    Patch_Call(0x006A633A, &Debug_Print_Warning);
    Patch_Call(0x006A648F, &Debug_Print_Warning);
    Patch_Call(0x006A6505, &Debug_Print_Warning);
    Patch_Call(0x006A65DD, &Debug_Print_Warning);
    Patch_Call(0x006A66FA, &Debug_Print_Warning);
    Patch_Call(0x006A679F, &Debug_Print_Warning);
    Patch_Call(0x006A67FF, &Debug_Print_Warning);
    Patch_Call(0x006A6862, &Debug_Print_Warning);
    Patch_Call(0x006A68D5, &Debug_Print_Warning);
}


/**
 *  Patch in the debug output handler.
 * 
 *  @author: CCHyper
 */
static void Debug_Handler_Hooks()
{
    /**
     *  Take control of the games debut output functions.
     */
    Hook_Function(0x004082D0, &Debug_Print);
    Hook_Function(0x004735C0, &Debug_Print_Line);

    /**
     *  Fixup strings to output to new warning and error outputs.
     */
    Debug_Print_Patch();

    /**
     *  Skips the class size logging done at startup.
     */
    Patch_Jump(0x005FF81C, 0x005FFC41);

    /**
     *  
     */
    Vinifera_Debug_Handler_Startup();
}


/**
 *  The assertion handler to install.
 * 
 *  @author: CCHyper
 */
static void Vinifera_Assert_Handler(TSPPAssertType type, const char *expr, const char *file, int line, const char *function, volatile bool *ignore, volatile bool *allow_break, volatile bool *exit, const char *msg, ...)
{
    va_list args;
    static char buf[4096];

    va_start(args, msg);
    vsnprintf(buf, sizeof(buf), msg, args);

    Vinifera_Assert((AssertType)type, expr, file, line, function, ignore, allow_break, exit, msg, args);

    va_end(args);
}


/**
 *  Installs the assertion handler for the TS++ library.
 * 
 *  @author: CCHyper
 */
static void Assert_Handler_Hooks()
{
    TSPP_Install_Assertion_Handler(Vinifera_Assert_Handler);

    TSPP_IgnoreAllAsserts = false;
    TSPP_SilentAsserts = false;
    TSPP_ExitOnAssert = false;
}


/**
 *  Exception handlers to all out implementation.
 * 
 *  @author: CCHyper
 */
static LONG __stdcall _Top_Level_Exception_Filter(EXCEPTION_POINTERS *e_info)
{
    return Vinifera_Exception_Handler(e_info->ExceptionRecord->ExceptionCode, e_info);
}

static void __cdecl _Structured_Exception_Translator(unsigned int code, EXCEPTION_POINTERS *e_info)
{
    Vinifera_Exception_Handler(code, e_info);
}


void Debug_Hooks()
{
    Debug_Handler_Hooks();
    Assert_Handler_Hooks();

    /**
     *  Set the runtime pure call handler.
     */
    _set_purecall_handler(Vinifera_PureCall_Handler);
    Hook_Function(0x006B51E5, &Vinifera_PureCall_Handler);

    /**
     *  Hook in the Exception handler.
     */
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)&_Top_Level_Exception_Filter);
    _set_se_translator((_se_translator_function)&_Structured_Exception_Translator);
    Hook_Function(0x005FF7D0, &_Top_Level_Exception_Filter);
    Hook_Function(0x00496350, &Vinifera_Exception_Handler);
    //ASM_Hook_Function(0x00495610, Dump_Exception_Info);

    /**
     *  Change the exception dialog to use the developer dialog.
     */
    Patch_Dword(0x0049551B+1, 222); // +1 to skip the "push" opcode.
    Patch_Dword(0x00495576+1, 222);

#ifndef NDEBUG
    /**
     *  Enable developer mode if the debugger is attached, otherwise ask
     *  the user if they wish to enable it.
     */
    if (IsDebuggerPresent() /*|| (MessageBox(nullptr, "Enable developer mode?", "Vinifera", MB_YESNO) == IDYES)*/) {
        Vinifera_DeveloperMode = true;
    }
#endif

    /**
     *  Create the debug output directory.
     */
    CreateDirectory(Vinifera_DebugDirectory, nullptr);

    /**
     *  Cleanup debug files older than 14 days.
     */
    DEBUG_INFO("Running cleanup on debug folder...\n");
    DEBUG_INFO("(files older than 5 days will be removed)\n");
    DeleteFilesOlderThan(5, Vinifera_DebugDirectory, "DEBUG_*");
    DeleteFilesOlderThan(5, Vinifera_DebugDirectory, "STACK_*");
    DeleteFilesOlderThan(5, Vinifera_DebugDirectory, "EXCEPT_*");
    DeleteFilesOlderThan(5, Vinifera_DebugDirectory, "CRASHDUMP_*");
    DeleteFilesOlderThan(5, Vinifera_DebugDirectory, "MINIDUMP_*");
    DeleteFilesOlderThan(5, Vinifera_DebugDirectory, "DEBUG_*.ZIP");
}
