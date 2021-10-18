#include <cassert>
#include "qplacer.h"

using namespace LunaCore::QPlacer;

void Placer::checkAndSetUnmovableNet(PlacerNet &net, const std::vector<PlacerNode> &nodes)
{
    // check that at least one node is movable
    ssize_t movable = 0;
    for(auto const netNodeId : net.m_nodes)
    {
        if (!nodes.at(netNodeId).isFixed())
        {
            movable++;
            break;
        }
    }

    if (movable == 0)
    {
        // ignore the net when no nodes are movable
        net.m_type = LunaCore::QPlacer::PlacerNetType::Ignore;
    }
}

void Placer::addStarNodes(std::vector<PlacerNode> &nodes, std::vector<PlacerNet> &nets)
{
    // add a star node to every net with more than 2 nodes
    ssize_t netId = 0;
    for(auto& net : nets)
    {
        checkAndSetUnmovableNet(net, nodes);
        
        if (net.m_type != LunaCore::QPlacer::PlacerNetType::Ignore)
        {
            if (net.m_nodes.size() > 2)
            {
                // create a star node
                nodes.emplace_back();
                auto &starNode  = nodes.back();
                starNode.m_type = PlacerNodeType::StarNode;
                starNode.m_connections.push_back(netId);

                // add the star node to the net
                // it should always be the last node
                PlacerNodeId starNodeId = nodes.size()-1;
                net.m_nodes.push_back(starNodeId);

                net.m_type = LunaCore::QPlacer::PlacerNetType::StarNet;
            }
            else if (net.m_nodes.size() == 2)
            {
                net.m_type = LunaCore::QPlacer::PlacerNetType::TwoNet;
            }
        }
        netId++;
    }
}

void Placer::solve(std::vector<PlacerNode> &nodes, std::vector<PlacerNet> &nets,
    Eigen::VectorXd &xpos, Eigen::VectorXd &ypos)
{
    addStarNodes(nodes, nets);

    buildEquations<XAxisAccessor>(nodes, nets, m_Amat_x, m_Bvec_x);
    buildEquations<YAxisAccessor>(nodes, nets, m_Amat_y, m_Bvec_y);

    xpos.resize(nodes.size());
    ypos.resize(nodes.size());

    Eigen::ConjugateGradient<Eigen::SparseMatrix<double>, Eigen::Upper | Eigen::Lower> solver;
    xpos = solver.compute(m_Amat_x).solve(m_Bvec_x);
    ypos = solver.compute(m_Amat_y).solve(m_Bvec_y);
}

std::string LunaCore::QPlacer::toString(const PlacerNodeType &t)
{
    switch(t)
    {
    case PlacerNodeType::MovableNode:
        return "MovableNode";
    case PlacerNodeType::FixedNode:
        return "FixedNode";
    case PlacerNodeType::StarNode:
        return "StarNode";
    default:
        return "Undefined";
    }
}


std::string toString(const PlacerNetType &t)
{
    switch(t)
    {
    case PlacerNetType::Ignore:
        return "Ignore";
    case PlacerNetType::StarNet:
        return "StarNet";
    case PlacerNetType::TwoNet:
        return "TwoNet";
    
    default:
    case PlacerNetType::Undefined:
        return "Undefined";
    }
}
