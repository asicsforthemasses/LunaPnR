
#include "design.h"

using namespace ChipDB;

Module* Design::createModule(const std::string &name)
{
    auto newModule = m_moduleLib.createModule(name);
    newModule->m_netlist.reset(new Netlist());
    return newModule;
}


void Design::clear()
{
    m_netlist.clear();
    m_techLib.clear();
    m_cellLib.clear();
    m_moduleLib.clear();
}
