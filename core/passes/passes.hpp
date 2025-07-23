// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <string>
#include <vector>
#include "database/database.h"

namespace LunaCore::Passes
{
    bool run(Database &database, const std::string &cmdstring);
    void registerAllPasses();

    std::vector<std::string> getNamesOfPasses();
};
