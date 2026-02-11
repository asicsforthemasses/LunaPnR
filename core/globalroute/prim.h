// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <limits>
#include <iostream>
#include "database/database.h"

namespace LunaCore::Prim
{

using NodeId = int32_t;

struct MSTreeEdge
{
    ChipDB::Coord64 m_pos;
    NodeId          m_self; ///< netNode index of this end point
};

struct MSTreeNode
{
    static constexpr NodeId c_NoParent{-1};

    NodeId     m_parent;   ///< netNode index of the parent
    NodeId     m_self;     ///< netNode index of this starting point

    ChipDB::Coord64 m_pos; ///< own position

    std::vector<MSTreeEdge> m_edges;

    [[nodiscard]] bool hasParent() const noexcept
    {
        return m_parent != c_NoParent;
    }

    void addEdge(NodeId child, const ChipDB::Coord64 &childPos);

    bool operator==(const MSTreeNode &rhs) const noexcept;
};

using MSTree = std::vector<MSTreeNode>;

/** construct a separable minimum spanning tree from a number of net terminals
    based on: New Algorithms for the Rectilinear Steiner Tree
    Problem", IEEE TRANSACTIONS ON COMPUTER-AIDED DESIGN, VOL 9 NO 2, FEBRUARY 1990.
*/
[[nodiscard]] MSTree prim(const std::vector<ChipDB::Coord64> &netNodes);

/** utility function for testing. Loads net node positions from a string.
    Format: <number of nodes> <x1> <y1> <x2> <y2> ... <x_N> <y_N>
*/
std::vector<ChipDB::Coord64> loadNetNodes(const std::string &src);

void toSVG(std::ostream &os, const MSTree &tree);

};

/** write MSTreeNode to ostream. Format: <parent id> <x> <y> <num edges> <child1 id> <child1 x> child1 y> ... */
inline std::ostream& operator<<(std::ostream &os, const LunaCore::Prim::MSTreeNode &node)
{
    os << node.m_self;
    os << " " << node.m_parent;
    os << " " << node.m_pos.m_x;
    os << " " << node.m_pos.m_y;
    os << " " << node.m_edges.size();
    for(auto const& edge : node.m_edges)
    {
        os << " " << edge.m_self << "  " << edge.m_pos.m_x << " " << edge.m_pos.m_y;
    }
    return os;
}

/** read MSTreeNode from ostream. Format: <parent id> <x> <y> <num edges> <child1 id> <child1 x> child1 y> ... */
inline std::istream& operator>>(std::istream &is, LunaCore::Prim::MSTreeNode &node)
{
    is >> node.m_self;
    is >> node.m_parent;
    is >> node.m_pos.m_x;
    is >> node.m_pos.m_y;
    std::size_t numEdges;
    is >> numEdges;
    node.m_edges.resize(numEdges);
    for(auto &edge : node.m_edges)
    {
        is >> edge.m_self;
        is >> edge.m_pos.m_x;
        is >> edge.m_pos.m_y;
    }
    return is;
}

/** write MSTree to ostream. */
inline std::ostream& operator<<(std::ostream &os, const LunaCore::Prim::MSTree &tree)
{
    os << tree.size() << "\n";

    for(auto const& node : tree)
    {
        os << node << "\n";
    }
    return os;
}

/** read MSTree from istream. */
inline std::istream& operator>>(std::istream &is, LunaCore::Prim::MSTree &tree)
{
    tree.clear();

    std::size_t numNodes;
    is >> numNodes;

    tree.resize(numNodes);

    for(auto& node : tree)
    {
        is >> node;
    }
    return is;
}
