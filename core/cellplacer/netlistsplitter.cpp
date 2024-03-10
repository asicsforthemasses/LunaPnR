// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <cassert>
#include "common/logging.h"
#include "netlistsplitter.h"

using namespace LunaCore::QPlacer;

PlacerNodeId NetlistSplitter::copyNodeToNewNetlistAndClearConnections(const PlacerNode &node)
{
    auto newNodeId = m_newNetlist.createNode();
    m_newNetlist.m_nodes.at(newNodeId) = node;
    m_newNetlist.m_nodes.at(newNodeId).m_connections.clear();

    return newNodeId;
}

PlacerNetId NetlistSplitter::copyNetToNewNetlistAndClearNodes(const PlacerNet &net)
{
    auto newNetId = m_newNetlist.createNet();
    m_newNetlist.m_nets.at(newNetId) = net;
    m_newNetlist.m_nets.at(newNetId).m_nodes.clear();
    return newNetId;
}

void NetlistSplitter::addNodeToNet(PlacerNodeId nodeId, PlacerNetId netId)
{
    m_newNetlist.m_nets.at(netId).m_nodes.push_back(nodeId);
}

void NetlistSplitter::addNetToNode(PlacerNetId netId, PlacerNodeId nodeId)
{
    m_newNetlist.m_nodes.at(nodeId).m_connections.push_back(netId);
}

PlacerNetlist NetlistSplitter::createNetlistFromSelection(
    const PlacerNetlist &netlist,
    Selector &selector
    )
{
    ExternalNodeOnNetHandler nullHandler;
    return createNetlistFromSelection(netlist, selector, nullHandler);
}

PlacerNetlist NetlistSplitter::createNetlistFromSelection(
    const PlacerNetlist &netlist,
    Selector &selector,
    ExternalNodeOnNetHandler &externalNodeHandler
    )
{
    m_newNetlist.m_nodes.clear();
    m_newNetlist.m_nets.clear();

    m_xlat.clear();
    m_xlat.resize(netlist.m_nodes.size(), -1);

    // select the nodes from the netlist
    // and copy to the new netlist
    PlacerNodeId nodeId = 0;
    for(auto const& node : netlist.m_nodes)
    {
        if(selector(nodeId, node))
        {
            auto newNodeId = copyNodeToNewNetlistAndClearConnections(node);
            m_xlat.at(nodeId) = newNodeId;
        }
        nodeId++;
    }

    // go through all the nets in the netlist
    // and check if a new node is part of the selection
    // if so, add it to a new net
    //
    // if the net turns out to be degenerate (1 or fewer nodes)
    // the net is removed

    PlacerNetId netId = 0;
    for(auto const& oldNet : netlist.m_nets)
    {
        PlacerNetId newNetId = -1;
        for(auto const& nodeInNetId : oldNet.m_nodes)
        {
            auto newNodeId = m_xlat.at(nodeInNetId);
            if (newNodeId != -1)
            {
                // node is used in sub netlist
                // create this net for the new netlist
                // if it doesn't exist.
                if (newNetId == -1)
                {
                    newNetId = copyNetToNewNetlistAndClearNodes(oldNet);
                }

                // tell the node about the net
                addNetToNode(newNetId, newNodeId);

                // tell the net about the node
                addNodeToNet(newNodeId, newNetId);
            }
            else
            {
                // node on net is not part of the selection
                // we have the opportunity to add special anchor nodes
                // here to keep a cluster of cells roughy in
                // the desired half

                externalNodeHandler(nodeInNetId,
                    netlist.m_nodes.at(nodeInNetId),
                    oldNet,
                    newNetId,
                    m_newNetlist);
            }
        }

        // check if the new net is degenerate; contains only one node
        if (newNetId != -1)
        {
            removeIfDegenerateNet(newNetId);
        }

        netId++;
    }

    return m_newNetlist;
}

void NetlistSplitter::removePreviouslyAddedNode(PlacerNetId netId)
{
    // tell the connected node, if there is any, to disconnect
    auto newNodeId = m_newNetlist.m_nets.back().m_nodes.front();
    auto &newNode = m_newNetlist.m_nodes.at(newNodeId);

    assert(newNode.m_connections.size() > 0);
    assert(newNode.m_connections.back() == netId);

    newNode.m_connections.pop_back();
}

void NetlistSplitter::removeIfDegenerateNet(PlacerNetId netId)
{
    auto netSize = m_newNetlist.m_nets.at(netId).m_nodes.size();

    if (netSize == 1)
    {
        std::stringstream ss;
        ss << "Net " << netId << " is degenerate\n";
        Logging::doLog(Logging::LogType::VERBOSE, ss);

        removePreviouslyAddedNode(netId);

        // remove degenerate net
        m_newNetlist.m_nets.pop_back();
    }
    else if (netSize == 0)
    {
        std::stringstream ss;
        ss << "Net " << netId << " is degenerate\n";
        Logging::doLog(Logging::LogType::VERBOSE, ss);

        // remove degenerate net
        m_newNetlist.m_nets.pop_back();
    }
}