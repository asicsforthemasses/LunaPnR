// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <QStringList>
#include <iostream>
#include <string_view>
#include <filesystem>
#include "cmdcompleter.hpp"

namespace GUI
{

QString longestCommonPrefix(QStringList &list)
{
    if (list.empty()) return "";

    if (list.size() == 1) return list.front();

    std::sort(list.begin(), list.end());

    auto minLength = std::min(list.front().size(), list.back().size());

    auto first = list.front();
    auto last  = list.back();
    std::size_t idx = 0;
    while( (idx < minLength) && (first.at(idx) == last.at(idx)))
    {
        idx++;
    }

    return first.first(idx);
}

QString LunaCommandCompleter::tryComplete(const QString &str)
{
    QStringList options;

    // for now, we just return the first option
    for(auto const& word : m_words)
    {
        if (word.startsWith(str))
        {
            options.push_back(word);
        }
    }

    auto prefix = longestCommonPrefix(options);

    if (prefix.isEmpty())
    {
        options.clear();
        // try to see if it's a filesystem path?
        try
        {
            auto p = std::filesystem::path(str.toStdString());
            auto lastPart = *std::prev(p.end());
            auto pp = p.parent_path();

            for(auto const& entry : std::filesystem::directory_iterator(pp))
            {
                QString entryPath = QString::fromStdString(entry.path().string());
                std::cout << "entryPath: " << entryPath.toStdString() << "\n";
                if (entryPath.startsWith(str))
                {
                    options.push_back(entryPath);
                }
            }

            prefix = longestCommonPrefix(options);
        }
        catch(std::exception &e)
        {
            // ignore exceptions caused by std::filesystem
            // in the command completer
        }
    }

    return prefix;
}

};