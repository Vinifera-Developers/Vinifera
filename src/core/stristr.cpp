/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Case insensitive strstr implementation.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "stristr.h"

#include <cstdlib>
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
