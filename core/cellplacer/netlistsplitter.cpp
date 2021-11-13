#include <cassert>
#include "common/logging.h"
#include "netlistsplitter.h"

using namespace LunaCore::QPlacer;

PlacerNetlist LunaCore::QPlacer::createNetlistFromSelection(
    const PlacerNetlist &netlist,
    Selector &selector
    )
{
    LunaCore::QPlacer::PlacerNetlist newNetlist;
    std::vector<LunaCore::QPlacer::PlacerNodeId> xlat(netlist.m_nodes.size(), -1);

    // select the nodes from the netlist
    // and copy to the new netlist
    PlacerNodeId nodeId = 0;
    for(auto const& node : netlist.m_nodes)
    {
        if(selector(nodeId, node))
        {
            auto newNodeId = newNetlist.createNode();
            xlat.at(nodeId) = newNodeId;
            newNetlist.m_nodes.at(newNodeId) = node;
            newNetlist.m_nodes.at(newNodeId).m_connections.clear();
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
            auto newNodeId = xlat.at(nodeInNetId);
            if (newNodeId != -1)
            {
                // node is used in sub netlist
                // create this net for the new netlist
                // if it doesn't exist.
                if (newNetId == -1)
                {                    
                    newNetId = newNetlist.createNet();
                    newNetlist.m_nets.at(newNetId) = net;
                    newNetlist.m_nets.at(newNetId).m_nodes.clear();
                }

                // tell the node about the net
                auto& newNode = newNetlist.m_nodes.at(newNodeId);
                newNode.m_connections.push_back(newNetId);

                // tell the net about the node
                newNetlist.m_nets.at(newNetId).m_nodes.push_back(newNodeId);
            }
            else
            {
                // node on net is not part of the selection
            }
        }

        // check if the new net is degenerate; contains only one node
        if (newNetId != -1)
        {
            auto netSize = newNetlist.m_nets.at(newNetId).m_nodes.size();            
        
            if (netSize == 1)
            {
                std::stringstream ss;
                ss << "Net " << newNetId << " is degenerate\n";
                doLog(LOG_VERBOSE, ss);

                // tell the connected node, if there is any, to disconnect 
                auto newNodeId = newNetlist.m_nets.back().m_nodes.front();
                auto &newNode = newNetlist.m_nodes.at(newNodeId);

                assert(newNode.m_connections.size() > 0);
                assert(newNode.m_connections.back() == newNetId);

                newNode.m_connections.pop_back();

                // remove degenerate net
                newNetlist.m_nets.pop_back();
            }
            else if (netSize == 0)
            {
                std::stringstream ss;
                ss << "Net " << newNetId << " is degenerate\n";
                doLog(LOG_VERBOSE, ss);

                // remove degenerate net
                newNetlist.m_nets.pop_back();
            }
        }

        netId++;
    }

    return newNetlist;    
}
