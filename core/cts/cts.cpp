#include "common/logging.h"
#include "cts.h"
#include "cts_private.h"

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


void subdivide(std::vector<LunaCore::CTS::Node> &nodes, bool xaxis)
{
    if (nodes.size() <= 4) return;

    std::vector<LunaCore::CTS::Node> group1;
    std::vector<LunaCore::CTS::Node> group2;

    group1.reserve(1 + nodes.size()/2);
    group2.reserve(1 + nodes.size()/2);

    if (xaxis)
    {
        std::sort(nodes.begin(), nodes.end(), sortX);
    }
    else
    {
        std::sort(nodes.begin(), nodes.end(), sortY);
    }

    nodes.clear();
    subdivide(group1, !xaxis);
    subdivide(group2, !xaxis);
}

bool LunaCore::CTS::doStuff(const std::string &clockNetName, ChipDB::Netlist &netlist)
{
    auto clockNet = netlist.lookupNet(clockNetName);
    if (!clockNet.isValid())
    {
        Logging::doLog(Logging::LogType::ERROR, "CTS cannot find the specified clock net %s\n", clockNetName.c_str());
        return false;
    }

    std::vector<Node> clkNodes;
    clkNodes.reserve(clockNet->numberOfConnections());

    for(auto const& conn : *clockNet)
    {
        auto ins = netlist.lookupInstance(conn.m_instanceKey);
        if (!ins)
        {
            Logging::doLog(Logging::LogType::ERROR, "CTS cannot find instance with key %d\n", conn.m_instanceKey);
            return false;
        }

        if (!ins->isPlaced())
        {
            Logging::doLog(Logging::LogType::ERROR, "CTS: instance %s has not been placed - aborting!\n", ins->name().c_str());
            return false;
        }

        auto &newNode = clkNodes.emplace_back();
        newNode.m_insKey = conn.m_instanceKey;
        newNode.m_pos    = ins->m_pos;
    }

    return true;
}
