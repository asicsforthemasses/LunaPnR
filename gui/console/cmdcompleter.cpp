#include <QStringList>
#include "cmdcompleter.hpp"

namespace GUI
{

std::list<QString> LunaCommandCompleter::tryComplete(const QString &str)
{
    std::list<QString> results;

    static const std::array<QString, 5> words =
    {
        "floorplan","info","place","read","write"
    };

    // for now, we just return the first option
    for(auto const& word : words)
    {
        if (word.startsWith(str))
        {
            results.push_back(word);
            return results;
        }
    }

    return results;
}

};