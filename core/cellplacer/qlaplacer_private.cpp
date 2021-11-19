#include <random>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/IterativeLinearSolvers>

#include "common/logging.h"
#include "qlaplacer.h"

using namespace LunaCore::QLAPlacer::Private;

using QLAPlacerNetlist = LunaCore::QPlacer::PlacerNetlist;

bool LunaCore::QLAPlacer::Private::doInitialPlacement(const ChipDB::Rect64 &regionRect, QLAPlacerNetlist &netlist)
{
    // place cells throughout the region uniformly

    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()

    std::uniform_int_distribution<ChipDB::CoordType> x_distribution(regionRect.left(), regionRect.right());
    std::uniform_int_distribution<ChipDB::CoordType> y_distribution(regionRect.bottom(), regionRect.top());

    for(auto &node : netlist.m_nodes)
    {
        if (!node.isFixed())
        {
            node.m_pos = ChipDB::Coord64{x_distribution(gen), y_distribution(gen)};

            // make sure the node isn't slightly outside the rectangle
            auto nodeRightPos = node.m_pos.m_x + node.m_size.m_x;
            auto nodeTopPos   = node.m_pos.m_y + node.m_size.m_y;

            if (nodeRightPos > regionRect.right())
            {
                node.m_pos.m_x -= nodeRightPos - regionRect.right();
            }

            if (nodeTopPos > regionRect.top())
            {
                node.m_pos.m_y -= nodeTopPos - regionRect.top();
            }
        }
    }

    return true;
}

bool LunaCore::QLAPlacer::Private::updatePositions(const LunaCore::QPlacer::PlacerNetlist &netlist, ChipDB::Netlist &nl)
{
    if (nl.m_instances.size() != netlist.m_nodes.size())
    {
        doLog(LOG_ERROR,"updatePositions: netlist mismatch!\n");
        return false;
    }

    //NOTE: this assumes the node order has not changed..
    LunaCore::QPlacer::PlacerNodeId nodeIdx = 0;
    for(auto &ins : nl.m_instances)
    {
        if ((ins != nullptr) && (ins->m_placementInfo != ChipDB::PlacementInfo::PLACEDANDFIXED) && (ins->m_placementInfo != ChipDB::PlacementInfo::IGNORE))
        {
            ins->m_pos = netlist.m_nodes.at(nodeIdx).m_pos;
            ins->m_placementInfo = ChipDB::PlacementInfo::PLACED;
            //doLog(LOG_VERBOSE,"  ins: %s -> %d,%d\n", ins->m_name.c_str(), ins->m_pos.m_x, ins->m_pos.m_y);
        }
        nodeIdx++;
    }   

    return true;
}

LunaCore::QPlacer::PlacerNetlist LunaCore::QLAPlacer::Private::createPlacerNetlist(const ChipDB::Netlist &nl)
{
    std::unordered_map<ChipDB::InstanceBase*, LunaCore::QPlacer::PlacerNodeId> ins2nodeId;
    std::unordered_map<ChipDB::Net*, LunaCore::QPlacer::PlacerNetId> net2netId;
    LunaCore::QPlacer::PlacerNetlist netlist;

    // create placer nodes
    for(auto ins : nl.m_instances)
    {
        auto nodeId = netlist.createNode();
        auto& placerNode = netlist.getNode(nodeId);
        placerNode.m_type = LunaCore::QPlacer::PlacerNodeType::MovableNode;
        placerNode.m_pos  = ChipDB::Coord64{0,0};
        placerNode.m_size = ins->instanceSize();

        if (ins->m_placementInfo == ChipDB::PlacementInfo::PLACEDANDFIXED)
        {
            placerNode.m_type = LunaCore::QPlacer::PlacerNodeType::FixedNode;
            placerNode.m_pos  = ins->m_pos;
            placerNode.m_weight = 10.0; // increase the weight of fixed nodes to pull apart the movable instances
        }

        ins2nodeId[ins] = netlist.numberOfNodes()-1;
    }

    // create placer nets
    for(auto net : nl.m_nets)
    {
        auto placerNetId = netlist.createNet();
        auto& placerNet = netlist.getNet(placerNetId);

        for(auto& conn : net->m_connections)
        {
            auto ins = conn.m_instance;
            auto iter = ins2nodeId.find(ins);

            if (iter == ins2nodeId.end())
            {
                doLog(LOG_ERROR, "createPlacerNetlist: cannot find node\n");
                return LunaCore::QPlacer::PlacerNetlist();
            }

            auto placerNodeId = iter->second;
            placerNet.m_nodes.push_back(placerNodeId);
            netlist.getNode(placerNodeId).m_connections.push_back(placerNetId);
        }
    }

    return netlist;
}

