/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Blowfish driven straw.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) Electronic Arts
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "blowstraw.h"

#include "asserthandler.h"
#include "fatal.h"

#include <cstring>


int BlowStraw::Get(void * source, int slen)
{
    if (source == nullptr || slen <= 0) {
        return 0;
    }

    if (BF == nullptr) {
        return Straw::Get(source, slen);
    }

    int total = 0;

    while (slen > 0) {

        if (Counter > 0) {
            int sublen = (slen < Counter) ? slen : Counter;
            std::memmove(source, &Buffer[sizeof(Buffer)-Counter], sublen);
            Counter -= sublen;
            source = ((char *)source) + sublen;
            slen -= sublen;
            total += sublen;
        }
        if (slen == 0) break;

        int incount = Straw::Get(Buffer, sizeof(Buffer));
        if (incount == 0) break;

        if (incount == sizeof(Buffer)) {
            if (Control == DECRYPT) {
                BF->Decrypt(Buffer, incount, Buffer);
            } else {
                BF->Encrypt(Buffer, incount, Buffer);
            }
        } else {
            std::memmove(&Buffer[sizeof(Buffer)-incount], Buffer, incount);
        }
        Counter = incount;
    }

    return total;
}


void BlowStraw::Key(void const * key, int length)
{
    if (BF == nullptr) {
        BF = new BlowfishEngine;
    }

    ASSERT(BF != nullptr);

    if (BF != nullptr) {
        BF->Submit_Key(key, length);
    }
}
