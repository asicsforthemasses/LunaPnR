// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <iostream>
#include "qplacertypes.h"

using namespace LunaCore::QPlacer;

PlacerNetId PlacerNetlist::createNet()
{
    m_nets.emplace_back();
    return m_nets.size()-1;
}

PlacerNodeId PlacerNetlist::createNode()
{
    m_nodes.emplace_back();
    return m_nodes.size()-1;
}

void PlacerNetlist::dump(std::ostream &os) const
{
    os << "PlacerNetlist:\n";
    size_t idx = 0;
    for(auto const& node: m_nodes)
    {
        std::cout << "Node: " << idx << " ";
        node.dump(os);
        idx++;
    }

    idx = 0;
    for(auto const& net: m_nets)
    {
        std::cout << "Net: " << idx << " ";
        net.dump(os);
        idx++;
    }
    os << "done.\n";
}
