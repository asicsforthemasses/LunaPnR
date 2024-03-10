// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

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

    /** remote NETCON instances from a netlist. FIXME: does not work!!! */
    bool removeNetconInstances(ChipDB::Netlist &netlist);

    /** calculate half-perimiter wire length, ignoring the exact pin locations */
    double calcHPWL(const ChipDB::Netlist &netlist);

    /** calculate the total cell area of cells in the netlist */
    double calcTotalCellArea(const ChipDB::Netlist &netlist);
};
