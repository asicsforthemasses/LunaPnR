// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <vector>
#include "qplacertypes.h"

namespace LunaCore::QPlacer
{

struct Selector
{
    virtual bool operator()(PlacerNodeId id, const PlacerNode &node) = 0;
};

struct ExternalNodeOnNetHandler
{
    virtual void operator()(const PlacerNodeId extNodeId, const PlacerNode &extdNode,
        const PlacerNet &oldNet,
        const PlacerNetId newNet,
        PlacerNetlist& newNetlist)
    {
        // default: do nothing.
    }
};

/** create a new placer netlist from a netlist by selecting
 *  nodes using a Selector object
*/

class NetlistSplitter
{
public:
    PlacerNetlist createNetlistFromSelection(
        const PlacerNetlist &netlist,
        Selector &selector,
        ExternalNodeOnNetHandler &externalNodeHandler
        );

    PlacerNetlist createNetlistFromSelection(
        const PlacerNetlist &netlist,
        Selector &selector
        );

protected:
    /** copy a node to the new netlist and return its new node Id */
    PlacerNodeId copyNodeToNewNetlistAndClearConnections(const PlacerNode &node);
    PlacerNetId  copyNetToNewNetlistAndClearNodes(const PlacerNet &net);
    
    void addNodeToNet(PlacerNodeId nodeId, PlacerNetId netId);
    void addNetToNode(PlacerNetId netId, PlacerNodeId nodeId);

    void removePreviouslyAddedNode(PlacerNetId netId);

    void removeIfDegenerateNet(PlacerNetId netId);

    PlacerNetlist m_newNetlist;
    std::vector<PlacerNodeId> m_xlat;
};

};