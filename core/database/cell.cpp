// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

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

std::shared_ptr<PinInfo> Cell::lookupPin(ObjectKey key)
{
    return m_pins[key];
}

std::shared_ptr<PinInfo> Cell::lookupPin(ObjectKey key) const
{
    return m_pins[key];
}
