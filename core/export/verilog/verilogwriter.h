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

    static bool write(std::ostream &os, const ChipDB::Module *mod);

protected:    
    static bool writeModuleDefinition(std::ostream &os, const ChipDB::Module *mod);
    static bool writeModuleInstances(std::ostream &os, const ChipDB::Module *mod);
};

};