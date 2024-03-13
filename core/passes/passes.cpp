#include <span>

#include "passes.hpp"
#include "floorplanpass.hpp"
#include "common/strutils.hpp"

namespace LunaCore::Passes
{

bool run(Database &database, const std::string &cmdstring)
{
    auto chunks = LunaCore::split(cmdstring, ' ');
    if (!chunks.empty())
    {
        return runPass(database,chunks.front(), std::span(chunks.begin()+1, chunks.end()));
    }

    return false;
}

void registerAllPasses()
{
    registerPass(new Floorplan());
}

};
