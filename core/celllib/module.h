/*
  LunaPnR Source Code
  
  SPDX-License-Identifier: GPL-3.0-only
  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/


#pragma once

#include <memory>
#include "cell.h"
#include "netlist/netlist.h"

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

    KeyObjPair<InstanceBase> addInstance(std::shared_ptr<InstanceBase> insPtr);
    KeyObjPair<Net> createNet(const std::string &netName);

    bool connect(const std::string &insName, const std::string &pinName, const std::string &netName);
    bool connect(InstanceObjectKey insKey, PinObjectKey pinKey, NetObjectKey netKey);    

    /** pointer to netlist, or null if the module is a black box */
    std::shared_ptr<Netlist> m_netlist;
};

};