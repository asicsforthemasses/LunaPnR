// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <cstdlib>
#include <vector>
#include <list>
#include <sstream>
#include <string>
#include <string_view>
#include <codecvt>
#include <locale>
#include <limits>
#include <algorithm>
#include <regex>

/** \addtogroup string_utils
  String and character encoding utilities
  @{
*/

typedef std::basic_stringstream<char32_t> u32sstream;   ///< a stringstream based on char32_t

/**
 @}
*/

namespace LunaCore
{

/** \addtogroup string_utils
*  String and character encoding utilities
* @{
*/

/** Colors for ANSI terminals */
namespace ANSI
{
    constexpr const char black[]  = "\033[1;30m";
    constexpr const char red[]    = "\033[1;31m";
    constexpr const char green[]  = "\033[1;32m";
    constexpr const char yellow[] = "\033[1;33m";
    constexpr const char blue[]   = "\033[1;34m";
    constexpr const char purple[] = "\033[1;35m";
    constexpr const char cyan[]   = "\033[1;36m";
    constexpr const char white[]  = "\033[1;37m";
    constexpr const char normal[] = "\033[0m";
};

/** convert a 32-bit char string to UTF-8 */
inline std::string toUTF8(const std::u32string &s)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.to_bytes(s);
}

/** convert a UTF-8 char string to UTF-32 */
inline std::u32string toUTF32(const std::string &s)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.from_bytes(s);
}

/** convert a string to upper case */
inline std::string toupper(const std::string &text)
{
    std::string result = text;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c){ return std::toupper(c); }
    );

    return result;
}

/** convert a string to lower case */
inline std::string tolower(const std::string &text)
{
    std::string result = text;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c){ return std::tolower(c); }
    );

    return result;
}

/** split a string based on a single delimiter */
inline std::vector<std::string> split(const std::string &text, const char delimiter)
{
    std::vector<std::string> chunks;

    std::size_t idx = 0;
    std::size_t chunkStart = 0;
    bool inChunk = false;
    while(idx < text.size())
    {
        if (text.at(idx) == delimiter)
        {
            if (inChunk)
            {
                auto chunkString = std::string(text.begin()+chunkStart, text.begin()+idx);
                chunks.emplace_back(chunkString);
            }
            inChunk = false;
        }
        else
        {
            if (inChunk == false)
            {
                chunkStart = idx;
            }
            inChunk = true;
        }
        idx++;
    }

    if ((chunkStart < text.size()) && inChunk)
    {
        auto chunkString = std::string(text.begin()+chunkStart, text.begin()+idx);
        chunks.emplace_back(chunkString);
    }

    return chunks;
}

/** split a string based on a multiple delimiters */
inline std::vector<std::string> split(const std::string &text, const std::string &delimiters)
{
    std::vector<std::string> chunks;

    std::size_t idx = 0;
    std::size_t chunkStart = 0;
    bool inChunk = false;
    while(idx < text.size())
    {
        auto iter = std::find(delimiters.begin(), delimiters.end(), text.at(idx));
        if (iter != delimiters.end())
        {
            if (inChunk)
            {
                auto chunkString = std::string(text.begin()+chunkStart, text.begin()+idx);
                chunks.emplace_back(chunkString);
            }
            inChunk = false;
        }
        else
        {
            if (inChunk == false)
            {
                chunkStart = idx;
            }
            inChunk = true;
        }
        idx++;
    }

    if ((chunkStart < text.size()) && inChunk)
    {
        auto chunkString = std::string(text.begin()+chunkStart, text.begin()+idx);
        chunks.emplace_back(chunkString);
    }

    return chunks;
}

/** replace 'from' with 'to' in string 'str' */
inline std::string findAndReplace(const std::string &str, const std::string &findMe, const std::string &replaceWithMe)
{
    std::ostringstream result;
    std::size_t pos = 0;
    std::size_t prevPos;

    while(true)
    {
        prevPos = pos;
        pos = str.find(findMe, pos);
        if (pos == std::string::npos)
            break;
        result << str.substr(prevPos, pos - prevPos);
        result << replaceWithMe;
        pos += findMe.size();
    }

    result << str.substr(prevPos);
    return result.str();
}

/** replace keys in curly braces with those found in a lookup-by-string container.
    keys that cannot be found in the container are not replaced.
*/
inline std::string replaceKeysInBraces(const auto &container, const std::string &str)
{
    std::regex curlymatcher(R"(\{(.*?)\})");    // matches everything within {}

    auto matches_begin = std::sregex_iterator(str.begin(),
        str.end(), curlymatcher);
    auto matches_end   = std::sregex_iterator();

    auto result = str;
    for(auto iter = matches_begin; iter != matches_end; iter++)
    {
        auto match = *iter;
        if (match.size() < 2) continue;

        auto const& completeMatch = match.str(0);
        auto const& keyName       = match.str(1);

        // get the key from the container
        // if the key doesn't exist, don't replace it

        if (container.contains(keyName))
        {
            result = findAndReplace(result, completeMatch, container.at(keyName));
        }
    }
    return result;
}

/** @} */

};
