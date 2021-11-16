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
        else
        {
            // node is not part of the selection
            // for certain algorithms, we need to do terminal
            // propagation and put a node somewhere on the
            // boundary or in the left/right half of the new region
        }
        nodeId++;
    }

    // go through all the nets in the netlist
    // and check if a new node is 

    PlacerNetId netId = 0;
    for(auto const& net : netlist.m_nets)
    {
        PlacerNetId newNetId = -1;
        for(auto const& nodeInNetId : net.m_nodes)
        {
            auto newNodeId = m_xlat.at(nodeInNetId);
            if (newNodeId != -1)
            {
                // node is used in sub netlist
                // create this net for the new netlist
                // if it doesn't exist.
                if (newNetId == -1)
                {                    
                    newNetId = copyNetToNewNetlistAndClearNodes(net);
                }

                // tell the node about the net
                addNetToNode(newNetId, newNodeId);

                // tell the net about the node
                addNodeToNet(newNodeId, newNetId);
            }
            else
            {
                // node on net is not part of the selection
            }
        }

        // check if the new net is degenerate; contains only one node
        if (newNetId != -1)
        {
            auto netSize = m_newNetlist.m_nets.at(newNetId).m_nodes.size();
        
            if (netSize == 1)
            {
                std::stringstream ss;
                ss << "Net " << newNetId << " is degenerate\n";
                doLog(LOG_VERBOSE, ss);

                removePreviouslyAddedNode(newNetId);

                // remove degenerate net
                m_newNetlist.m_nets.pop_back();
            }
            else if (netSize == 0)
            {
                std::stringstream ss;
                ss << "Net " << newNetId << " is degenerate\n";
                doLog(LOG_VERBOSE, ss);

                // remove degenerate net
                m_newNetlist.m_nets.pop_back();
            }
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
