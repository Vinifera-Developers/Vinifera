/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          STRISTR.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Case insensitive strstr implementation.
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
#include "stristr.h"
#include <cstring>


/**
 *  Case-insensitive strstr.
 * 
 *  @author: CCHyper
 */
char *stristr(const char *str, const char *str_search)
{
    char *sors, *subs, *res = nullptr;
    if ((sors = strdup(str)) != nullptr) {
        if ((subs = strdup(str_search)) != nullptr) {
            res = std::strstr(strlwr(sors), strlwr(subs));
            if (res != nullptr)
                res = (char *)(str + (res - sors));
            std::free(subs);
        }
        std::free(sors);
    }
    return res;
}