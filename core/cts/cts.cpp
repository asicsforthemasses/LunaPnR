#include <cassert>
#include "common/logging.h"
#include "cts.h"
#include "cts_private.h"

using namespace LunaCore::CTS;

ClockTreeNode::ClockTreeNode(ClockTreeNode* parent) : m_parent(parent)
{
}

ClockTreeNode::~ClockTreeNode()
{
    for(auto childPtr : m_children)
    {
        if (childPtr != nullptr)
        {
            delete childPtr;
        }
    }
}

struct 
{
    constexpr bool operator()(const LunaCore::CTS::Node &n1, const LunaCore::CTS::Node &n2) const
    {
        return n1.m_pos.m_x < n2.m_pos.m_x;
    }
} sortX;

struct 
{
    constexpr bool operator()(const LunaCore::CTS::Node &n1, const LunaCore::CTS::Node &n2) const
    {
        return n1.m_pos.m_y < n2.m_pos.m_y;
    }
} sortY;


void subdivide(std::vector<LunaCore::CTS::Node> &nodes, bool xaxis, 
    ClockTreeNode *parent)
{
    if (nodes.size() <= 4) 
    {
        assert(parent != nullptr);

        for(auto const& node : nodes)
        {
            parent->addCell(node.m_insKey);
        }
        return;
    }

    std::vector<LunaCore::CTS::Node> group1;
    std::vector<LunaCore::CTS::Node> group2;

    // medianIndex is the index where the second group begins
    auto medianIndex = nodes.size()/2;

    if (xaxis)
    {
        std::sort(nodes.begin(), nodes.end(), sortX);
    }
    else
    {
        std::sort(nodes.begin(), nodes.end(), sortY);
    }

    group1.reserve(1 + medianIndex);
    group2.reserve(1 + medianIndex);

    std::copy(nodes.begin(), nodes.begin()+medianIndex, group1.begin());
    std::copy(nodes.begin()+medianIndex, nodes.end(), group2.begin());

    auto child1 = new ClockTreeNode(parent);
    auto child2 = new ClockTreeNode(parent);

    parent->setChild(0, child1);
    parent->setChild(1, child2);

    nodes.clear();
    subdivide(group1, !xaxis, child1);
    subdivide(group2, !xaxis, child2);
}

std::unique_ptr<LunaCore::CTS::ClockTreeNode> LunaCore::CTS::doStuff(const std::string &clockNetName, ChipDB::Netlist &netlist)
{
    auto clockNet = netlist.lookupNet(clockNetName);
    if (!clockNet.isValid())
    {
        Logging::doLog(Logging::LogType::ERROR, "CTS cannot find the specified clock net %s\n", clockNetName.c_str());
        return nullptr;
    }

    std::vector<Node> clkNodes;
    clkNodes.reserve(clockNet->numberOfConnections());

    for(auto const& conn : *clockNet)
    {
        auto ins = netlist.lookupInstance(conn.m_instanceKey);
        if (!ins)
        {
            Logging::doLog(Logging::LogType::ERROR, "CTS cannot find instance with key %d\n", conn.m_instanceKey);
            return nullptr;
        }

        if (!ins->isPlaced())
        {
            Logging::doLog(Logging::LogType::ERROR, "CTS: instance %s has not been placed - aborting!\n", ins->name().c_str());
            return nullptr;
        }

        auto &newNode = clkNodes.emplace_back();
        newNode.m_insKey = conn.m_instanceKey;
        newNode.m_pos    = ins->m_pos;
    }

    auto headNode = std::make_unique<LunaCore::CTS::ClockTreeNode>(nullptr);

    subdivide(clkNodes, true, headNode.get());

    return std::move(headNode);
}
