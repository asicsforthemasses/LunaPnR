// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <limits>
#include "common/dbtypes.h"

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

    std::vector<MSTreeEdge> m_edges;

    [[nodiscard]] bool hasParent() const noexcept
    {
        return m_parent != c_NoParent;
    }

    void addEdge(NodeId child, const ChipDB::Coord64 &childPos);
};

using MSTree = std::vector<MSTreeNode>;

[[nodiscard]] MSTree prim(const std::vector<ChipDB::Coord64> &netNodes);

};