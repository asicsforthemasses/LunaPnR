#pragma once
#include<string>
#include "database/database.h"

namespace LunaCore::Passes
{
    bool run(Database &database, const std::string &cmdstring);
    void registerAllPasses();
};
