#include "netlisttools.h"
#include "common/logging.h"


bool LunaCore::NetlistTools::writePlacementFile(std::ostream &os, const ChipDB::Netlist *netlist)
{
    if (!os.good())
    {
        return false;
    }

    if (netlist == nullptr)
    {
        return false;
    }

    for(auto ins : netlist->m_instances)
    {
        if (ins.isValid() && 
            ((ins->m_placementInfo == ChipDB::PlacementInfo::PLACED) || (ins->m_placementInfo == ChipDB::PlacementInfo::PLACEDANDFIXED)))
        {
            auto insSize = ins->instanceSize();
            os << ins->m_pos.m_x << " " << ins->m_pos.m_y << " " << insSize.m_x << " " << insSize.m_y << "\n";
        }
    }

    return true;
}


LunaCore::NetlistTools::NetlistHistogram LunaCore::NetlistTools::calcNetlistHistogram(const ChipDB::Netlist *netlist)
{
    NetlistHistogram hist;

    if (netlist == nullptr)
    {
        return hist;
    }

    for(const auto net : netlist->m_nets)
    {
        hist[net->m_connections.size()]++;
    }

    return hist;
}

bool LunaCore::NetlistTools::removeNetconInstances(ChipDB::Netlist &netlist)
{
    size_t netsRemoved = 0;
    size_t degenerateNets = 0;
    for (auto ins : netlist.m_instances)
    {
        if (ins->getArchetypeName() == "__NETCON")
        {
            auto netConIns = ins;
            auto net1 = netConIns->getConnectedNet(0);
            auto net2 = netConIns->getConnectedNet(1);

            if ((net1 == nullptr) || (net2 == nullptr))
            {
                // handle degenerate net: remove NETCON            
                degenerateNets++;
            }
            else
            {
                auto netToRemove = net1;
                auto netToAssign = net2;

                for(auto const &connection : netToRemove->m_connections)
                {
                    auto insToBeUpdated = netlist.m_instances.at(connection.m_instanceKey);
                    if (insToBeUpdated != netConIns.key())
                    {
                        insToBeUpdated->connect(connection.m_pinIndex, netToAssign);
                    }
                }

                netToRemove->m_connections.clear();
                delete ins;
                ins = nullptr;
                netsRemoved++;
            }
        }
    }
    std::stringstream ss;
    ss << "Removed " << netsRemoved << " netcon nets (found " << degenerateNets << " degerate nets)\n";
    doLog(LOG_VERBOSE, ss);
    return true;
}
