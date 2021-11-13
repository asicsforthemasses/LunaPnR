#pragma once

#include "qplacertypes.h"

namespace LunaCore::QPlacer
{

struct Selector
{
    virtual bool operator()(PlacerNodeId id, const PlacerNode &node) = 0;
};

/** create a new placer netlist from a netlist by selecting
 *  nodes using a Selector object
*/

PlacerNetlist createNetlistFromSelection(
    const PlacerNetlist &netlist,
    Selector &selector
    );

};