/*
  LunaPnR Source Code
  
  SPDX-License-Identifier: GPL-3.0-only
  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/


/*

    Experimental verilog writer

*/

#pragma once
#include <iostream>
#include "lunapnr_version.h"
#include "netlist/netlist.h"

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