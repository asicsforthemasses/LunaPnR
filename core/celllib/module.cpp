#include "module.h"

using namespace ChipDB;

// **********************************************************************
//   Module
// **********************************************************************

bool Module::addInstance(InstanceBase* insPtr)
{
    if (insPtr == nullptr)
    {
        return false;
    }

    if (insPtr->m_name.empty())
    {
        return false;
    }

    if (m_netlist)
    {
        m_netlist->m_instances.add(insPtr->m_name, insPtr);
        return true;
    }

    return false;   // cannot add instances to a black box
}

Net* Module::createNet(const std::string &netName)
{
    if (!m_netlist)
    {
        return nullptr; // cannot add nets to a black box
    }

    // if the net already exists, return that one.
    auto myNet = m_netlist->m_nets.lookup(netName);
    if (myNet != nullptr)
        return myNet;

    myNet = new Net;
    myNet->m_name = netName;
    m_netlist->m_nets.add(netName, myNet);

    return myNet;
}
