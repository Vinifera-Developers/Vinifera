// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cerrno>
#include <cctype>
#include <climits>
#include <cstdlib>
#include <cstring>

namespace StatusEffects {
// Preserve TS decimal/$hex/hexh spellings while reporting invalid conversion.
inline bool Parse_Integer(const char* text, int& result, bool percent = false)
{
    if (!text) return false;
    while (*text == ' ' || *text == '\t') ++text;
    const char* finish = text + std::strlen(text);
    while (finish > text && (finish[-1] == ' ' || finish[-1] == '\t')) --finish;
    if (percent && finish > text && finish[-1] == '%') --finish;
    const bool suffix_hex = finish > text && (finish[-1] == 'h' || finish[-1] == 'H');
    const bool prefix_hex = *text == '$';
    if (prefix_hex) ++text;
    errno = 0;
    char* end = nullptr;
    const long long parsed = std::strtoll(text, &end, prefix_hex || suffix_hex ? 16 : 10);
    // Check digit consumption before advancing past a suffix or whitespace.
    if (end == text || errno || parsed < INT_MIN || parsed > INT_MAX) return false;
    if (suffix_hex && (*end == 'h' || *end == 'H')) ++end;
    if (percent && *end == '%') ++end;
    while (*end == ' ' || *end == '\t') ++end;
    if (*end) return false;
    result = static_cast<int>(parsed);
    return true;
}
inline bool Parse_Boolean(const char* text, bool& result)
{
    if (!text) return false;
    switch (std::toupper(static_cast<unsigned char>(*text))) {
        case 'Y': case 'T': case '1': result = true; return true;
        case 'N': case 'F': case '0': result = false; return true;
        default: return false;
    }
}
}
