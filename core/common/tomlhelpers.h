// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <type_traits>
#include <toml++/toml.h>

namespace ChipDB::TOML
{

template <typename T>
concept SimpleType = std::is_integral_v<T> || std::is_same_v<T, std::string>
    || std::is_same_v<T, const std::string>
    || std::is_same_v<T, bool>;

void insert(toml::table &tbl, const std::string &name,
    SimpleType auto const& value)
{
    tbl.insert(name, value);
}

template<typename T>
void insert(toml::table &tbl, const std::string &arrayName,
    const std::vector<T> &array)
{
    toml::array arr;
    arr.insert(arr.begin(), array.begin(), array.end());
    tbl.insert(arrayName, arr);
}

bool retrieve(toml::table &tbl, const std::string &name, SimpleType auto &result)
{
    using valueType = std::remove_reference_t<decltype(result)>;
    auto ptr = tbl[name].value<valueType>();

    if (!ptr)
    {
        if constexpr (std::is_same_v<valueType, std::string>)
        {
            result.clear();
        }

        if constexpr (std::is_same_v<valueType, bool>)
        {
            result = false;
        }

        if constexpr (std::is_integral_v<valueType>)
        {
            result = 0;
        }

        return false;
    }

    result = ptr.value();
    return true;
}

template<typename T>
bool retrieve(toml::table &tbl, const std::string &arrayName, std::vector<T> &result)
{
    result.clear();

    auto arrPtr = tbl[arrayName].as_array();
    if (arrPtr != nullptr)
    {
        arrPtr->for_each(
            [&](toml::value<T> &item)
            {
                result.emplace_back(item);
            }
        );

        return true;
    }

    return false;
}

};
