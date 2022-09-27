// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <queue>
#include "common/dbtypes.h"
#include "prim.h"

namespace LunaCore::Prim::Private
{

/** Calculate cost based on 3-tuple from "New Algorithms for the Rectilinear Steiner Tree
Problem", IEEE TRANSACTIONS ON COMPUTER-AIDED DESIGN, VOL 9 NO 2, FEBRUARY 1990 */
struct CostTuple
{
    ChipDB::CoordType m_dist;
    ChipDB::CoordType m_minAbsY;
    ChipDB::CoordType m_minMaxX;
};

[[nodiscard]] bool operator<(const CostTuple &lhs, const CostTuple &rhs) noexcept;

struct TreeEdge
{
    static constexpr NodeId c_Undefined = -1;
    NodeId from{c_Undefined};
    NodeId to{c_Undefined};
    CostTuple m_edgeCost;
};

struct TreeEdgeCompare
{
    [[nodiscard]] bool operator()(const TreeEdge* lhs, const TreeEdge* rhs) const noexcept
    {
        return !(lhs->m_edgeCost < rhs->m_edgeCost);
    }
};

using PriorityQueue = std::priority_queue<TreeEdge*, std::vector<TreeEdge*>, TreeEdgeCompare>;

inline CostTuple calcCost(const ChipDB::Coord64 &node1, const ChipDB::Coord64 &node2) noexcept
{
    CostTuple t;
    t.m_dist = std::abs(node1.m_x - node2.m_x) + std::abs(node1.m_y - node2.m_y);
    t.m_minAbsY = -std::abs(node1.m_y - node2.m_y);
    t.m_minMaxX = -std::max(node1.m_x, node2.m_x);
    return t;
};

};