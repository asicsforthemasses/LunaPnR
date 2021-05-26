#include "instance.h"

using namespace ChipDB;

std::string Instance::getArchetypeName() const
{
    if (m_cell != nullptr)
        return m_cell->m_name;
    else
        return "UNKNOWN";
}

bool Instance::isModule() const
{
    return m_cell->isModule();
}

const Instance::Pin Instance::pin(ssize_t pinIndex) const
{
    auto pinInfoPtr = m_cell->m_pins[pinIndex];

    // check if the pin exists
    if (pinInfoPtr == nullptr)
    {
        // nope, return an invalid pin
        return Pin();
    }

    Pin p;
    p.m_pinIndex = pinIndex;
    p.m_info = pinInfoPtr;
    p.m_connection = m_pinToNet[pinIndex];

    return p;
}

const Instance::Pin Instance::pin(const std::string &pinName) const
{
    auto pinIndex = m_cell->m_pins.lookupIndex(pinName);
    if (pinIndex < 0)
    {
        // the pin does not exist!
        return Instance::Pin();
    }

    Pin p;
    p.m_pinIndex = pinIndex;
    p.m_info     = m_cell->m_pins[pinIndex];
    p.m_connection = m_pinToNet[pinIndex];

    return p;
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
