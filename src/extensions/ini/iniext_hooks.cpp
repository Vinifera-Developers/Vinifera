/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          INIEXT_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for the extended INIClass.
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
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "ini.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "b64pipe.h"
#include "buffpipe.h"
#include "cstraw.h"
#include "filestraw.h"
#include <unordered_map>

#include "hooker.h"
#include "miscutil.h"
#include "strtrim.h"
#include "syringe.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class INIClassExt : public INIClass
{
public:
    int _Load(Straw& ffile, bool);
    int _Get_String(char const* section, char const* entry, char const* defvalue, char* buffer, int size) const;

    int _Get_Int(char const* section, char const* entry, int defvalue) const;
    bool _Get_Bool(char const* section, char const* entry, bool defvalue) const;
    double _Get_Float(char const* section, char const* entry, double defvalue) const;
    TPoint2D<int> _Get_Point(char const* section, char const* entry, TPoint2D<int> const& defvalue) const;
    TPoint3D<int> _Get_Point(char const* section, char const* entry, TPoint3D<int> const& defvalue) const;
    TPoint3D<float> _Get_Point(char const* section, char const* entry, TPoint3D<float> const& defvalue) const;
    int _Get_UUBlock(char const* section, void* block, int len) const;
    int _Get_TextBlock(char const* section, char* buffer, int len) const;
    CLSID _Get_UUID(char const* section, char const* entry, CLSID defvalue) const;
    Rect _Get_Rect(char const* section, char const* entry, Rect const& defvalue) const;

    void Inherit_File(INIClass const& ini);
    void Include_File(INIClass const& ini);
};


/**
 *  Reads a line from the INI file.
 *
 *  @author: ZivDero
 */
bool Read_Line(Straw& file, std::string& line)
{
    line.clear();

    while (true) {
        char c;
        if (file.Get(&c, sizeof(c)) != sizeof(c)) { // EOF
            return !line.empty();
        }

        if (c == '\n') {
            return true;
        }

        if (c != '\r') {
            line.push_back(c);
        }
    }
}


/**
 *  Extracts a section name from a line.
 *
 *  @author: ZivDero
 */
std::string Extract_Section_Name(std::string_view line)
{
    auto l = line.find('[');
    if (l == std::string_view::npos) return {};

    auto r = line.find(']', l + 1);
    if (r == std::string_view::npos || r <= l + 1) return {};

    std::string name(line.substr(l + 1, r - l - 1));
    strtrim(name.data());

    return name;
}


/**
 *  Inherits the INI data from another INI file.
 *
 *  @author: ZivDero
 */
void INIClassExt::Inherit_File(INIClass const& ini)
{
    for (const INISection* section = ini.SectionList.First(); section; section = section->Next_Valid() ? section->Next() : nullptr) {
        if (strcmp(section->Section, "$Inherit") == 0 || strcmp(section->Section, "$Include") == 0) {
            continue;
        }
        for (const INIEntry* entry = section->EntryList.First(); entry; entry = entry->Next_Valid() ? entry->Next() : nullptr) {
            if (Is_Present(section->Section, entry->Entry)) {
                continue;
            }
            Put_String(section->Section, entry->Entry, entry->Value);
        }
    }
}


/**
 *  Includes the INI data from another INI file.
 *
 *  @author: ZivDero
 */
void INIClassExt::Include_File(INIClass const& ini)
{
    for (const INISection* section = ini.SectionList.First(); section; section = section->Next_Valid() ? section->Next() : nullptr) {
        if (strcmp(section->Section, "$Inherit") == 0 || strcmp(section->Section, "$Include") == 0) {
            continue;
        }
        for (const INIEntry* entry = section->EntryList.First(); entry; entry = entry->Next_Valid() ? entry->Next() : nullptr) {
            Put_String(section->Section, entry->Entry, entry->Value);
        }
    }
}


/**
 *  Loads the INI data from the data stream (straw).
 *
 *  @author: ZivDero, tomsons26
 */
