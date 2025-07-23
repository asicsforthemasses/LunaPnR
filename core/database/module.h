// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <memory>
#include "cell.h"
#include "netlist.h"

namespace ChipDB
{

class Module : public Cell
{
public:
    Module(const std::string &name);

    virtual bool isModule() const override
    {
        return true;
    }

    KeyObjPair<Instance> addInstance(std::shared_ptr<Instance> insPtr);
    KeyObjPair<Net> createNet(const std::string &netName);

    bool connect(const std::string &insName, const std::string &pinName, const std::string &netName);
    bool connect(InstanceObjectKey insKey, PinObjectKey pinKey, NetObjectKey netKey);

    /** pointer to netlist, or null if the module is a black box */
    std::shared_ptr<Netlist> m_netlist;
};

};