template<class AxisAccessor>
void updateWeights(const LunaCore::QPlacer::PlacerNetlist &netlist, 
    const LunaCore::QPlacer::PlacerNodeId &node1Id, 
    const LunaCore::QPlacer::PlacerNodeId &node2Id, 
    double netWeight,
    ssize_t netSize)
{
    auto const & node1 = netlist.m_nodes.at(node1Id);
    auto const & node2 = netlist.m_nodes.at(node2Id);

    auto distance = std::abs(AxisAccessor::get(node1.m_pos) - AxisAccessor::get(node2.m_pos));

    // make sure distance is some sane minimum to avoid division by zero..
    distance = std::max(static_cast<ChipDB::CoordType>(10), distance);

    double weight = 1.0/(static_cast<double>(netSize-1) * static_cast<double>(distance));

    doLog(LOG_INFO, "Add connection %s: (%d,%d) with weight %f\n", AxisAccessor::m_name, node1Id, node2Id, weight);
    doLog(LOG_INFO, "Add connection %s: (%d,%d) with weight %f\n", AxisAccessor::m_name, node2Id, node1Id, weight);
}


struct ExtremaResults
{
    LunaCore::QPlacer::PlacerNodeId m_minNodeIdx;
    LunaCore::QPlacer::PlacerNodeId m_maxNodeIdx;
    ChipDB::CoordType m_min;
    ChipDB::CoordType m_max;
};

template<class AxisAccessor>
ExtremaResults findExtremeNodes(const LunaCore::QPlacer::PlacerNetlist &netlist,
    const LunaCore::QPlacer::PlacerNet &net)
{
    ExtremaResults results;
    results.m_min = std::numeric_limits<decltype(results.m_min)>::max();
    results.m_max = std::numeric_limits<decltype(results.m_max)>::min();
    results.m_minNodeIdx = 0;
    results.m_maxNodeIdx = 0;

    for(auto const& nodeId : net.m_nodes)
    {
        auto const& node = netlist.m_nodes.at(nodeId);
        auto pos = AxisAccessor::get(node.m_pos);
        if (pos > results.m_max)
        {
            results.m_maxNodeIdx = nodeId;
            results.m_max = pos;
        }
        if (pos < results.m_min)
        {
            results.m_minNodeIdx = nodeId;
            results.m_min = pos;
        }        
    }

    assert(results.m_minNodeIdx != results.m_maxNodeIdx);

    return results;
};


bool LunaCore::QLAPlacer::Private::doQuadraticB2B(LunaCore::QPlacer::PlacerNetlist &netlist)
{
    // two-pin nets are connected together with weight w=1/|length|
    //
    // multi-pin nets: extreme nodes are connected together and 
    //                 each to the interior nodes using weight
    //                 w = 1/((p-1)|length_of_edge|)

    size_t degenerateNets = 0;

    for(auto const& net : netlist.m_nets)
    {
        if (net.m_nodes.size() == 2)
        {
            updateWeights<ChipDB::XAxisAccessor>(netlist, 
                net.m_nodes.at(0), net.m_nodes.at(1), 
                net.m_weight, net.m_nodes.size());

            updateWeights<ChipDB::YAxisAccessor>(netlist, 
                net.m_nodes.at(0), net.m_nodes.at(1), 
                net.m_weight, net.m_nodes.size());                
        }
        else if (net.m_nodes.size() > 2)
        {
            // find the extrema nodes on the net
            auto xResult = findExtremeNodes<ChipDB::XAxisAccessor>(netlist, net);
            auto yResult = findExtremeNodes<ChipDB::YAxisAccessor>(netlist, net);

            doLog(LOG_VERBOSE, "xmin: %d nodeIdx %d\n", xResult.m_min, xResult.m_minNodeIdx);
            doLog(LOG_VERBOSE, "xmax: %d nodeIdx %d\n", xResult.m_max, xResult.m_maxNodeIdx);
            doLog(LOG_VERBOSE, "ymin: %d nodeIdx %d\n", yResult.m_min, yResult.m_minNodeIdx);
            doLog(LOG_VERBOSE, "ymax: %d nodeIdx %d\n", yResult.m_max, yResult.m_maxNodeIdx);

            // connect the extreme nodes together
            updateWeights<ChipDB::XAxisAccessor>(netlist, 
                xResult.m_minNodeIdx, xResult.m_maxNodeIdx,
                net.m_weight, net.m_nodes.size());

            updateWeights<ChipDB::YAxisAccessor>(netlist, 
                yResult.m_minNodeIdx, yResult.m_maxNodeIdx,
                net.m_weight, net.m_nodes.size());

            // connect each internal node to the min node
            for(auto const nodeIdx : net.m_nodes)
            {
                if ((nodeIdx != xResult.m_minNodeIdx) && (nodeIdx != xResult.m_maxNodeIdx))
                {
                    updateWeights<ChipDB::XAxisAccessor>(netlist, 
                        xResult.m_minNodeIdx, nodeIdx,
                        net.m_weight, net.m_nodes.size());

                    updateWeights<ChipDB::XAxisAccessor>(netlist, 
                        xResult.m_maxNodeIdx, nodeIdx,
                        net.m_weight, net.m_nodes.size());
                }

                if ((nodeIdx != yResult.m_minNodeIdx) && (nodeIdx != yResult.m_maxNodeIdx))
                {
                    updateWeights<ChipDB::YAxisAccessor>(netlist, 
                        yResult.m_minNodeIdx, nodeIdx,
                        net.m_weight, net.m_nodes.size());

                    updateWeights<ChipDB::YAxisAccessor>(netlist, 
                        yResult.m_maxNodeIdx, nodeIdx,
                        net.m_weight, net.m_nodes.size());
                }
            }
        }
        else
        {
            degenerateNets++;
        }
    }

    return true;
}
