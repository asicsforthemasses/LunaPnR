#pragma once

#include <vector>
#include <string>

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
};

};  // namespace