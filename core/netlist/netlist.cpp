// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <algorithm>
#include "netlist.h"
#include "instance.h"
#include "net.h"

using namespace ChipDB;


void Netlist::clear()
{
    m_instances.clear();
    m_nets.clear();
}

std::size_t Netlist::createUniqueID()
{
    m_uniqueCounter++;
    return m_uniqueCounter;
}

KeyObjPair<Net> Netlist::createNet(const std::string &netName)
{
    // check if the net exists
    auto netObjPair = m_nets[netName];

    if (netObjPair.isValid())
    {
        // net exists, return this instead of createing a new one
        return netObjPair;
    }
    
    auto result = m_nets.add(std::make_shared<Net>(netName));
    if (result)
    {
        return result.value();
    }
    
    return KeyObjPair<Net>{};
}

std::shared_ptr<Instance> Netlist::lookupInstance(InstanceObjectKey key)
{
    return m_instances[key];
}

KeyObjPair<Instance> Netlist::lookupInstance(const std::string &name)
{
    return m_instances[name];
}

std::shared_ptr<Net> Netlist::lookupNet(InstanceObjectKey key)
{
    return m_nets[key];
}

KeyObjPair<Net> Netlist::lookupNet(const std::string &name)
{
    return m_nets[name];
}

bool Netlist::connect(InstanceObjectKey insKey, PinObjectKey pinKey, NetObjectKey netKey)
{
    auto ins = m_instances[insKey];
    auto net = m_nets[netKey];

    if (net && ins)
    {
        if (!ins->setPinNet(pinKey, netKey))
        {
            return false;
        }

        if (net->hasConnection(insKey, pinKey))
        {
            return false;
        }

        net->addConnection(insKey, pinKey);
        return true;
    }

    return false;
}

bool Netlist::connect(const std::string &insName, const std::string &pinName, const std::string &netName)
{
    auto netKeyObjPair = m_nets[netName];
    if (!netKeyObjPair.isValid())
    {
        return false;
    }

    auto insKeyObjPair = m_instances[insName];
    if (!insKeyObjPair.isValid())
    {
        return false;
    }

    // connect (ins,pin) to net
    auto const pin = insKeyObjPair->getPin(pinName);
    if (pin.m_pinKey != ObjectNotFound)
    {
        insKeyObjPair->setPinNet(pin.m_pinKey, netKeyObjPair.key());
    }

    // add (ins, pin) to net
    if (!netKeyObjPair->hasConnection(insKeyObjPair.key(), pin.m_pinKey))
    {
        netKeyObjPair->addConnection(insKeyObjPair.key(), pin.m_pinKey);
        return true;
    }

    return false;
}
