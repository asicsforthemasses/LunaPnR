// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <vector>
#include <string>
#include <iostream>

#include "visitor.h"
#include "namedstorage.h"
#include "net.h"
#include "instance.h"

namespace ChipDB
{

class Netlist
{
public:
    Netlist() = default;

    void clear();

    IMPLEMENT_ACCEPT;

    NamedStorage<Instance>  m_instances;
    NamedStorage<Net>       m_nets;

    std::shared_ptr<Instance> lookupInstance(InstanceObjectKey key);
    KeyObjPair<Instance> lookupInstance(const std::string &name);

    std::shared_ptr<Net> lookupNet(InstanceObjectKey key);
    KeyObjPair<Net> lookupNet(const std::string &name);

    KeyObjPair<Net> createNet(const std::string &netName);

    bool connect(const std::string &insName, const std::string &pinName, const std::string &netName);
    bool connect(InstanceObjectKey insKey, PinObjectKey pinKey, NetObjectKey netKey);

    std::size_t createUniqueID();

protected:
    std::size_t m_uniqueCounter{0};
};

};  // namespace