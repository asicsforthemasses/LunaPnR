#include "instance.h"

using namespace ChipDB;

// **********************************************************************
//  Instance implementation
// **********************************************************************

std::string Instance::getArchetypeName() const
{
    if (m_cell != nullptr)
        return m_cell->m_name;
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

const PinInfo* Instance::getPinInfo(ssize_t pinIndex) const
{
    // check if the cell is present
    if (m_cell == nullptr)
    {
        // nope, return an invalid pin
        return nullptr;
    }

    return m_cell->m_pins[pinIndex];
}

const PinInfo* Instance::getPinInfo(const std::string &pinName) const
{
    // check if the cell is present
    if (m_cell == nullptr)
    {
        // nope, return an invalid pin
        return nullptr;
    }

    auto pinIndex = m_cell->m_pins.lookupIndex(pinName);
    if (pinIndex < 0)
    {
        // the pin does not exist!
        return nullptr;
    }

    return m_cell->m_pins[pinIndex];
}

const ssize_t Instance::getPinIndex(const std::string &pinName) const
{
    // check if the cell is present
    if (m_cell == nullptr)
    {
        // nope, return an invalid pin
        return -1;
    }

    return m_cell->lookupPinIndex(pinName);
}

const size_t Instance::getNumberOfPins() const
{
    // check if the cell is present
    if (m_cell == nullptr)
    {
        // nope..
        return 0;
    }   

    return m_cell->m_pins.size();
}

Net* Instance::getConnectedNet(ssize_t pinIndex)
{
    if (pinIndex < 0)
        return nullptr;

    if (pinIndex < m_pinToNet.size())
        return m_pinToNet[pinIndex];

    return nullptr;  // pin not found - index out of bounds
}

bool Instance::connect(ssize_t pinIndex, Net *net)
{   
    if (pinIndex < 0)
        return false;   // invalid pin

    if (pinIndex < m_pinToNet.size())
    {
        m_pinToNet[pinIndex] = net;
        return true;
    }

    return false;   // pin does not exist!
}

bool Instance::connect(const std::string &pinName, Net *net)
{
    auto pinIndex = m_cell->lookupPinIndex(pinName);
    if (pinIndex < 0)
        return false;   // pin not found
        
    return Instance::connect(pinIndex, net);
}

// **********************************************************************
//  PinInstance implementation
// **********************************************************************

std::string PinInstance::getArchetypeName() const
{
    return "__PIN";
}

const PinInfo* PinInstance::getPinInfo(ssize_t pinIndex) const
{
    if (pinIndex == 0)
        return &m_pinInfo;
    else 
        return nullptr;
}

const PinInfo* PinInstance::getPinInfo(const std::string &pinName) const
{
    if (m_name == pinName)
        return &m_pinInfo;
    
    return nullptr;
}

const ssize_t PinInstance::getPinIndex(const std::string &pinName) const
{
    if (m_name == pinName)
        return 0;

    return -1;
}

const size_t PinInstance::getNumberOfPins() const
{
    return 1;
}

Net* PinInstance::getConnectedNet(ssize_t pinIndex)
{
    if (pinIndex == 0)
        return m_connectedNet;

    return nullptr;  // pin not found - index out of bounds
}

bool PinInstance::connect(ssize_t pinIndex, Net *net)
{   
    if (pinIndex != 0)
        return false;   // invalid pin

    m_connectedNet = net;
    return true;
}

bool PinInstance::connect(const std::string &pinName, Net *net)
{
    if (m_name != pinName)
        return false;   // invalid pin
        
    m_connectedNet = net;
    return true;
}
