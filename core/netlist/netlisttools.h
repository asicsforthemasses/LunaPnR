#pragma once

#include <map>
#include "netlist.h"

namespace LunaCore::NetlistTools
{

    /** write a placement file (for debugging)
     *  for each instance: x y xsize ysize
    */
    bool writePlacementFile(std::ostream &os, const ChipDB::Netlist *netlist);

    using NetlistHistogram = std::map<size_t /* number of connections */, size_t /* number of occurences */>;

    /** returns a histogram of the net connections */
    NetlistHistogram calcNetlistHistogram(const ChipDB::Netlist *netlist);

};