#pragma once

#include <vector>
#include "floorplan/floorplan.h"
#include "netlist/netlist.h"
#include "techlib/techlib.h"
#include "celllib/celllib.h"

namespace ChipDB
{

class Design
{
public:
    Design() : m_uniqueIDCounter(0) {}

    /** clear netlist, celllib, modules and technology information */
    void clear();

    Module* createModule(const std::string &name);

    Netlist                 m_netlist;
    CellLib                 m_cellLib;
    ModuleLib               m_moduleLib;
    TechLib                 m_techLib;
    Floorplan               m_floorplan;
    
    auto createUniqueID()
    {
        return m_uniqueIDCounter++;
    }

protected:
    uint32_t m_uniqueIDCounter;
};

};