int INIClassExt::_Load(Straw& ffile, bool)
{
    std::string line;
    line.reserve(1024);

    CacheStraw file;
    file.Get_From(ffile);

    std::string section;

    while (Read_Line(file, line)) {

        /**
         *  Determine if this line is a comment or blank line. Throw it out if it is.
         */
        Strip_Comments(line.data());
        if (line.empty() || line[0] == ';' || line[0] == '=') {
            continue;
        }

        /**
         *  Process a section.
         */
        if (Line_Contains_Section(line.data())) {
            section = Extract_Section_Name(line);
            strtrim(section.data());
            continue;
        }

        /**
         *  We haven't found the first section yet, discard the line.
         */
        if (section.empty()) {
            continue;
        }

        /**
         *  The line isn't an obvious comment. Make sure that there is the "=" character
         *  at an appropriate spot.
         */
        char* buffer = line.data();
        char* divider = strchr(buffer, '=');
        if (!divider) continue;

        /**
         *  Split the line into entry and value sections. Be sure to catch the
         *  "=foobar" and "foobar=" cases. These lines are ignored.
         */
        *divider++ = '\0';
        strtrim(buffer);
        if (!strlen(buffer)) continue;

        strtrim(divider);
        if (!strlen(divider)) continue;

        if (Put_String(section.c_str(), buffer, divider) == false) {
            return false;
        }
    }

    constexpr const char* inherit_section = "$Inherit";
    constexpr const char* include_section = "$Include";

    std::vector<std::string> inherits;
    if (Section_Present(inherit_section)) {
        int count = Entry_Count(inherit_section);
        for (int i = 0; i < count; i++) {
            std::string entry = Get_String(inherit_section, Get_Entry(inherit_section, i), {});
            if (std::ranges::find(inherits, entry) == inherits.end()) {
                inherits.emplace_back(entry);
            }
        }
    }

    std::vector<std::string> includes;
    if (Section_Present(include_section)) {
        int count = Entry_Count(include_section);
        for (int i = 0; i < count; i++) {
            std::string entry = Get_String(include_section, Get_Entry(include_section, i), {});
            if (std::ranges::find(includes, entry) == includes.end()) {
                includes.emplace_back(entry);
            }
        }
    }

    for (auto& filename : inherits) {
        CCFileClass ifile(filename.c_str());
        if (ifile.Is_Available()) {
            INIClass iini;
            iini.Load(ifile);
            Inherit_File(iini);
        } else {
            DEBUG_FATAL("INIClassExt::_Load - Inherit file not found: %s\n", filename.c_str());
            char error[512];
            std::snprintf(error, sizeof(error), "INIClassExt::_Load - Inherit file not found: %s\nThe game will now exit.", filename.c_str());
            MessageBox(MainWindow, error, "Vinifera", MB_OK | MB_ICONERROR);
            Emergency_Exit(EXIT_FAILURE);
        }
    }

    for (auto& filename : includes) {
        CCFileClass ifile(filename.c_str());
        if (ifile.Is_Available()) {
            INIClass iini;
            iini.Load(ifile);
            Include_File(iini);
        } else {
            DEBUG_FATAL("INIClassExt::_Load - Include file not found: %s\n", filename.c_str());
            char error[512];
            std::snprintf(error, sizeof(error), "INIClassExt::_Load - Include file not found: %s\nThe game will now exit.", filename.c_str());
            MessageBox(MainWindow, error, "Vinifera", MB_OK | MB_ICONERROR);
            Emergency_Exit(EXIT_FAILURE);
        }
    }

    return true;
}


/**
 *  Cached inherited sections.
 */
static std::unordered_map<void*, std::vector<std::string>> InheritedSections;


/**
 *  Get_String replacement that checks inherited sections if the entry is not found in the main section.
 *
 *  @author: ZivDero, tomsons26
 */
