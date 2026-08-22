/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Misc utility functions for common tasks.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include <string_view>
#include <time.h>
#include <vector>
#include <windows.h>


struct VoxelObject;
typedef int clockid_t;
enum KeyNumType;


char const *Get_Text_Time();

void Seconds_To_Hms(float seconds, int & h, int & m, int & s);

void Get_Time(int &hour, int &min, int &sec);
void Get_Full_Time(int &day, int &month, int &year, int &hour, int &min, int &sec);
const char *Get_Date_Time_String(bool filename_safe = false);
void Get_File_Id_String(char const *filename, char &str);
bool Get_File_Creation_Time(char const *filename, time_t &time);
bool Get_EXE_File_Header(char const *filename, IMAGE_FILE_HEADER *header);
bool Get_EXE_File_Header_From_Instance(HINSTANCE h, IMAGE_FILE_HEADER *f_hdr);
int Compare_EXE_Version(HINSTANCE h, char const *filename);
bool Get_Version_Info(char const *filename, VS_FIXEDFILEINFO *file_info);

void HexPrint32(const uint32_t *data, size_t size);
void HexPrint64(const uint64_t *data, size_t size);

int Clock_Get_Time(clockid_t id, struct timespec *ts);

bool Create_Directory(char const *name);
bool Directory_Exists(char const *name);

void Set_Working_Directory();

int Get_Last_System_Error();

bool Delete_File(char const *filename);
bool Rename_File(char const *filename, char const *new_filename);
bool Replace_File(char const *filename, char const *new_filename);
bool File_Exists(char const *filename);
bool WinAPI_File_Exists(const char *filename);

bool Is_Full_Path(const char *path);

const char *Get_User_Documents_Path();

const char *Filename_From_Path(const char *filename);

bool Parse_Boolean(const char* value, bool defval);

bool Key_Down(int key);


/**
 *  Convenient generator-like class for splitting and trimming strings.
 */
enum class StringSplitOptions : unsigned {
    None = 0,
    RemoveEmpty = 1 << 0,
    Trim = 1 << 1,
};

inline StringSplitOptions operator|(StringSplitOptions a, StringSplitOptions b)
{
    return static_cast<StringSplitOptions>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

inline bool HasFlag(StringSplitOptions v, StringSplitOptions f)
{
    return (static_cast<unsigned>(v) & static_cast<unsigned>(f)) != 0;
}

class SplitView
{
    std::string_view s_;
    char delim_;
    StringSplitOptions opts_;

public:
    SplitView(std::string_view s, char delim, StringSplitOptions opts = StringSplitOptions::None) : s_(s), delim_(delim), opts_(opts) {}

    class iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::string_view;
        using difference_type = std::ptrdiff_t;
        using pointer = const std::string_view*;
        using reference = const std::string_view&;

        iterator() : done_(true) {} // End iterator constructor

        iterator(std::string_view s, char delim, StringSplitOptions opts, bool done) : s_(s), delim_(delim), opts_(opts), pos_(0), done_(done)
        {
            if (!done_) advance();
        }

        reference operator*() const { return current_; }
        pointer operator->() const { return &current_; }

        iterator& operator++()
        {
            advance();
            return *this;
        }

        iterator operator++(int)
        {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const iterator& other) const
        {
            if (done_ && other.done_) return true;
            return done_ == other.done_ && pos_ == other.pos_;
        }
        bool operator!=(const iterator& other) const { return !(*this == other); }

    private:
        void advance()
        {
            while (pos_ <= s_.size()) {
                size_t next_delim = s_.find(delim_, pos_);
                size_t end = (next_delim == std::string_view::npos) ? s_.size() : next_delim;

                std::string_view token = s_.substr(pos_, end - pos_);

                pos_ = (next_delim == std::string_view::npos) ? s_.size() + 1 : next_delim + 1;

                if (HasFlag(opts_, StringSplitOptions::Trim)) {
                    auto b = token.find_first_not_of(" \t");
                    if (b == std::string_view::npos)
                        token = {};
                    else {
                        auto e = token.find_last_not_of(" \t");
                        token = token.substr(b, e - b + 1);
                    }
                }

                if (token.empty() && HasFlag(opts_, StringSplitOptions::RemoveEmpty)) {
                    continue;
                }

                current_ = token;
                return;
            }
            done_ = true;
        }

        std::string_view s_;
        char delim_ = 0;
        StringSplitOptions opts_ = StringSplitOptions::None;
        size_t pos_ = 0;
        bool done_ = false;
        std::string_view current_;
    };

    iterator begin() const { return iterator {s_, delim_, opts_, false}; }
    iterator end() const { return iterator {}; }
};
