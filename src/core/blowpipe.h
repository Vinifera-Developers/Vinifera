/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Blowfish driven pipe.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) Electronic Arts
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "blowfish.h"
#include "pipe.h"


class BlowPipe : public Pipe
{
    public:
        typedef enum CryptControl {
            ENCRYPT,
            DECRYPT
        } CryptControl;

    public:
        BlowPipe(CryptControl control) : BF(nullptr), Counter(0), Control(control) {}
        virtual ~BlowPipe() { delete BF; BF = nullptr; }

        virtual int Flush()override;
        virtual int Put(const void * source, int slen) override;

        void Key(const void * key, int length);

    protected:
        BlowfishEngine * BF;

    private:
        char Buffer[8];
        int Counter;
        CryptControl Control;

    private:
        BlowPipe(BlowPipe &) = delete;
        BlowPipe & operator = (const BlowPipe &) = delete;
};
