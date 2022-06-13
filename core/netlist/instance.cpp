// SPDX-FileCopyrightText: 2021-2022 Niels Moseley, <n.a.moseley@moseleyinstruments.com>, et al.
// SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "instance.h"

using namespace ChipDB;

std::string ChipDB::toString(const InstanceType &t)
{
    constexpr const std::array<const char*, 5> names = 
    {{
        "UNKNOWN", "ABSTRACT", "CELL", "MODULE", "PIN"
    }};

    auto index = static_cast<size_t>(t);
    if (index < names.size())
    {
        return std::string(names[index]);
    }
    else
    {
        return std::string("UNKNOWN");
    }
};

// **********************************************************************
//  Instance implementation
// **********************************************************************

std::string Instance::getArchetypeName() const
{
    if (m_cell != nullptr)
        return m_cell->name();
    else
        return "UNKNOWN";
}

double Instance::getArea() const noexcept
{
    if (m_cell == nullptr)
        return 0.0;
    else
        return m_cell->m_area;
}

InstanceBase::Pin Instance::getPin(PinObjectKey key) const
{
    InstanceBase::Pin pin;
    if (!m_cell)
    {
        return InstanceBase::Pin();
    }

    pin.m_pinInfo = m_cell->m_pins[key];

    if (m_pinToNet.size() > key)
    {
        pin.m_netKey = m_pinToNet.at(key);
    }
    else
    {
        pin.m_netKey = ChipDB::ObjectNotFound;
    }

    pin.m_pinKey  = key;

    return pin;
}

InstanceBase::Pin Instance::getPin(const std::string &pinName) const
{
    InstanceBase::Pin pin;
    if (!m_cell)
    {
        return InstanceBase::Pin();
    }

    auto pinKeyObjPair = m_cell->m_pins[pinName];
    if (pinKeyObjPair.isValid())
    {
        pin.m_pinInfo = pinKeyObjPair.ptr();
        pin.m_pinKey  = pinKeyObjPair.key();
        
        auto key = pinKeyObjPair.key();
        if (key < m_pinToNet.size())
        {
            pin.m_netKey = m_pinToNet.at(key);
        }
        else
        {
            pin.m_netKey = ChipDB::ObjectNotFound;
        }
    }

    return pin;
}

bool Instance::setPinNet(PinObjectKey pinKey, NetObjectKey netKey)
{
    if (!m_cell)
    {
        return false;
    }

    if (m_pinToNet.size() < m_cell->getNumberOfPins())
    {
        m_pinToNet.resize(m_cell->getNumberOfPins(), ChipDB::ObjectNotFound);
    }

    if (pinKey < m_pinToNet.size())
    {
        m_pinToNet.at(pinKey) = netKey;
        return true;
    }

    return false;
}

size_t Instance::getNumberOfPins() const
{
    // check if the cell is present
    if (!m_cell)
    {
        // nope..
        return 0;
    }   

    return m_cell->m_pins.size();
}

// **********************************************************************
//  PinInstance implementation
// **********************************************************************

std::string PinInstance::getArchetypeName() const
{
    return "__PIN";
}

InstanceBase::Pin PinInstance::getPin(PinObjectKey key) const
{
    InstanceBase::Pin pin;

    if (key == 0)
    {
        pin.m_pinInfo = std::make_shared<PinInfo>(m_pinInfo);
        pin.m_pinKey  = key;
        pin.m_netKey  = m_connectedNet;
    }

    return pin;
}

InstanceBase::Pin PinInstance::getPin(const std::string &pinName) const
{
    InstanceBase::Pin pin;

    if (m_pinInfo.m_name == pinName)
    {
        pin.m_pinInfo = std::make_shared<PinInfo>(m_pinInfo);
        pin.m_netKey  = m_connectedNet;
    }

    return pin;
}

bool PinInstance::setPinNet(PinObjectKey pinKey, NetObjectKey netKey)
{
    if (pinKey != 0)
    {
        return false;   // there is only one pin
    }

    m_connectedNet = netKey;
    return true;
}

size_t PinInstance::getNumberOfPins() const
{
    return 1;
}
