// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "instance.h"
#include "common/logging.h"

using namespace ChipDB;

std::string ChipDB::toString(const InstanceType &t)
{
    constexpr const std::array<const char *, 5> names =
        {{"UNKNOWN", "ABSTRACT", "CELL", "MODULE", "PIN"}};

    auto index = static_cast<size_t>(t);
    if (index < names.size())
    {
        return names[index];
    }
    else
    {
        return "UNKNOWN";
    }
};

// **********************************************************************
//  Instance implementation
// **********************************************************************

std::string Instance::getArchetypeName() const noexcept
{
    switch (m_insType)
    {
    case InstanceType::PIN:
        return "__PIN";
    case InstanceType::CELL:
    case InstanceType::ABSTRACT:
        if (m_cell != nullptr)
            return m_cell->name();
    default:    // intentional fall-though
        return "UNKNOWN";
    }
}

double Instance::getArea() const noexcept
{
    if (m_cell == nullptr)
        return 0.0;
    else
        return m_cell->m_area;
}

Instance::Pin Instance::getPin(PinObjectKey key) const
{
    Instance::Pin pin;
    if (!m_cell)
    {
        return Instance::Pin{};
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

    pin.m_pinKey = key;

    return pin;
}

Instance::Pin Instance::getPin(const std::string &pinName) const
{
    Instance::Pin pin;
    if (!m_cell)
    {
        return Instance::Pin{};
    }

    auto pinKeyObjPair = m_cell->m_pins[pinName];
    if (pinKeyObjPair.isValid())
    {
        pin.m_pinInfo = pinKeyObjPair.ptr();
        pin.m_pinKey = pinKeyObjPair.key();

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
        Logging::logWarning("Instance::setPinNet failed for pin id %d and net id %d\n", pinKey, netKey);
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

bool Instance::disconnectPin(PinObjectKey pinKey)
{
    return setPinNet(pinKey, ChipDB::ObjectNotFound);
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

/** returns true if this is a CORE filler cell */
bool Instance::isCoreFiller() const noexcept
{
    if (!m_cell) return false;

    return ((m_cell->m_subclass == ChipDB::CellSubclass::SPACER) && (m_cell->m_class == ChipDB::CellClass::CORE));
}

/** returns true if this is a CORE decap cell */
bool Instance::isCoreDecap() const noexcept
{
    if (!m_cell) return false;

    return ((m_cell->m_subclass == ChipDB::CellSubclass::DECAP) && (m_cell->m_class == ChipDB::CellClass::CORE));
}

#if 0
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
        pin.m_pinKey = key;
        pin.m_netKey = m_connectedNet;
    }

    return pin;
}

InstanceBase::Pin PinInstance::getPin(const std::string &pinName) const
{
    InstanceBase::Pin pin;

    if (m_pinInfo.m_name == pinName)
    {
        pin.m_pinInfo = std::make_shared<PinInfo>(m_pinInfo);
        pin.m_netKey = m_connectedNet;
    }

    return pin;
}

bool PinInstance::setPinNet(PinObjectKey pinKey, NetObjectKey netKey)
{
    if (pinKey != 0)
    {
        return false; // there is only one pin
    }

    m_connectedNet = netKey;
    return true;
}

size_t PinInstance::getNumberOfPins() const
{
    return 1;
}
#endif
