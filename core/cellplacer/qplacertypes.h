#pragma once

#include <vector>
#include <cstdlib>
#include "common/dbtypes.h"

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

struct PlacerNode
{
    PlacerNode() : m_type(PlacerNodeType::Undefined), m_weight(1.0f) {}

    ChipDB::Coord64 m_pos;      ///< center position of node in nm
    ChipDB::Coord64 m_size;     ///< size in nm

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
        os << "  Pos: " << m_pos << "\n";
    }

    std::vector<PlacerNetId> m_connections;    
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