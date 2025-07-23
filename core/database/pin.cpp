// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <algorithm>
#include "pin.h"

using namespace ChipDB;

std::string ChipDB::toString(const IOType &iotype)
{
    switch(iotype)
    {
    case IOType::UNKNOWN:
        return "UNKNOWN";
    case IOType::INPUT:
        return "INPUT";
    case IOType::OUTPUT:
        return "OUTPUT";
    case IOType::OUTPUT_TRI:
        return "TRI-STATE";
    case IOType::IO:
        return "INOUT";
    case IOType::POWER:
        return "POWER";
    case IOType::GROUND:
        return "GROUND";
    default:
        return "?";
    }
};


KeyObjPair<PinInfo> PinInfoList::createPin(const std::string &name)
{
    KeyObjPair<PinInfo> result = find(name);
    if (!result.isValid())
    {
        m_pins.push_back(std::make_shared<PinInfo>(name));
        return {static_cast<ObjectKey>(m_pins.size()-1), m_pins.back()};
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
    ObjectKey key = 0;
    for(auto pin : m_pins)
    {
        if (pin->name() == name)
        {
            return {key, pin};
        }

        key++;
    }

    return {};
}
