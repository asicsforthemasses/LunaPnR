/*

    Quadratic look-ahead placer

*/

#pragma once

#include "common/dbtypes.h"
#include "netlist/netlist.h"
#include "qplacertypes.h"

namespace LunaCore::QLAPlacer::Private
{
    LunaCore::QPlacer::PlacerNetlist createPlacerNetlist(const ChipDB::Netlist &nl);
    bool doInitialPlacement(const ChipDB::Rect64 &regionRect, LunaCore::QPlacer::PlacerNetlist &netlist);
    bool doQuadraticB2B(LunaCore::QPlacer::PlacerNetlist &netlist);
    bool updatePositions(const LunaCore::QPlacer::PlacerNetlist &netlist, ChipDB::Netlist &nl);
};

namespace LunaCore::QLAPlacer
{

    bool place(const ChipDB::Rect64 &regionRect, const ChipDB::Module *mod);

};

