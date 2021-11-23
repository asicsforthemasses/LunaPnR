#include "cell.h"

using namespace ChipDB;

// **********************************************************************
//   Cell
// **********************************************************************

PinInfo* Cell::createPin(const std::string &name)
{
    return m_pins.createPin(name);
}

PinInfo* Cell::lookupPin(const std::string &name)
{
    return m_pins.lookup(name);
}

const PinInfo* Cell::lookupPin(const std::string &name) const
{
    return m_pins.lookup(name);
}

ssize_t Cell::lookupPinIndex(const std::string &name) const
{
    return m_pins.lookupIndex(name);
}

PinInfo* Cell::lookupPin(ssize_t index)
{
    return m_pins[index];
}

const PinInfo* Cell::lookupPin(ssize_t index) const
{
    return m_pins[index];
}


