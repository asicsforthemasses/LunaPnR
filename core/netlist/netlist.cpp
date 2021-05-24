#include <algorithm>
#include "netlist.h"

using namespace ChipDB;

void Net::addConnection(AbstractInstance *instance, size_t pinIndex)
{
    auto iter = std::find_if(m_connections.begin(), m_connections.end(),
        [instance, pinIndex](auto conn)
        {
            return (conn.m_instance == instance) && (conn.m_pinIndex == pinIndex);
        }
    );

    if (iter != m_connections.end())
    {
        // connection does not yet exist -> add it
        NetConnect conn = {.m_instance = instance, .m_pinIndex = pinIndex};
        m_connections.push_back(conn);
    }
}

Pin* CellInstance::lookupPin(const std::string &pinName)
{
    // lookup pin index

    ssize_t pinIndex = m_cell->lookupPinIndex(pinName);
    if (pinIndex < 0)
    {
        return nullptr;
    }

    if (pinIndex < m_pins.size())
    {
        return &m_pins[pinIndex];
    }

    // pin not found!
    return nullptr;
}

const Pin* CellInstance::lookupPin(const std::string &pinName) const
{
    // lookup pin index

    ssize_t pinIndex = m_cell->lookupPinIndex(pinName);
    if (pinIndex < 0)
    {
        return nullptr;
    }

    if (pinIndex < m_pins.size())
    {
        return &m_pins[pinIndex];
    }

    // pin not found!
    return nullptr;
}

Pin* CellInstance::lookupPin(size_t pinIndex)
{
    if (pinIndex < m_pins.size())
    {
        return &m_pins[pinIndex];
    }

    // pin not found!
    return nullptr;
}

const Pin* CellInstance::lookupPin(size_t pinIndex) const
{
    if (pinIndex < m_pins.size())
    {
        return &m_pins[pinIndex];
    }

    // pin not found!
    return nullptr;
}

ssize_t CellInstance::lookupPinIndex(const std::string &pinName) const
{
    return m_cell->lookupPinIndex(pinName);
}

const Pin* ModuleInstance::lookupPin(const std::string &pinName) const
{
    // lookup pin index

    ssize_t pinIndex = m_module->lookupPinIndex(pinName);
    if (pinIndex < 0)
    {
        return nullptr;
    }

    if (pinIndex < m_pins.size())
    {
        return &m_pins[pinIndex];
    }

    // pin not found!
    return nullptr;
}

const Pin* ModuleInstance::lookupPin(size_t pinIndex) const
{
    if (pinIndex < m_pins.size())
    {
        return &m_pins[pinIndex];
    }

    // pin not found!
    return nullptr;
}

Pin* ModuleInstance::lookupPin(const std::string &pinName)
{
    // lookup pin index

    ssize_t pinIndex = m_module->lookupPinIndex(pinName);
    if (pinIndex < 0)
    {
        return nullptr;
    }

    if (pinIndex < m_pins.size())
    {
        return &m_pins[pinIndex];
    }

    // pin not found!
    return nullptr;    
}

Pin* ModuleInstance::lookupPin(size_t pinIndex)
{
    if (pinIndex < m_pins.size())
    {
        return &m_pins[pinIndex];
    }

    // pin not found!
    return nullptr;
}

ssize_t ModuleInstance::lookupPinIndex(const std::string &pinName) const
{
    return m_module->lookupPinIndex(pinName);
}
