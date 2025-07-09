/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SHA.CPP
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
#include "sha.h"
#include <algorithm>


void SHA::Process_Partial(void const * & data, long & length)
{
    if (length == 0 || data == nullptr) return;
    if (PartialCount == 0 && length >= SRC_BLOCK_SIZE) return;

    int add_count = std::min((int)length, (int)SRC_BLOCK_SIZE - PartialCount);
    std::memcpy(&Partial[PartialCount], data, add_count);
    data = ((char const *&)data) + add_count;
    PartialCount += add_count;
    length -= add_count;

    if (PartialCount == SRC_BLOCK_SIZE) {
        Process_Block(&Partial[0], Acc);
        Length += (long)SRC_BLOCK_SIZE;
        PartialCount = 0;
    }
}


void SHA::Hash(void const * data, long length)
{
    IsCached = false;

    Process_Partial(data, length);

    if (length == 0) return;

    long blocks = (length / SRC_BLOCK_SIZE);
    long const * source = (long const *)data;
    for (int bcount = 0; bcount < blocks; bcount++) {
        Process_Block(source, Acc);
        Length += (long)SRC_BLOCK_SIZE;
        source += SRC_BLOCK_SIZE/sizeof(long);
        length -= (long)SRC_BLOCK_SIZE;
    }

    data = source;
    Process_Partial(data, length);
}


int SHA::Result(void * result) const
{
    if (IsCached) {
        std::memcpy(result, &FinalResult, sizeof(FinalResult));
    }

    long length = Length + PartialCount;
    int partialcount = PartialCount;
    char partial[SRC_BLOCK_SIZE];
    std::memcpy(partial, Partial, sizeof(Partial));

    partial[partialcount] = (char)0x80;

    SHADigest acc = Acc;
    if ((SRC_BLOCK_SIZE - partialcount) < 9) {
        if (partialcount+1 < SRC_BLOCK_SIZE) {
            std::memset(&partial[partialcount+1], '\0', SRC_BLOCK_SIZE - (partialcount+1));
        }
        Process_Block(&partial[0], acc);
        partialcount = 0;
    } else {
        partialcount++;
    }

    std::memset(&partial[partialcount], '\0', SRC_BLOCK_SIZE - partialcount);
    *(long *)(&partial[SRC_BLOCK_SIZE-4]) = _byteswap_ulong((length*8));
    Process_Block(&partial[0], acc);

    std::memcpy((char *)&FinalResult, &acc, sizeof(acc));
    for (int index = 0; index < sizeof(FinalResult)/sizeof(long); index++) {
        (long &)FinalResult.Long[index] = _byteswap_ulong(FinalResult.Long[index]);
    }
    (bool&)IsCached = true;
    std::memcpy(result, &FinalResult, sizeof(FinalResult));

    return sizeof(FinalResult);
}


void SHA::Process_Block(void const * source, SHADigest & acc) const
{
    long block[PROC_BLOCK_SIZE/sizeof(long)];
    long const * data = (long const *)source;
    int index;
    for (index = 0; index < SRC_BLOCK_SIZE/sizeof(long); index++) {
        block[index] = _byteswap_ulong(data[index]);
    }

    for (index = SRC_BLOCK_SIZE/sizeof(long); index < PROC_BLOCK_SIZE/sizeof(long); index++) {
        block[index] = _rotl(block[index-3] ^ block[index-8] ^ block[index-14] ^ block[index-16], 1);
    }

    SHADigest alt = acc;
    for (index = 0; index < PROC_BLOCK_SIZE/sizeof(long); index++) {
        long temp = _rotl(alt.Long[0], 5) +
                        Do_Function(index, alt.Long[1], alt.Long[2], alt.Long[3]) +
                        alt.Long[4] + block[index] + Get_Constant(index);
        alt.Long[4] = alt.Long[3];
        alt.Long[3] = alt.Long[2];
        alt.Long[2] = _rotl(alt.Long[1], 30);
        alt.Long[1] = alt.Long[0];
        alt.Long[0] = temp;
    }
    acc.Long[0] += alt.Long[0];
    acc.Long[1] += alt.Long[1];
    acc.Long[2] += alt.Long[2];
    acc.Long[3] += alt.Long[3];
    acc.Long[4] += alt.Long[4];
}


void SHA::Print(const void *buffer, char *output)
{
    for (int i = 0; i < 20; i++) {
        std::sprintf(&output[2 * i], "%02x", *(static_cast<const uint8_t *>(buffer) + i));
    }
}


int SHA::Print_Result(char *output)
{
    uint8_t buffer[20];

    int retval = Result(buffer);
    Print(buffer, output);
    return retval;
}
