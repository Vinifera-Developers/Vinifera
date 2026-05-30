/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Blowfish driven pipe.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) Electronic Arts
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "blowpipe.h"

#include "asserthandler.h"
#include "fatal.h"

#include <cstring>


int BlowPipe::Flush()
{
    int total = 0;
    if (Counter > 0 && BF != nullptr) {
        total += Pipe::Put(Buffer, Counter);
    }
    Counter = 0;
    total += Pipe::Flush();
    return total;
}


int BlowPipe::Put(const void * source, int slen)
{
    if (source == nullptr || slen < 1) {
        return Pipe::Put(source, slen);
    }

    if (BF == nullptr) {
        return Pipe::Put(source, slen);
    }

    int total = 0;

    if (Counter) {
        int sublen = ((int)sizeof(Buffer)-Counter < slen) ? (sizeof(Buffer)-Counter) : slen;
        std::memmove(&Buffer[Counter], source, sublen);
        Counter += sublen;
        source = ((char *)source) + sublen;
        slen -= sublen;

        if (Counter == sizeof(Buffer)) {
            if (Control == DECRYPT) {
                BF->Decrypt(Buffer, sizeof(Buffer), Buffer);
            } else {
                BF->Encrypt(Buffer, sizeof(Buffer), Buffer);
            }
            total += Pipe::Put(Buffer, sizeof(Buffer));
            Counter = 0;
        }
    }

    while (slen >= sizeof(Buffer)) {
        if (Control == DECRYPT) {
            BF->Decrypt(source, sizeof(Buffer), Buffer);
        } else {
            BF->Encrypt(source, sizeof(Buffer), Buffer);
        }
        total += Pipe::Put(Buffer, sizeof(Buffer));
        source = ((char *)source) + sizeof(Buffer);
        slen -= sizeof(Buffer);
    }

    if (slen > 0) {
        std::memmove(Buffer, source, slen);
        Counter = slen;
    }

    return total;
}


void BlowPipe::Key(const void * key, int length)
{
    if (BF == nullptr) {
        BF = new BlowfishEngine;
    }

    ASSERT(BF != nullptr);

    if (BF != nullptr) {
        BF->Submit_Key(key, length);
    }
}
