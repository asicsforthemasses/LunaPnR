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

    /** write a placement file (for debugging)
     *  for each instance: x y xsize ysize
    */
    void writePlacementFile(std::ostream &os) const;

    IMPLEMENT_ACCEPT;

    NamedStorage<InstanceBase*> m_instances;
    NamedStorage<Net*>          m_nets;
};

};  // namespace