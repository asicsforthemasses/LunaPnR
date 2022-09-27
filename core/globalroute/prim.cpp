// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <memory>
#include <sstream>
#include "prim.h"
#include "prim_private.h"

using namespace LunaCore::Prim;
using namespace LunaCore::Prim::Private;

void LunaCore::Prim::MSTreeNode::addEdge(NodeId child, const ChipDB::Coord64 &childPos)
{
    auto &edge = m_edges.emplace_back();
    edge.m_pos = childPos;
    edge.m_self = child;
}

MSTree LunaCore::Prim::prim(const std::vector<ChipDB::Coord64> &netNodes)
{
    Prim::Private::PriorityQueue pqueue;
    for(NodeId idx = 1; idx < netNodes.size(); ++idx)
    {
        auto newEdge = new Prim::Private::TreeEdge;
        newEdge->from = 0;
        newEdge->to   = idx;
        newEdge->m_edgeCost = calcCost(netNodes.at(0), netNodes.at(idx));
        pqueue.push(newEdge);
    }

    MSTree tree;
    tree.resize(netNodes.size());
    tree.at(0).m_parent = MSTreeNode::c_NoParent;

    while(pqueue.empty())
    {
        std::unique_ptr<TreeEdge> minEdge;
        minEdge.reset(pqueue.top());      // take ownership of TreeEdge ptr

        pqueue.pop();
        auto currentNodeId = minEdge->to;

        for(NodeId idx=0; idx < netNodes.size(); idx++)
        {
            auto nextNodeId = idx;

            if (tree.at(idx).hasParent()) continue;

            if (nextNodeId == currentNodeId)
            {
                // the head node has from == -1,
                // and this will cause an exception if we don't 
                // check for it. 
                if (minEdge->from >= 0)
                {
                    tree.at(minEdge->from).addEdge(minEdge->to, netNodes.at(minEdge->to));
                    //auto &edge = tree.at(minEdge->from).addEdge(minEdge->m_dest, netNodes.at(minEdge->m_dest));                    
                    //edge.m_src  = minEdge->from;
                    //edge.m_dest = minEdge->to;
                }
                if (minEdge->to != 0) tree.at(minEdge->to).m_parent = minEdge->from;
            }
            else
            {
                auto newEdge = new TreeEdge;
                newEdge->from = currentNodeId;
                newEdge->to   = nextNodeId;
                auto const& currentNodePos = netNodes.at(currentNodeId);
                auto const& nextNodePos = netNodes.at(nextNodeId);
                newEdge->m_edgeCost = calcCost(currentNodePos, nextNodePos);
                pqueue.push(newEdge);
            }
        }        
    }


    return std::move(tree);
}

std::vector<ChipDB::Coord64> LunaCore::Prim::loadNetNodes(const std::string &src)
{
    std::vector<ChipDB::Coord64> nodes;
    std::stringstream ss(src);

    int Npoints = 0;
    ss >> Npoints;

    if (Npoints == 0) return {};

    nodes.reserve(Npoints);

    for(int p=0; p<Npoints; p++)
    {
        int64_t x,y;
        ss >> x;
        ss >> y;

        nodes.emplace_back(ChipDB::Coord64{x,y});
    }    

    return std::move(nodes);
}