int INIClassExt::_Get_String(char const* section, char const* entry, char const* defvalue, char* buffer, int size) const
{
    /**
     *  Verify that the parameters are nominally legal.
     */
    if (buffer == nullptr || size < 2 || section == nullptr || entry == nullptr) return 0;

    if (std::string_view(section) == "E1") {
        DEBUG_INFO("E1: Fetching [%s]->%s\n", section, entry);
    }

    /**
     *  Fetch the entry string if it is present.
     */
    bool has_value = false;
    INIEntry* entryptr = Find_Entry(section, entry);
    if (entryptr != nullptr && entryptr->Value != nullptr) {
        defvalue = entryptr->Value;
        has_value = true;
    }

    /**
     *  Attempt to find the entry string among inherited sections. If not,
     *  then the normal default value will be used as the entry value.
     */
    if (!has_value) {
        INISection* sectionptr = Find_Section(section);
        if (InheritedSections.contains(sectionptr)) {
            for (const std::string& inherited_section : InheritedSections[sectionptr]) {
                int count = Get_String(inherited_section.c_str(), entry, "", buffer, size);
                if (count > 0) {
                    //DEBUG_INFO("Fetched [%s]->%s from %s\n", section, entry, inherited_section.c_str());
                    return count;
                }
            }
        }
    }

    /**
     *  Fill in the buffer with the entry value and return with the length of the string.
     */
    if (defvalue == nullptr) {
        buffer[0] = '\0';
        return 0;
    } else {
        strncpy(buffer, defvalue, size);
        buffer[size - 1] = '\0';
        strtrim(buffer);
        return strlen(buffer);
    }
}


/**
 *  Caches the inherited sections upon putting the $Inherits entry.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004DDD3A, _INIClass_Put_String_Cache_Inherits, 5)
{
    GET_STACK(INIClass::INISection*, secptr, 0x14);
    GET_STACK(char const*, entry, 0x50);
    GET_STACK(char const*, string, 0x54);

    if (strcmp(entry, "$Inherits") == 0) {
        InheritedSections[secptr].clear();
        for (std::string_view part : SplitView(string, ',', StringSplitOptions::RemoveEmpty | StringSplitOptions::Trim)) {
            InheritedSections[secptr].emplace_back(part);
        }
    }

    return 0;
}

DEFINE_HOOK(0x004DED22, _INIClass_INISection_DTOR_Clear_Inherits, 5)
{
    GET(INIClass::INISection*, secptr, ECX);

    InheritedSections.erase(secptr);

    return 0;
}


/**
 *  Various INI getter replacements.
 *
 *  @author: tomsons26, ZivDero
 */
int INIClassExt::_Get_Int(char const* section, char const* entry, int defvalue) const
{
    if (section == nullptr || entry == nullptr) return defvalue;

    std::string value = Get_String(section, entry, {});
    if (!value.empty()) {
        if (value[0] == '$') {
            sscanf(value.c_str(), "$%x", &defvalue);
        } else {
            if (tolower(value.back()) == 'h') {
                sscanf(value.c_str(), "%xh", &defvalue);
            } else {
                defvalue = atoi(value.c_str());
            }
        }
    }
    return defvalue;
}

bool INIClassExt::_Get_Bool(char const* section, char const* entry, bool defvalue) const
{
    if (section == nullptr || entry == nullptr) return defvalue;

    std::string value = Get_String(section, entry, {});
    if (!value.empty()) {
        switch (toupper(value[0])) {
        case 'Y':
        case 'T':
        case '1':
            return true;

        case 'N':
        case 'F':
        case '0':
            return false;
        }
    }
    return defvalue;
}

double INIClassExt::_Get_Float(char const* section, char const* entry, double defvalue) const
{
    if (section == nullptr || entry == nullptr) return defvalue;

    std::string value = Get_String(section, entry, {});
    if (!value.empty()) {
        float val;
        sscanf(value.c_str(), "%f", &val);
        defvalue = val;
        if (value.find('%') != std::string::npos) {
            defvalue /= 100.0;
        }
    }
    return defvalue;
}

TPoint2D<int> INIClassExt::_Get_Point(char const* section, char const* entry, TPoint2D<int> const& defvalue) const
{
    char buffer[64];
    if (Get_String(section, entry, "", buffer, sizeof(buffer))) {
        int x, y;
        std::sscanf(buffer, "%d,%d", &x, &y);
        return {x, y};
    }
    return defvalue;
}

TPoint3D<int> INIClassExt::_Get_Point(char const* section, char const* entry, TPoint3D<int> const& defvalue) const
{
    char buffer[64];
    if (Get_String(section, entry, "", buffer, sizeof(buffer))) {
        int x, y, z;
        std::sscanf(buffer, "%d,%d,%d", &x, &y, &z);
        return {x, y, z};
    }
    return defvalue;
}

