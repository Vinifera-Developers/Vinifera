/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Records recent game state changes for desync debugging.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include <cstdio>


class AbstractClass;
class AnimTypeClass;
class Coord;
class FootClass;
class Random2Class;


/**
 *  Records recent gameplay state changes (random number draws, facing changes,
 *  target assignments, mission overrides, animation creations) into fixed-size
 *  ring buffers so that the moments leading up to a desync can be dumped into
 *  the desync log by Extension::Print_CRCs.
 *
 *  Ported from ts-patches (log_more_oos.c).
 */
class SyncRecorder final
{
public:
    static void Record_RNG_Call(Random2Class* rng, unsigned caller);
    static void Record_RNG_Call(Random2Class* rng, unsigned caller, int minval, int maxval);
    static void Record_Facing_Change(unsigned dir, unsigned caller);
    static void Record_TarCom_Change(const AbstractClass* object, const AbstractClass* target, unsigned caller);
    static void Record_Override_Mission(const FootClass* foot, unsigned caller);
    static void Record_Anim_Construction(const AnimTypeClass* animtype, const Coord& coord, unsigned caller);

    static void Print_All(FILE* fp);

private:
    static void Print_RNG_Calls(FILE* fp, unsigned count);
    static void Print_Facing_Changes(FILE* fp, unsigned count);
    static void Print_TarCom_Changes(FILE* fp, unsigned count);
    static void Print_Override_Mission_Calls(FILE* fp, unsigned count);
    static void Print_Anim_Constructor_Calls(FILE* fp, unsigned count);
};
