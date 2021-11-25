#include <random>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/IterativeLinearSolvers>

#include "common/logging.h"
#include "qlaplacer.h"

using namespace LunaCore::QLAPlacer::Private;

using QLAPlacerNetlist = LunaCore::QPlacer::PlacerNetlist;

std::string toString(Eigen::ComputationInfo info)
{
    std::string str;
    switch(info)
    {
    case Eigen::ComputationInfo::Success:
        return "OK";
    case Eigen::ComputationInfo::NoConvergence:
        return "No convergence";
    case Eigen::ComputationInfo::NumericalIssue:
        return "Numerical issue";
    case Eigen::ComputationInfo::InvalidInput:
        return "Invalid input";    
    }

    return "Unknown";
}

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
    doLog(LOG_VERBOSE,"Updating instance positions: \n");
    LunaCore::QPlacer::PlacerNodeId nodeIdx = 0;
    for(auto *ins : nl.m_instances)
    {
        if ((ins != nullptr) && (ins->m_placementInfo != ChipDB::PlacementInfo::PLACEDANDFIXED) && (ins->m_placementInfo != ChipDB::PlacementInfo::IGNORE))
        {
            auto const& node = netlist.m_nodes.at(nodeIdx);
            //ins->m_pos = node.m_pos - ChipDB::Coord64{node.m_size.m_x/2, node.m_size.m_y/2};
            ins->m_pos = node.m_pos;
            ins->m_placementInfo = ChipDB::PlacementInfo::PLACED;
            doLog(LOG_VERBOSE,"  ins: %s -> %d,%d\n", ins->m_name.c_str(), ins->m_pos.m_x, ins->m_pos.m_y);
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
    for(auto * ins : nl.m_instances)
    {
        auto nodeId = netlist.createNode();
        auto& placerNode = netlist.getNode(nodeId);
        placerNode.m_type = LunaCore::QPlacer::PlacerNodeType::MovableNode;
        placerNode.m_size = ins->instanceSize();
        //placerNode.m_pos  = ins->m_pos + ChipDB::Coord64{placerNode.m_size.m_x/2, placerNode.m_size.m_y/2};
        placerNode.m_pos  = ins->m_pos;
        placerNode.m_weight = 1.0;
        
        if (ins->m_placementInfo == ChipDB::PlacementInfo::PLACEDANDFIXED)
        {
            placerNode.m_type = LunaCore::QPlacer::PlacerNodeType::FixedNode;
            //placerNode.m_pos  = ins->m_pos + ChipDB::Coord64{placerNode.m_size.m_x/2, placerNode.m_size.m_y/2};
            placerNode.m_weight = 4.0; // increase the weight of fixed nodes to pull apart the movable instances
        }

        ins2nodeId[ins] = nodeId;
    }

    // create placer nets
    for(auto net : nl.m_nets)
    {
        auto placerNetId = netlist.createNet();
        auto& placerNet = netlist.getNet(placerNetId);
        placerNet.m_weight = 1.0;

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
void updateWeights(
    LunaCore::QLAPlacer::Private::SolverData &solverData,
    const LunaCore::QPlacer::PlacerNetlist &netlist, 
    const LunaCore::QPlacer::PlacerNodeId &node1Id, 
    const LunaCore::QPlacer::PlacerNodeId &node2Id, 
    double netWeight,
    ssize_t netSize)
{
    auto const & node1 = netlist.m_nodes.at(node1Id);
    auto const & node2 = netlist.m_nodes.at(node2Id);

    auto distance = std::abs(AxisAccessor::get(node1.m_pos) - AxisAccessor::get(node2.m_pos));

    // make sure distance is some sane minimum to avoid division by zero..
    distance = std::max(static_cast<ChipDB::CoordType>(1), distance);

#if 1
    double weight = netWeight/(static_cast<double>(netSize-1) * static_cast<double>(distance));
#else
    double weight = netWeight/(static_cast<double>(netSize-1));
#endif
    // update the solver matrix according to the node mobility
    // e.g. if both nodes are fixed, don't add anything!

    if (node1.isFixed() && node2.isFixed())
    {
        return;
    }

    if (node1.isFixed() || node2.isFixed())
    {
        // one of the two nodes is fixed
        if (node1.isFixed())
        {
            solverData.m_Amat.coeffRef(node2Id, node2Id)  += weight * node1.m_weight;
            solverData.m_Amat.coeffRef(node1Id, node1Id)  = 1.0f; // attempt to make life of solver easier
            solverData.m_Bvec[node2Id] = weight * AxisAccessor::get(node1.m_pos);
        }
        else
        {
            solverData.m_Amat.coeffRef(node1Id, node1Id)  += weight * node2.m_weight;
            solverData.m_Amat.coeffRef(node2Id, node2Id)  = 1.0f; // attempt to make life of solver easier
            solverData.m_Bvec[node1Id] = weight * AxisAccessor::get(node2.m_pos);
        }
    }
    else
    {
        // both nodes are movable
        solverData.m_Amat.coeffRef(node1Id, node1Id)  += weight;
        solverData.m_Amat.coeffRef(node2Id, node2Id)  += weight;
        solverData.m_Amat.coeffRef(node1Id, node2Id)  -= weight;
        solverData.m_Amat.coeffRef(node2Id, node1Id)  -= weight;

        if ((node1Id == 20) || (node2Id == 20))
        {
            doLog(LOG_VERBOSE, "Add connection %s: (%d,%d) with weight %f (distance=%ld)\n", AxisAccessor::m_name, node1Id, node2Id, netWeight, distance);
            doLog(LOG_VERBOSE, "Add connection %s: (%d,%d) with weight %f (distance=%ld)\n", AxisAccessor::m_name, node2Id, node1Id, netWeight, distance);
        }
    }

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

    while (results.m_minNodeIdx == results.m_maxNodeIdx)
    {
        // min and max nodes are the same 
        // so we randomly choose another
        results.m_maxNodeIdx = rand() % net.m_nodes.size();
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

    doLog(LOG_INFO, "Building solver matrices\n");

    LunaCore::QLAPlacer::Private::SolverData XSolverData;
    LunaCore::QLAPlacer::Private::SolverData YSolverData;

    XSolverData.m_Amat.resize(netlist.numberOfNodes(), netlist.numberOfNodes());
    XSolverData.m_Amat.reserve(2*netlist.numberOfNodes());    // rough estimate of size
    XSolverData.m_Bvec = Eigen::VectorXd::Zero(netlist.numberOfNodes());

    YSolverData.m_Amat.resize(netlist.numberOfNodes(), netlist.numberOfNodes());
    YSolverData.m_Amat.reserve(2*netlist.numberOfNodes());    // rough estimate of size
    YSolverData.m_Bvec = Eigen::VectorXd::Zero(netlist.numberOfNodes());

    ssize_t netIdx = 0;
    for(auto const& net : netlist.m_nets)
    {
        //doLog(LOG_VERBOSE,"Net %d:\n", netIdx);
        if (net.m_nodes.size() == 2)
        {
            updateWeights<ChipDB::XAxisAccessor>(XSolverData, 
                netlist, 
                net.m_nodes.at(0), net.m_nodes.at(1), 
                net.m_weight, net.m_nodes.size());

            updateWeights<ChipDB::YAxisAccessor>(YSolverData, 
                netlist, 
                net.m_nodes.at(0), net.m_nodes.at(1), 
                net.m_weight, net.m_nodes.size());
        }
        else if (net.m_nodes.size() > 2)
        {
            // find the extrema nodes on the net
            auto xResult = findExtremeNodes<ChipDB::XAxisAccessor>(netlist, net);
            auto yResult = findExtremeNodes<ChipDB::YAxisAccessor>(netlist, net);

#if 0
            doLog(LOG_VERBOSE, "xmin: %d nodeIdx %d\n", xResult.m_min, xResult.m_minNodeIdx);
            doLog(LOG_VERBOSE, "xmax: %d nodeIdx %d\n", xResult.m_max, xResult.m_maxNodeIdx);
            doLog(LOG_VERBOSE, "ymin: %d nodeIdx %d\n", yResult.m_min, yResult.m_minNodeIdx);
            doLog(LOG_VERBOSE, "ymax: %d nodeIdx %d\n", yResult.m_max, yResult.m_maxNodeIdx);
#endif
            // connect the extreme nodes together
            updateWeights<ChipDB::XAxisAccessor>(XSolverData,
                netlist, 
                xResult.m_minNodeIdx, xResult.m_maxNodeIdx,
                net.m_weight, net.m_nodes.size());

            updateWeights<ChipDB::YAxisAccessor>(YSolverData,
                netlist, 
                yResult.m_minNodeIdx, yResult.m_maxNodeIdx,
                net.m_weight, net.m_nodes.size());

            // connect each internal node to the min node
            for(auto const nodeIdx : net.m_nodes)
            {
                if ((nodeIdx != xResult.m_minNodeIdx) && (nodeIdx != xResult.m_maxNodeIdx))
                {
                    updateWeights<ChipDB::XAxisAccessor>(XSolverData,
                        netlist, 
                        xResult.m_minNodeIdx, nodeIdx,
                        net.m_weight, net.m_nodes.size());

                    updateWeights<ChipDB::XAxisAccessor>(XSolverData,
                        netlist, 
                        xResult.m_maxNodeIdx, nodeIdx,
                        net.m_weight, net.m_nodes.size());
                }

                if ((nodeIdx != yResult.m_minNodeIdx) && (nodeIdx != yResult.m_maxNodeIdx))
                {
                    updateWeights<ChipDB::YAxisAccessor>(YSolverData,
                        netlist, 
                        yResult.m_minNodeIdx, nodeIdx,
                        net.m_weight, net.m_nodes.size());

                    updateWeights<ChipDB::YAxisAccessor>(YSolverData,
                        netlist, 
                        yResult.m_maxNodeIdx, nodeIdx,
                        net.m_weight, net.m_nodes.size());
                }
            }
        }
        else
        {
            degenerateNets++;
        }

        netIdx++;
    }

    doLog(LOG_VERBOSE,"  Number of degenerate nets: %ld\n", degenerateNets);

#if 0
    std::stringstream ss;
    ss << "X Amat: \n" << XSolverData.m_Amat << "\n";
    ss << "X Bvec: \n" << XSolverData.m_Bvec << "\n";

    ss << "Y Amat: \n" << YSolverData.m_Amat << "\n";    
    ss << "Y Bvec: \n" << YSolverData.m_Bvec << "\n";

    doLog(LOG_VERBOSE,"  Solver matrices: \n");
    doLog(LOG_VERBOSE, ss);

    doLog(LOG_INFO, "Number of degenerate nets: %ld\n", degenerateNets);

    doLog(LOG_INFO, "Running solver\n");
#endif

    Eigen::ConjugateGradient<Eigen::SparseMatrix<double>, Eigen::Upper | Eigen::Lower> solver;

    XSolverData.m_Amat.makeCompressed();
    solver.compute(XSolverData.m_Amat);
    Eigen::MatrixXd xpos = solver.solve(XSolverData.m_Bvec);
    auto xpos_err  = solver.error();
    auto xpos_info = solver.info();

    if (xpos_info != Eigen::ComputationInfo::Success)
    {
        doLog(LOG_WARN,"  Eigen reports error code %s for x axis\n", toString(xpos_info).c_str());
    }

    //std::cout << XSolverData.m_Amat <<  "\n";
    //std::cout << XSolverData.m_Bvec <<  "\n";
    //std::cout << xpos << "\n";

    YSolverData.m_Amat.makeCompressed();
    solver.compute(YSolverData.m_Amat);
    Eigen::MatrixXd  ypos = solver.solve(YSolverData.m_Bvec);
    auto ypos_err  = solver.error();
    auto ypos_info = solver.info();

    if (ypos_info != Eigen::ComputationInfo::Success)
    {
        doLog(LOG_WARN,"  Eigen reports error code %s for y axis\n", toString(xpos_info).c_str());
    }

    ssize_t fixedNodes = 0;
    ssize_t idx = 0;
    for(auto & node : netlist.m_nodes)
    {
        if (!node.isFixed())
        {
            node.m_pos.m_x = static_cast<ChipDB::CoordType>(xpos(idx));
            node.m_pos.m_y = static_cast<ChipDB::CoordType>(ypos(idx));

            //doLog(LOG_VERBOSE, "  Node %d pos %d,%d\n", idx, node.m_pos.m_x, node.m_pos.m_y);
        }
        else
        {
            //doLog(LOG_VERBOSE, "  Node %d pos %d,%d (fixed, but solver says %f, %f)\n", idx, node.m_pos.m_x, node.m_pos.m_y,
            //    xpos(idx), ypos(idx));
            fixedNodes++;
        }
        idx++;
    }

    doLog(LOG_INFO, "Number of fixed nodes: %ld\n", fixedNodes);

    return true;
}

double LunaCore::QLAPlacer::Private::calcHPWL(const LunaCore::QPlacer::PlacerNetlist &netlist)
{
    // get the extreme coordinates for each net
    double hpwl = 0.0f;
    for(auto const &net : netlist.m_nets)
    {
        ChipDB::CoordType xmin = std::numeric_limits<ChipDB::CoordType>::max();
        ChipDB::CoordType xmax = std::numeric_limits<ChipDB::CoordType>::min();
        ChipDB::CoordType ymin = std::numeric_limits<ChipDB::CoordType>::max();
        ChipDB::CoordType ymax = std::numeric_limits<ChipDB::CoordType>::min();

        for(auto const nodeId : net.m_nodes)
        {
            auto nodePos = netlist.m_nodes.at(nodeId).m_pos;
            xmin = std::min(nodePos.m_x, xmin);
            xmax = std::max(nodePos.m_x, xmax);
            ymin = std::min(nodePos.m_y, ymin);
            ymax = std::max(nodePos.m_y, ymax);
        }

        hpwl += (xmax - xmin) + (ymax-ymin);
    }

    return hpwl;
}
