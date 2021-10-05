
#include "design.h"

using namespace ChipDB;

Module* Design::createModule(const std::string &name)
{
    auto newModule = m_moduleLib.createModule(name);
    return newModule;
}


void Design::clear()
{
    m_netlist.m_nets.clear();
    m_netlist.m_instances.clear();
    m_techLib.m_layers.clear();
    m_techLib.m_sites.clear();
    m_techLib.m_manufacturingGrid = 0;
    m_cellLib.m_cells.clear();    
}