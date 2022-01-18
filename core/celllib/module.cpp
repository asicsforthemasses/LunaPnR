#include "module.h"

using namespace ChipDB;

// **********************************************************************
//   Module
// **********************************************************************

bool Module::addInstance(std::shared_ptr<InstanceBase> insPtr)
{
    if (!insPtr)
    {
        return false;
    }

    if (insPtr->m_name.empty())
    {
        return false;
    }

    if (m_netlist)
    {
        auto result = m_netlist->m_instances.add(insPtr);
        return result.has_value();
    }

    return false;   // cannot add instances to a black box
}

KeyObjPair<Net> Module::createNet(const std::string &netName)
{
    if (!m_netlist)
    {
        return KeyObjPair<Net>();
    }

    return m_netlist->createNet(netName);

#if 0
    // if the net already exists, return that one.
    auto netKeyObj = m_netlist->m_nets[netName];
    if (netKeyObj.isValid() != nullptr)
    {
        return netKeyObj.;
    }

    myNet = new Net;
    myNet->m_name = netName;
    m_netlist->m_nets.add(netName, myNet);

    return myNet;
#endif
}
