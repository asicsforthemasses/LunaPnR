#include <QStringList>
#include <string_view>
#include "cmdcompleter.hpp"

namespace GUI
{

std::list<QString> LunaCommandCompleter::tryComplete(const QString &str)
{
    std::list<QString> results;

    // for now, we just return the first option
    for(auto const& word : m_words)
    {
        if (word.startsWith(str))
        {
            results.push_back(word);
            return results;
        }
    }

    // return if there are 0 or 1 solutions
    if (results.size() <= 1)
    {
        return results;
    }

    QString prefix = str;

    // find the longest common prefix
    for(auto const& option : results)
    {

    }

    return results;
}

};