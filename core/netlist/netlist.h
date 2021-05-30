#pragma once

#include <vector>
#include <string>
#include "common/visitor.h"
#include "common/namedstorage.h"

namespace ChipDB
{

class Net;      // pre-declaration
class Instance; // pre-declaration

class Netlist
{
public:
    
    IMPLEMENT_ACCEPT;

    NamedStorage<Instance*> m_instances;
    NamedStorage<Net*>      m_nets;        
};

};  // namespace