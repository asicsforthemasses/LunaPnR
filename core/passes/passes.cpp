#include <span>

#include "common/strutils.hpp"
#include "passes.hpp"
#include "floorplanpass.hpp"
#include "readpass.hpp"
#include "writepass.hpp"
#include "infopass.hpp"
#include "scriptpass.hpp"
#include "filesystempass.hpp"

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
    registerPass(new ReadPass());
    registerPass(new WritePass());
    registerPass(new InfoPass());
    registerPass(new ScriptPass());
    registerPass(new LSPass());
    registerPass(new CDPass());
    registerPass(new CWDPass());
}

};
