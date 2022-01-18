#pragma once

#include <memory>
#include "cell.h"
#include "netlist/netlist.h"

namespace ChipDB
{

class Module : public Cell
{
public:
    virtual bool isModule() const override
    {
        return true;
    }

    bool addInstance(std::shared_ptr<InstanceBase> insPtr);
    KeyObjPair<Net> createNet(const std::string &netName);

    /** pointer to netlist, or null if the module is a black box */
    std::unique_ptr<Netlist> m_netlist;
};

};