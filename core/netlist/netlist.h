#pragma once

#include <vector>
#include <string>
#include <iostream>

#include "common/visitor.h"
#include "common/namedstorage.h"
#include "net.h"
#include "instance.h"

namespace ChipDB
{

class Netlist
{
public:
    
    void clear();

    IMPLEMENT_ACCEPT;

    NamedStorage<InstanceBase*> m_instances;
    NamedStorage<Net*>          m_nets;

    Net* createNet(const std::string &netName);

    bool connect(const std::string &insName, const std::string &pinName, const std::string &netName);
};

};  // namespace