#include "module.h"

using namespace ChipDB;

// **********************************************************************
//   Module
// **********************************************************************

KeyObjPair<InstanceBase> Module::addInstance(std::shared_ptr<InstanceBase> insPtr)
{
    if (!insPtr)
    {
        return KeyObjPair<InstanceBase>();
    }

    if (insPtr->name().empty())
    {
        return KeyObjPair<InstanceBase>();
    }

    if (m_netlist)
    {
        auto result = m_netlist->m_instances.add(insPtr);
        return result.value();
    }

    return KeyObjPair<InstanceBase>();   // cannot add instances to a black box
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
