// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

/*

    Experimental verilog writer

*/

#pragma once
#include <iostream>
#include "database/database.h"

namespace LunaCore::Verilog
{

class Writer
{
public:

    static bool write(std::ostream &os, const std::shared_ptr<ChipDB::Module> mod);

protected:
    static bool writeModuleDefinition(std::ostream &os, const std::shared_ptr<ChipDB::Module> mod);
    static bool writeModuleInstances(std::ostream &os, const std::shared_ptr<ChipDB::Module> mod);
};

};