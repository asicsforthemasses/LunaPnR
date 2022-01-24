#include "instance.h"

using namespace ChipDB;


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
    pin.m_netKey  = m_pinToNet.at(key);
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
        pin.m_netKey  = m_pinToNet.at(pinKeyObjPair.key());
    }

    return pin;
}

bool Instance::setPinNet(PinObjectKey pinKey, NetObjectKey netKey)
{
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
