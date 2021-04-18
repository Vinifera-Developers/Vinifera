/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          FIXEDSTRING.H
 *
 *  @authors       tomsons26
 *
 *  @brief         String container with a fixed buffer size.
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
#pragma once

#include "always.h"
#include "debughandler.h"


template<int SIZE>
class FixedString
{
    public:
        FixedString();
        ~FixedString() {}

        void operator+=(const char *string) { Append(string); }

        void Append(const char *string);
        int Format(const char *format, ...);
        void Clear();

        inline bool Empty() const { return Buffer[0] == '\0'; }
        inline char *Peek_Buffer() { return Buffer; }
        inline int Get_Length() const { return Length; }

    private:
        int Length;
        static char Buffer[SIZE+1];
};


template<int SIZE>
char FixedString<SIZE>::Buffer[SIZE+1];


template<int SIZE>
FixedString<SIZE>::FixedString() :
    Length(0)
{
    Buffer[0] = '\0';
}


template<int SIZE>
int FixedString<SIZE>::Format(const char *format, ...)
{
    va_list arg_list;
    va_start(arg_list, format);

    char temp_buffer[2048] = { 0 };
    int retval = vsnprintf(temp_buffer, sizeof(temp_buffer), format, arg_list);

    Append(temp_buffer);

    va_end(arg_list);

    return retval;
}


template<int SIZE>
void FixedString<SIZE>::Append(const char *string)
{
    int src_len = std::strlen(string);
    if (src_len + Length < SIZE-1) {
        Length += src_len;
        std::strcat(Buffer, string);
    } else {
        DEBUG_WARNING("FixedString<%d>::Append() - Unable to append string, string is too long or buffer is full!\n", SIZE);
    }
}


template<int SIZE>
void FixedString<SIZE>::Clear()
{
    Length = 0;
    Buffer[0] = '\0';
}
