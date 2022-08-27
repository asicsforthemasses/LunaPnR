// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "module.h"

using namespace ChipDB;

// **********************************************************************
//   Module
// **********************************************************************

Module::Module(const std::string &name) : Cell(name)
{
    m_netlist = std::make_shared<Netlist>();
}

KeyObjPair<Instance> Module::addInstance(std::shared_ptr<Instance> insPtr)
{
    if (!insPtr)
    {
        return KeyObjPair<Instance>();
    }

    if (insPtr->name().empty())
    {
        return KeyObjPair<Instance>();
    }

    if (m_netlist)
    {
        auto result = m_netlist->m_instances.add(insPtr);
        return result.value();
    }

    return KeyObjPair<Instance>();   // cannot add instances to a black box
}

KeyObjPair<Net> Module::createNet(const std::string &netName)
{
    if (!m_netlist)
    {
        return KeyObjPair<Net>();
    }

    return m_netlist->createNet(netName);
}

bool Module::connect(const std::string &insName, const std::string &pinName, const std::string &netName)
{
    return m_netlist->connect(insName, pinName, netName);
}

bool Module::connect(InstanceObjectKey insKey, PinObjectKey pinKey, NetObjectKey netKey)
{
    return m_netlist->connect(insKey, pinKey, netKey);
}
