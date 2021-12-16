/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SHA.H
 *
 *  @author        Joe L. Bostic (see notes below)
 *
 *  @contributors  CCHyper
 *
 *  @brief         Implementation of the Secure Hash Algorithm.
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
 *  @note          This file contains heavily modified code from the source code
 *                 released by Electronic Arts for the C&C Remastered Collection
 *                 under the GPL3 license. Source:
 *                 https://github.com/ElectronicArts/CnC_Remastered_Collection
 *
 ******************************************************************************/
#pragma once

#include "always.h"
#include <new>


class SHA
{
    public:
        SHA() :
            IsCached(false), FinalResult(), Length(0),
            PartialCount(0), Partial()
        {
            Acc.Long[0] = SA;
            Acc.Long[1] = SB;
            Acc.Long[2] = SC;
            Acc.Long[3] = SD;
            Acc.Long[4] = SE;
        };

        void Init() {
            new ((void*)this) SHA;
        };

        int Result(void * result) const;

        void Hash(void const * data, long length);

        int Print_Result(char *output);

        static int Digest_Size() { return sizeof(SHADigest); }

    private:
        typedef union {
            unsigned long Long[5];
            unsigned char Char[20];
        } SHADigest;

        enum : unsigned
        {
            SA=0x67452301L,
            SB=0xefcdab89L,
            SC=0x98badcfeL,
            SD=0x10325476L,
            SE=0xc3d2e1f0L,
            K1=0x5a827999L,
            K2=0x6ed9eba1L,
            K3=0x8f1bbcdcL,
            K4=0xca62c1d6L,
            SRC_BLOCK_SIZE=16*sizeof(long),
            PROC_BLOCK_SIZE=80*sizeof(long)
        };

        long Get_Constant(int index) const {
            if (index < 20) return K1;
            if (index < 40) return K2;
            if (index < 60) return K3;
            return K4;
        };

        long Function1(long X, long Y, long Z) const {
            return(Z ^ ( X & ( Y ^ Z ) ) );
        };

        long Function2(long X, long Y, long Z) const {
            return( X ^ Y ^ Z );
        };

        long Function3(long X, long Y, long Z) const {
            return( (X & Y) | (Z & (X | Y) ) );
        };

        long Function4(long X, long Y, long Z) const {
            return( X ^ Y ^ Z );
        };

        long Do_Function(int index, long X, long Y, long Z) const {
            if (index < 20) return Function1(X, Y, Z);
            if (index < 40) return Function2(X, Y, Z);
            if (index < 60) return Function3(X, Y, Z);
            return Function4(X, Y, Z);
        };

        void Process_Block(void const * source, SHADigest & acc) const;
        void Process_Partial(void const * & data, long & length);

        void Print(const void *buffer, char *stringbuff);

    private:
        bool IsCached;
        SHADigest FinalResult;
        SHADigest Acc;
        long Length;
        int PartialCount;
        char Partial[SRC_BLOCK_SIZE];
};
