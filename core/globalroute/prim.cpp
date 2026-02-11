// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <memory>
#include <sstream>
#include "prim.h"
#include "prim_private.h"
#include "tinysvgpp.h"

using namespace LunaCore::Prim;
using namespace LunaCore::Prim::Private;

bool LunaCore::Prim::MSTreeNode::operator==(const MSTreeNode &rhs) const noexcept
{
    if (m_pos != rhs.m_pos) return false;
    if (m_parent != rhs.m_parent) return false;
    if (m_self != rhs.m_self) return false;

    if (m_edges.size() != rhs.m_edges.size()) return false;
    for(std::size_t idx=0; idx < m_edges.size(); idx++)
    {
        if (m_edges.at(idx).m_pos != rhs.m_edges.at(idx).m_pos) return false;
        if (m_edges.at(idx).m_self != rhs.m_edges.at(idx).m_self) return false;
    }

    return true;
}

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

    std::size_t idx = 0;
    for(auto &treeNode : tree)
    {
        treeNode.m_parent = MSTreeNode::c_NoParent;
        treeNode.m_self = idx;
        treeNode.m_pos  = netNodes.at(idx);
        idx++;
    }

    tree.at(0).m_parent = 0;

    while(!pqueue.empty())
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

        auto &newNode = nodes.emplace_back();
        newNode.m_x = x;
        newNode.m_y = y;
    }

    return std::move(nodes);
}

TinySVGPP::Point toSVGPoint(const ChipDB::Coord64 &p)
{
    return {static_cast<float>(p.m_x), static_cast<float>(p.m_y)};
}

void LunaCore::Prim::toSVG(std::ostream &os, const MSTree &tree)
{
    TinySVGPP::Canvas canvas;
    using SVGPoint = TinySVGPP::Point;

    // determine the node extents
    TinySVGPP::Viewport vp;
    vp.flipY = true;
    vp.xmin = std::numeric_limits<float>::max();
    vp.xmax = std::numeric_limits<float>::min();
    vp.ymin = std::numeric_limits<float>::max();
    vp.ymax = std::numeric_limits<float>::min();
    for(auto const& node : tree)
    {
        vp.xmin = std::min((float)node.m_pos.m_x, vp.xmin);
        vp.xmax = std::max((float)node.m_pos.m_x, vp.xmax);
        vp.ymin = std::min((float)node.m_pos.m_y, vp.ymin);
        vp.ymax = std::max((float)node.m_pos.m_y, vp.ymax);
    }

    auto border = std::max(vp.width(), vp.height())/10;

    vp.xmin -= border * 0.5;
    vp.xmax += border * 0.5;
    vp.ymin -= border * 0.5;
    vp.ymax += border * 0.5;

    auto aspectScale = static_cast<float>(vp.height())/static_cast<float>(vp.width());
    std::cout << "Aspect scale: " << aspectScale << "\n";
    vp.canvasWidth = 2000;
    vp.canvasHeight = std::ceil(vp.canvasWidth*aspectScale);

    canvas.setSize(vp.canvasWidth, vp.canvasHeight);
    canvas.fill("black").stroke("black").rect(0,0, vp.canvasWidth, vp.canvasHeight);

    // draw all edges
    canvas.stroke("green", 4.0).fill("green");
    for(auto const& treeNode : tree)
    {
        auto p1 = treeNode.m_pos;
        for(auto const& treeEdge : treeNode.m_edges)
        {
            auto p2 = treeEdge.m_pos;
            //auto steinerPoint = LunaCore::LSteinerPoint(p1,p2,treeEdge.m_shape);
#if 0
            if (steinerPoint)
            {
                canvas.line(
                    SVGPoint{vp.toWindow(SVGPoint{p1.m_x, p1.m_y})},
                    SVGPoint{vp.toWindow(SVGPoint{steinerPoint->m_x, steinerPoint->m_y})}
                );
                canvas.line(
                    SVGPoint{vp.toWindow(SVGPoint{steinerPoint->m_x, steinerPoint->m_y})},
                    SVGPoint{vp.toWindow(SVGPoint{p2.m_x, p2.m_y})}
                );
            }
            else
            {
                canvas.line(
                    SVGPoint{vp.toWindow(SVGPoint{p1.m_x, p1.m_y})},
                    SVGPoint{vp.toWindow(SVGPoint{p2.m_x, p2.m_y})}
                );
            }
#else
            canvas.line(
                SVGPoint{vp.toWindow(toSVGPoint(p1))},
                SVGPoint{vp.toWindow(toSVGPoint(p2))}
            );
#endif
        }
    }

    // draw bounding boxes
    canvas.stroke("white", 2.0).fill("none").opacity(0.5);
    for(auto const& treeNode : tree)
    {
        auto p1 = treeNode.m_pos;
        for(auto const& treeEdge : treeNode.m_edges)
        {
            auto p2 = treeEdge.m_pos;

            if ((p1.m_x != p2.m_x) && (p1.m_y != p2.m_x))
            {
                canvas.rect(
                    SVGPoint{vp.toWindow(toSVGPoint(p1))},
                    SVGPoint{vp.toWindow(toSVGPoint(p2))}
                );
            }
        }
    }

    // draw all the nodes
    canvas.stroke("orange").fill("orange").opacity(1.0);
    for(auto const& node : tree)
    {
        auto p = vp.toWindow(toSVGPoint(node.m_pos));
        if (node.m_self == 0)
            canvas.stroke("red").circle(p, 6.0).stroke("orange");
        else
            canvas.circle(p, 2.0);

        std::stringstream ss;
        ss << node.m_self;
        canvas.text(p + SVGPoint{5.0,-5.0}, ss.str());
    }

    canvas.toSVG(os);
}
