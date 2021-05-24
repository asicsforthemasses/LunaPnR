#pragma once

#include <vector>
#include "netlist.h"
#include "celllib/celllib.h"

namespace ChipDB
{

class Design
{
public:

    Module* createModule(const std::string &name);

    Netlist                 m_netlist;
    CellLib                 m_cellLib;
    ModuleLib               m_moduleLib;
};

};