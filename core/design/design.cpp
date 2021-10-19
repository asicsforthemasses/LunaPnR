
#include "design.h"

using namespace ChipDB;

Module* Design::createModule(const std::string &name)
{
    auto newModule = m_moduleLib.createModule(name);
    newModule->m_netlist = std::make_unique<Netlist>();
    return newModule;
}


void Design::clear()
{
    m_netlist.clear();
    m_techLib.clear();
    m_cellLib.clear();
    m_moduleLib.clear();
    m_floorplan.clear();

    m_topModule = nullptr;
}

bool Design::setTopModule(const std::string &moduleName)
{
    m_topModule = m_moduleLib.lookup(moduleName);
    return (m_topModule != nullptr);
}
