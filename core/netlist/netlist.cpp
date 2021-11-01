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

#if 0
// ************************************************************************
//   CellInstance
// ************************************************************************

bool CellInstance::connectPin(size_t pinIndex, Net *net)
{
    if (pinIndex < m_pinToNet.size())
    {
        m_pinToNet[pinIndex] = net;
        return true;
    }
    else
    {
        return false;
    }
}

AbstractInstance::PinConnection CellInstance::pin(const std::string &pinName)
{
    // lookup pin index
    ssize_t pinIndex = m_cell->lookupPinIndex(pinName);
    if (pinIndex < 0)
    {
        return AbstractInstance::PinConnection();
    }

    if (pinIndex < m_pinToNet.size())
    {
        return AbstractInstance::PinConnection(&m_cell->m_pins.at(pinIndex), pinIndex, m_pinToNet[pinIndex]);
    }

    // pin not found!
    return AbstractInstance::PinConnection();
}

const AbstractInstance::PinConnection CellInstance::pin(const std::string &pinName) const
{
    // lookup pin index
    ssize_t pinIndex = m_cell->lookupPinIndex(pinName);
    if (pinIndex < 0)
    {
        return AbstractInstance::PinConnection();
    }

    if (pinIndex < m_pinToNet.size())
    {
        return AbstractInstance::PinConnection(&m_cell->m_pins.at(pinIndex), pinIndex, m_pinToNet[pinIndex]);
    }

    // pin not found!
    return AbstractInstance::PinConnection();
}

AbstractInstance::PinConnection CellInstance::pin(size_t pinIndex)
{
    if (pinIndex < m_pinToNet.size())
    {
        return AbstractInstance::PinConnection(&m_cell->m_pins.at(pinIndex), pinIndex, m_pinToNet[pinIndex]);
    }

    // pin not found!
    return AbstractInstance::PinConnection();
}

const AbstractInstance::PinConnection CellInstance::pin(size_t pinIndex) const
{
    if (pinIndex < m_pinToNet.size())
    {
        return AbstractInstance::PinConnection(&m_cell->m_pins.at(pinIndex), pinIndex, m_pinToNet[pinIndex]);
    }

    // pin not found!
    return AbstractInstance::PinConnection();
}

ssize_t CellInstance::pinIndex(const std::string &pinName) const
{
    return m_cell->lookupPinIndex(pinName);
}


// ************************************************************************
//   ModuleInstance
// ************************************************************************

bool ModuleInstance::connectPin(size_t pinIndex, Net *net)
{
    if (pinIndex < m_pinToNet.size())
    {
        m_pinToNet[pinIndex] = net;
        return true;
    }
    else
    {
        return false;
    }
}

const AbstractInstance::PinConnection ModuleInstance::pin(const std::string &pinName) const
{
    // lookup pin index
    ssize_t pinIndex = m_module->lookupPinIndex(pinName);
    if (pinIndex < 0)
    {
        return AbstractInstance::PinConnection();
    }

    if (pinIndex < m_pinToNet.size())
    {
        return AbstractInstance::PinConnection(&m_module->m_pins.at(pinIndex), pinIndex, m_pinToNet[pinIndex]);
    }

    // pin not found!
    return AbstractInstance::PinConnection();
}

const AbstractInstance::PinConnection ModuleInstance::pin(size_t pinIndex) const
{
    if (pinIndex < m_pinToNet.size())
    {
        return AbstractInstance::PinConnection(&m_module->m_pins.at(pinIndex), pinIndex, m_pinToNet[pinIndex]);
    }

    // pin not found!
    return AbstractInstance::PinConnection();
}

AbstractInstance::PinConnection ModuleInstance::pin(const std::string &pinName)
{
    // lookup pin index
    ssize_t pinIndex = m_module->lookupPinIndex(pinName);
    if (pinIndex < 0)
    {
        return AbstractInstance::PinConnection();
    }

    if (pinIndex < m_pinToNet.size())
    {
        return AbstractInstance::PinConnection(&m_module->m_pins.at(pinIndex), pinIndex, m_pinToNet[pinIndex]);
    }

    // pin not found!
    return AbstractInstance::PinConnection();   
}

AbstractInstance::PinConnection ModuleInstance::pin(size_t pinIndex)
{
    if (pinIndex < m_pinToNet.size())
    {
        return AbstractInstance::PinConnection(&m_module->m_pins.at(pinIndex), pinIndex, m_pinToNet[pinIndex]);
    }

    // pin not found!
    return AbstractInstance::PinConnection();
}

ssize_t ModuleInstance::pinIndex(const std::string &pinName) const
{
    return m_module->lookupPinIndex(pinName);
}
#endif