TPoint3D<float> INIClassExt::_Get_Point(char const* section, char const* entry, TPoint3D<float> const& defvalue) const
{
    char buffer[64];
    if (Get_String(section, entry, "", buffer, sizeof(buffer))) {
        float x, y, z;
        std::sscanf(buffer, "%f,%f,%f", &x, &y, &z);
        return {x, y, z};
    }
    return defvalue;
}

int INIClassExt::_Get_UUBlock(char const* section, void* block, int len) const
{
    if (section == nullptr) return 0;

    Base64Pipe b64pipe(Base64Pipe::DECODE);
    BufferPipe bpipe(block, len);

    b64pipe.Put_To(&bpipe);

    int total = 0;
    int counter = Entry_Count(section);
    for (int index = 0; index < counter; index++) {
        char buffer[128];

        int length = Get_String(section, Get_Entry(section, index), "=", buffer, sizeof(buffer));
        int outcount = b64pipe.Put(buffer, length);
        total += outcount;
    }
    total += b64pipe.End();
    return total;
}

int INIClassExt::_Get_TextBlock(char const* section, char* buffer, int len) const
{
    if (len <= 0) return 0;

    buffer[0] = '\0';
    if (len <= 1) return 0;

    int elen = Entry_Count(section);
    int total = 0;
    for (int index = 0; index < elen; index++) {
        if (index > 0) {
            *buffer++ = ' ';
            len--;
            total++;
        }

        Get_String(section, Get_Entry(section, index), "", buffer, len);

        int partial = std::strlen(buffer);
        total += partial;
        buffer += partial;
        len -= partial;
        if (len <= 1) break;
    }
    return total;
}

CLSID INIClassExt::_Get_UUID(char const* section, char const* entry, CLSID defvalue) const
{
    char buffer[128];

    if (Get_String(section, entry, "", buffer, sizeof(buffer))) {
        wchar_t wBuffer[128];
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buffer, -1, wBuffer, std::size(wBuffer));
        CLSID uuid;
        if (SUCCEEDED(CLSIDFromString(wBuffer, &uuid))) {
            return uuid;
        }
    }
    return defvalue;
}

Rect INIClassExt::_Get_Rect(char const* section, char const* entry, Rect const& defvalue) const
{
    char buffer[64];

    if (Get_String(section, entry, "", buffer, sizeof(buffer))) {
        Rect retval = defvalue;
        sscanf(buffer, "%d,%d,%d,%d", &retval.X, &retval.Y, &retval.Width, &retval.Height);
        return retval;
    }
    return defvalue;
}


/**
 *  Main function for patching the hooks.
 */
void INIClassExtension_Hooks()
{
    Patch_Jump(0x004DB7D0, &INIClassExt::_Load);
    Patch_Jump(0x004DDF60, &INIClassExt::_Get_String);

    Patch_Jump(0x004DD140, &INIClassExt::_Get_Int);
    Patch_Jump(0x004DE140, &INIClassExt::_Get_Bool);
    Patch_Jump(0x004DD9F0, &INIClassExt::_Get_Float);
    Patch_Jump(0x004DE340, static_cast<TPoint2D<int> (INIClassExt::*)(char const*, char const*, TPoint2D<int> const&) const>(&INIClassExt::_Get_Point));
    Patch_Jump(0x004DE520, static_cast<TPoint3D<int> (INIClassExt::*)(char const*, char const*, TPoint3D<int> const&) const>(&INIClassExt::_Get_Point));
    Patch_Jump(0x004DE730, static_cast<TPoint3D<float> (INIClassExt::*)(char const*, char const*, TPoint3D<float> const&) const>(&INIClassExt::_Get_Point));
    Patch_Jump(0x004DCAD0, &INIClassExt::_Get_UUBlock);
    Patch_Jump(0x004DCDE0, &INIClassExt::_Get_TextBlock);
    Patch_Jump(0x004DD320, &INIClassExt::_Get_UUID);
    Patch_Jump(0x004DD610, &INIClassExt::_Get_Rect);
}
