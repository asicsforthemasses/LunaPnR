#include "common/logging.h"
#include "cts.h"
#include "cts_private.h"

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
