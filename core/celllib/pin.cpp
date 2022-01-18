#include <algorithm>
#include "pin.h"

using namespace ChipDB;

std::string ChipDB::toString(const IOType &iotype)
{
    switch(iotype)
    {
    case IOType::UNKNOWN:
        return std::string("UNKNOWN");
    case IOType::INPUT:
        return std::string("INPUT");
    case IOType::OUTPUT:
        return std::string("OUTPUT");
    case IOType::OUTPUT_TRI:
        return std::string("TRI-STATE");
    case IOType::IO:
        return std::string("INOUT");
    case IOType::POWER:
        return std::string("POWER");
    case IOType::GROUND:
        return std::string("GROUND");
    default:
        return std::string("?");
    }
};


KeyObjPair<PinInfo> PinInfoList::createPin(const std::string &name)
{    
    KeyObjPair<PinInfo> result = find(name);
    if (!result.isValid())
    {
        m_pins.push_back(std::make_shared<PinInfo>(name));
        return KeyObjPair<PinInfo>(m_pins.size()-1, m_pins.back());
    }
    
    // return the existing one
    return result;
}

/** access pin directly with bounds checking */
KeyObjPair<PinInfo> PinInfoList::operator[](const std::string &name)
{
    return find(name);
}

/** access pin directly with bounds checking */
KeyObjPair<PinInfo> PinInfoList::operator[](const std::string &name) const
{
    return find(name);
}

KeyObjPair<PinInfo> PinInfoList::find(const std::string &name) const
{
    ssize_t key = 0;
    for(auto pin : m_pins)
    {
        if (pin->name() == name)
        {
            return KeyObjPair<PinInfo>(key, pin);
        }

        key++;
    }

    return KeyObjPair<PinInfo>();
}
