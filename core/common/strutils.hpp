// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <cstdlib>
#include <vector>
#include <list>
#include <string>
#include <string_view>
#include <limits>
#include <algorithm>

namespace LunaCore
{

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

};
