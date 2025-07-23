// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <vector>
#include <cstdlib>
#include "database/database.h"

/** analytical placer namespace */
namespace LunaCore::QPlacer
{

using PlacerNodeId = ssize_t;
using PlacerNetId  = ssize_t;

enum class PlacerNodeType
{
    Undefined,
    MovableNode,
    FixedNode,
    StarNode
};

enum class PlacerNetType
{
    Undefined,
    StarNet,
    TwoNet,
    Ignore
};

class PlacerNode
{
public:

    PlacerNode() : m_type(PlacerNodeType::Undefined), m_weight(1.0f), m_size{0,0} {}

    PlacerNodeType  m_type;     ///< node type
    float           m_weight;   ///< default = 1.0, and 10.0 for fixed pins

    constexpr bool isFixed() const noexcept
    {
        return m_type == PlacerNodeType::FixedNode;
    }

    void dump(std::ostream &os) const
    {
        os << "Nets: ";
        for(auto const& netId : m_connections)
        {
            os << " " << netId;
        }
        os << "  LLPos: " << m_pos << "\n";
    }

    std::vector<PlacerNetId> m_connections;

    /** get cell center position */
    constexpr ChipDB::Coord64 getCenterPos() const noexcept
    {
        return ChipDB::Coord64{m_pos.m_x + m_size.m_x/2, m_pos.m_y + m_size.m_y/2};
    }

    /** set x position of lower left corner of cell */
    constexpr void setLLX(ChipDB::CoordType x) noexcept
    {
        m_pos.m_x = x;
    }

    /** set y position of upper left corner of cell */
    constexpr void setLLY(ChipDB::CoordType y) noexcept
    {
        m_pos.m_y = y;
    }

    /** get lower left cell position */
    constexpr ChipDB::Coord64 getLLPos() const noexcept
    {
        return ChipDB::Coord64{m_pos.m_x, m_pos.m_y};
    }

    constexpr ChipDB::Coord64 getURPos() const noexcept
    {
        return ChipDB::Coord64{m_pos.m_x + m_size.m_x, m_pos.m_y  + m_size.m_y};
    }

    constexpr ChipDB::CoordType left() const noexcept
    {
        return m_pos.m_x;
    }

    constexpr ChipDB::CoordType bottom() const noexcept
    {
        return m_pos.m_y;
    }

    constexpr ChipDB::CoordType right() const noexcept
    {
        return m_pos.m_x + m_size.m_x;
    }

    constexpr ChipDB::CoordType top() const noexcept
    {
        return m_pos.m_y + m_size.m_y;
    }

    constexpr ChipDB::CoordType width() const noexcept
    {
        return m_size.m_x;
    }

    constexpr ChipDB::CoordType height() const noexcept
    {
        return m_size.m_y;
    }

    constexpr void setLLPos(const ChipDB::Coord64 &pos) noexcept
    {
        m_pos = pos;
    }

    constexpr void setCenterPos(const ChipDB::Coord64 &pos) noexcept
    {
        m_pos = ChipDB::Coord64{pos.m_x - m_size.m_x/2, pos.m_y - m_size.m_y/2};
    }

    constexpr void setCenterX(const ChipDB::CoordType x) noexcept
    {
        m_pos.m_x = x - m_size.m_x/2;
    }

    constexpr void setCenterY(const ChipDB::CoordType y) noexcept
    {
        m_pos.m_y = y - m_size.m_y/2;
    }

    constexpr ChipDB::CoordType getCenterX() const noexcept
    {
        return m_pos.m_x + m_size.m_x/2;
    }

    constexpr ChipDB::CoordType getCenterY() const noexcept
    {
        return m_pos.m_y + m_size.m_y/2;
    }

    constexpr void setSize(const ChipDB::Coord64 &size) noexcept
    {
        m_size = size;
    }

protected:
    ChipDB::Coord64 m_pos;      ///< center position of node in nm
    ChipDB::Coord64 m_size;     ///< size in nm
};

struct PlacerNet
{
    PlacerNet() : m_type(PlacerNetType::Undefined), m_weight(1.0f) {}

    bool isValid() const noexcept
    {
        return m_nodes.size() >= 2;
    }

    void dump(std::ostream &os) const
    {
        for(auto const& nodeId : m_nodes)
        {
            os << " " << nodeId;
        }
        os << "\n";
    }

    std::vector<PlacerNodeId>   m_nodes;
    PlacerNetType               m_type;
    float m_weight;
};

struct PlacerNetlist
{
    std::vector<PlacerNet>  m_nets;
    std::vector<PlacerNode> m_nodes;

    PlacerNetId     createNet();
    PlacerNodeId    createNode();

    PlacerNet& getNet(PlacerNetId id)
    {
        return m_nets.at(id);
    }

    PlacerNode& getNode(PlacerNodeId id)
    {
        return m_nodes.at(id);
    }

    const PlacerNet& getNet(PlacerNetId id) const
    {
        return m_nets.at(id);
    }

    const PlacerNode& getNode(PlacerNodeId id) const
    {
        return m_nodes.at(id);
    }

    [[nodiscard]] size_t numberOfNodes() const noexcept
    {
        return m_nodes.size();
    }

    [[nodiscard]] size_t numberOfNets() const noexcept
    {
        return m_nets.size();
    }

    void dump(std::ostream &os) const;
};

}; // namespace