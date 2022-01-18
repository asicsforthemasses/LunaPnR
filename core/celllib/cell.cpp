#include "cell.h"

using namespace ChipDB;

// **********************************************************************
//   Cell
// **********************************************************************

KeyObjPair<PinInfo> Cell::createPin(const std::string &name)
{
    return m_pins.createPin(name);
}

KeyObjPair<PinInfo> Cell::lookupPin(const std::string &name)
{
    return m_pins[name];
}

KeyObjPair<PinInfo> Cell::lookupPin(const std::string &name) const
{
    return m_pins[name];
}

std::shared_ptr<Cell> Cell::lookupPin(ObjectKey key)
{
    return m_pins[key];
}
