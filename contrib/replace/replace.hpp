// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only
//

#pragma once
#include <string>
#include <string_view>
#include <algorithm>

std::string replace(const std::string &text, const std::string &findText, const std::string &replaceText)
{
    if (findText.size() > text.size()) return text;
    if (findText.empty()) return text;

    std::string result;
    result.reserve(text.size());

    std::size_t idx = 0;
    std::size_t searchEndIdx = text.size() - findText.size() + 1;
    while(idx < searchEndIdx)
    {
        std::size_t searchIdx = 0;
        while(searchIdx < findText.size())
        {
            if (text.at(idx+searchIdx) != findText.at(searchIdx))
                break;  // exit, no (complete) match

            searchIdx++;
        }

        if (searchIdx == findText.size())
        {
            // found a complete match
            result.append(replaceText);
            idx += findText.size();
        }
        else
        {
            // found a partial or no match
            searchIdx++;            
            for(std::size_t cidx=0; cidx<searchIdx; cidx++)
            {
                result += text.at(idx++);
            }
        }
    }

    // append the last bit of the string, if needed.
    if (idx != text.size())
    {
        result.append(std::string_view(text.begin()+idx, text.end()));
    }

    return result;
}
