#include <random>
#include <deque>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/IterativeLinearSolvers>

#include "common/logging.h"
#include "qlaplacer.h"
#include "export/svg/svgwriter.h"

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
    for(auto * ins : nl.m_instances)
    {
        auto nodeId = netlist.createNode();
        auto& placerNode = netlist.getNode(nodeId);
        placerNode.m_type = LunaCore::QPlacer::PlacerNodeType::MovableNode;
        placerNode.m_size = ins->instanceSize();
        placerNode.m_pos  = ins->m_pos;
        placerNode.m_weight = 1.0;
        
        if (ins->m_placementInfo == ChipDB::PlacementInfo::PLACEDANDFIXED)
        {
            placerNode.m_type = LunaCore::QPlacer::PlacerNodeType::FixedNode;
        }

        ins2nodeId[ins] = nodeId;
    }

    // create placer nets
    ssize_t netIdx = 0;
    for(auto net : nl.m_nets)
    {
        auto placerNetId = netlist.createNet();
        auto& placerNet = netlist.getNet(placerNetId);
        placerNet.m_weight = 1.0;

        doLog(LOG_VERBOSE, "NetId %d - Net name %s\n", netIdx, net->m_name.c_str());

        for(auto& conn : net->m_connections)
        {
            auto ins = conn.m_instance;

            if (ins == nullptr)
            {
                continue;
            }

            auto iter = ins2nodeId.find(ins);

            if (iter == ins2nodeId.end())
            {
                doLog(LOG_ERROR, "createPlacerNetlist: cannot find node\n");
                return LunaCore::QPlacer::PlacerNetlist();
            }

            auto placerNodeId = iter->second;
            placerNet.m_nodes.push_back(placerNodeId);
            netlist.getNode(placerNodeId).m_connections.push_back(placerNetId);

            // FIXME:
            if (ins->m_placementInfo == ChipDB::PlacementInfo::PLACEDANDFIXED) 
            {
                placerNet.m_weight = 1;
            }

            doLog(LOG_VERBOSE, "  NodeId %d %s\n", placerNodeId, ins->m_name.c_str());
        }    

        netIdx++;    
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

    double weight = netWeight/(static_cast<double>(netSize-1) * static_cast<double>(distance));

    double fixedWeight = netWeight/static_cast<double>(netSize-1);

    // update the solver matrix according to the node mobility
    // e.g. if both nodes are fixed, don't add anything!

    if (node1.isFixed() && node2.isFixed())
    {
        solverData.m_Amat.coeffRef(node1Id, node1Id)  = 1.0f; // attempt to make life of solver easier
        solverData.m_Amat.coeffRef(node2Id, node2Id)  = 1.0f; // attempt to make life of solver easier
        return;
    }

    if (node1.isFixed() || node2.isFixed())
    {
        // one of the two nodes is fixed
        if (node1.isFixed())
        {
            solverData.m_Amat.coeffRef(node2Id, node2Id)  += fixedWeight;
            solverData.m_Amat.coeffRef(node1Id, node1Id)  = 1.0f; // attempt to make life of solver easier
            solverData.m_Bvec[node2Id] = fixedWeight * AxisAccessor::get(node1.m_pos);

            if (node2Id == 20)
            {
                doLog(LOG_VERBOSE,"Node20: axis=%s anchor pos=%d w=%f (fixed -> node %d)\n", AxisAccessor::m_name, AxisAccessor::get(node1.m_pos), fixedWeight, node1Id);
            }
        }
        else
        {
            solverData.m_Amat.coeffRef(node1Id, node1Id)  += fixedWeight;
            solverData.m_Amat.coeffRef(node2Id, node2Id)  = 1.0f; // attempt to make life of solver easier
            solverData.m_Bvec[node1Id] = fixedWeight * AxisAccessor::get(node2.m_pos);

            if (node1Id == 20)
            {
                doLog(LOG_VERBOSE,"Node20: axis=%s anchor pos=%d w=%f (fixed -> node %d)\n", AxisAccessor::m_name, AxisAccessor::get(node2.m_pos), fixedWeight, node2Id);
            }            
        }
    }
    else
    {
        // both nodes are movable
        solverData.m_Amat.coeffRef(node1Id, node1Id)  += weight;
        solverData.m_Amat.coeffRef(node2Id, node2Id)  += weight;
        solverData.m_Amat.coeffRef(node1Id, node2Id)  -= weight;
        solverData.m_Amat.coeffRef(node2Id, node1Id)  -= weight;

        if (node1Id == 20)
        {
            doLog(LOG_VERBOSE,"Node20: axis=%s dst pos=%d w=%f (movable -> node %d)\n", AxisAccessor::m_name, AxisAccessor::get(node2.m_pos), fixedWeight, node2Id);
        }            
        if (node2Id == 20)
        {
            doLog(LOG_VERBOSE,"Node20: axis=%s dst pos=%d w=%f (movable -> node %d)\n", AxisAccessor::m_name, AxisAccessor::get(node1.m_pos), fixedWeight, node1Id);
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
            if (netIdx == 55)
            {
                doLog(LOG_VERBOSE, "xmin: %d nodeIdx %d\n", xResult.m_min, xResult.m_minNodeIdx);
                doLog(LOG_VERBOSE, "xmax: %d nodeIdx %d\n", xResult.m_max, xResult.m_maxNodeIdx);
                doLog(LOG_VERBOSE, "ymin: %d nodeIdx %d\n", yResult.m_min, yResult.m_minNodeIdx);
                doLog(LOG_VERBOSE, "ymax: %d nodeIdx %d\n", yResult.m_max, yResult.m_maxNodeIdx);
            }
#endif

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
    Eigen::VectorXd xpos = solver.solve(XSolverData.m_Bvec);
    //auto xpos_err  = solver.error();
    auto xpos_info = solver.info();

    if (xpos_info != Eigen::ComputationInfo::Success)
    {
        doLog(LOG_WARN,"  Eigen reports error code %s for x axis\n", toString(xpos_info).c_str());
    }

    YSolverData.m_Amat.makeCompressed();
    solver.compute(YSolverData.m_Amat);
    Eigen::VectorXd  ypos = solver.solve(YSolverData.m_Bvec);
    //auto ypos_err  = solver.error();
    auto ypos_info = solver.info();

    if (ypos_info != Eigen::ComputationInfo::Success)
    {
        doLog(LOG_WARN,"  Eigen reports error code %s for y axis\n", toString(ypos_info).c_str());
    }

    ssize_t fixedNodes = 0;
    ssize_t idx = 0;
    for(auto & node : netlist.m_nodes)
    {
        if (!node.isFixed())
        {
            node.m_pos.m_x = static_cast<ChipDB::CoordType>(xpos(idx));
            node.m_pos.m_y = static_cast<ChipDB::CoordType>(ypos(idx));
        }
        else
        {
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
    ssize_t netId = 0;
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

        auto netHPWL = (xmax - xmin) + (ymax-ymin);

        hpwl += netHPWL;
        netId++;
    }

    return hpwl;
}

void LunaCore::QLAPlacer::Private::writeNetlistToSVG(std::ostream &os, 
    const ChipDB::Rect64 &regionRect,
    const LunaCore::QPlacer::PlacerNetlist &netlist)
{
    LunaCore::SVG::Writer svg(os, 2000,2000);

    svg.setFillColour(0xFF);
    svg.setStrokeWidth(0);
    svg.drawRectangle({0,0},{2001,2001});
    svg.setFontSize(16);

    svg.setViewport(regionRect.expanded(ChipDB::Margins64{100,100,100,100}));
    
    //draw nodes
    
    svg.setStrokeWidth(5);
    size_t nodeIdx = 0;
    for(auto const &node : netlist.m_nodes)
    {
        std::stringstream ss;
        ss << nodeIdx;
        if (node.isFixed())
        {
            svg.setStrokeColour(0xFF000080);    // red transparent
        }
        else
        {
            svg.setStrokeColour(0x00FF0080);    // green transparent
        }
        svg.drawCircle(node.m_pos, 20);
        svg.setStrokeColour(0xFFFFFFFF);    // white
        svg.drawCenteredText(node.m_pos, ss.str());
        nodeIdx++;
    }

    // draw nets
    svg.setStrokeColour(0xFFFFFF80);    // white transparent
    svg.setStrokeWidth(3);
    for(auto const &net : netlist.m_nets)
    {
        if (net.m_nodes.size() < 2)
        {
            continue;
        }

        auto firstNode = netlist.m_nodes.at(net.m_nodes.front());
        for(size_t idx=1; idx<net.m_nodes.size(); idx++)
        {
            auto const& secondNode = netlist.m_nodes.at(net.m_nodes.at(idx));
            svg.drawLine(firstNode.m_pos, secondNode.m_pos);
        }
    }
}


void LunaCore::QLAPlacer::Private::lookaheadLegaliser(const ChipDB::Rect64 &regionRect, LunaCore::QPlacer::PlacerNetlist &netlist)
{
    // FIXME: these constants should come from the cell library.

    const ChipDB::CoordType blockMinHeight = 20000; 
    const ChipDB::CoordType blockMinWidth  = 4*blockMinHeight;

    std::deque<Block> blockQueue;
    blockQueue.push_back(Block{.m_extents = regionRect, .m_level = 0});

    while(!blockQueue.empty())
    {
        auto block = blockQueue.front();
        blockQueue.pop_front();

        if ((block.m_extents.width() <= blockMinWidth) || (block.m_extents.height() <= blockMinHeight))
        {
            // if we cannot sub-divide any more, skip the block
            continue;
        }

        // collect all the cells in the block

        std::vector<LunaCore::QPlacer::PlacerNodeId> movableNodesInBlock;

        QPlacer::PlacerNodeId idx = 0;
        for(auto const &node : netlist.m_nodes)
        {
            if ((!node.isFixed()) && block.m_extents.contains(node.m_pos))
            {
                movableNodesInBlock.push_back(idx);
            }
            idx++;
        }

        // calculate total area
        double totalCellArea = 0;
        for(auto const &nodeId : movableNodesInBlock)
        {
            totalCellArea += static_cast<double>(netlist.m_nodes.at(nodeId).m_size.m_x) 
                * static_cast<double>(netlist.m_nodes.at(nodeId).m_size.m_y);
        }

        if (block.m_level % 2 == 0)
        {
            // horizontal split
            // sort movable nodes in x direction
            std::sort(movableNodesInBlock.begin(), movableNodesInBlock.end(), [&](const QPlacer::PlacerNodeId &n1, const QPlacer::PlacerNodeId &n2)
                {
                    return netlist.m_nodes.at(n1).m_pos.m_x < netlist.m_nodes.at(n2).m_pos.m_x;
                }
            );

            // divide the block into two part with equal white space
            // FIXME: for now we assume no blockages
            Block SubBlockleft  = {.m_extents = block.m_extents, .m_level = block.m_level + 1};
            Block SubBlockright = {.m_extents = block.m_extents, .m_level = block.m_level + 1};

            double leftBlockArea = static_cast<double>(SubBlockleft.m_extents.width()) 
                * static_cast<double>(SubBlockleft.m_extents.height());

            auto cutPos = block.m_extents.left() + block.m_extents.width()/2;

            SubBlockleft.m_extents.setUR({cutPos, block.m_extents.top()});
            SubBlockright.m_extents.setLL({cutPos, block.m_extents.bottom()});

            doNonlinearScaling(SubBlockleft, netlist);
            doNonlinearScaling(SubBlockright, netlist);

            blockQueue.push_back(SubBlockleft);
            blockQueue.push_back(SubBlockright);
        }
        else
        {
            // vertical split
            // sort movable nodes in y direction
            std::sort(movableNodesInBlock.begin(), movableNodesInBlock.end(), [&](const QPlacer::PlacerNodeId &n1, const QPlacer::PlacerNodeId &n2)
                {
                    return netlist.m_nodes.at(n1).m_pos.m_y < netlist.m_nodes.at(n2).m_pos.m_y;
                }
            );          

            // divide the block into two part with equal white space
            // FIXME: for now we assume no blockages
            Block SubBlocktop     = {.m_extents = block.m_extents, .m_level = block.m_level + 1};
            Block SubBlockbottom  = {.m_extents = block.m_extents, .m_level = block.m_level + 1};

            auto cutPos = block.m_extents.bottom() + block.m_extents.height()/2;

            SubBlocktop.m_extents.setLL({block.m_extents.left(), cutPos});
            SubBlockbottom.m_extents.setUR({block.m_extents.right(), cutPos});

            doNonlinearScaling(SubBlocktop, netlist);
            doNonlinearScaling(SubBlockbottom, netlist);

            blockQueue.push_back(SubBlocktop);
            blockQueue.push_back(SubBlockbottom);
        }
    }
}

void LunaCore::QLAPlacer::Private::doNonlinearScaling(const Block &block, LunaCore::QPlacer::PlacerNetlist &netlist)
{

}
