
#include "design.h"

using namespace ChipDB;

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
    auto moduleKeyObjPair = m_moduleLib.lookupModule(moduleName);
    if (moduleKeyObjPair.isValid())
    {
        m_topModule = moduleKeyObjPair.ptr();
        return true;
    }
    return false;
}
